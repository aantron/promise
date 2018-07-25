let test = Framework.test;



/* Counts the number of live words in the heap at the time it is called. To get
   the number allocated and retained between two points, call this function at
   both points, then subtract the two results from each other. */
let countAllocatedWords = () => {
  Gc.full_major();
  let stat = Gc.stat();
  stat.Gc.live_words;
};

/* Checks that loop() does not leak memory. loop() is a function that starts an
   asynchronous computation, and the promise it returns resolves with the number
   of words allocated in the heap during the computation. loop() is run twice,
   the second time for 10 times as many iterations as the first. If the number
   of words allocated is not roughly constant, the computation leaks memory.

   baseIterations is used to adjust how many iterations to run. Different loops
   take different amounts of time, and we don't want to slow down the tests too
   much by running a slow loop for too many iterations.

   loop must call countAllocatedWords itself. Factoring the call out to this
   function doesNotLeakMemory will call countAllocatedWords too late, because
   loop will have returned and released all references that it is holding. */
let doesNotLeakMemory = (loop, baseIterations) =>
  loop(baseIterations)
  |> Repromise.Rejectable.andThen(wordsAllocated =>

    loop(baseIterations * 10)
    |> Repromise.Rejectable.andThen(wordsAllocated' => {

      let ratio = float_of_int(wordsAllocated') /. float_of_int(wordsAllocated);
      Repromise.Rejectable.resolve(ratio < 2.);
    }));



let promiseLoopTests = Framework.suite("promise loop", [
  /* A pretty simple promise loop. This is just a function that takes a promise,
     and calls .andThen on it. The callback passed to .andThen calls the loop
     recursively, passing another promise to the next iteration. The interesting
     part is not the argument promise, but the result promise returned by each
     iteration.

     If Repromise is implemented naively, the iteration will result in a big
     chain of promises hanging in memory: a memory leak. Here is how:

     - At the first iteration, .andThen creates an outer pending promise p0, and
       returns it immediately to the rest of the code.
     - Later, the callback passed to .andThen runs. It again calls .andThen,
       creating another pending promise p1. The callback then returns p1. This
       means that resolving p1 should resolve p0, so a naive implementation
       will store a reference in p1 to p0.
     - Later, the callback passed to p1's .andThen runs, doing the same thing:
       creating another pending promise p2, pointing to p1.
     - By iteration N, there is a chain of N pending promises set up, such that
       resolving the inner-most promise in the chain, created by the last
       .andThen, will resolve the outer-most promise p0, created by the first
       .andThen. This is the memory leak. */
  test("promise loop memory leak", () => {
    let instrumentedPromiseLoop = n => {
      let initialWords = countAllocatedWords();

      let rec promiseLoop: Repromise.t(int) => Repromise.t(int) =
          previousPromise =>

        previousPromise
        |> Repromise.andThen(n => {
          if (n == 0) {
            let wordsAllocated = countAllocatedWords() - initialWords;
            Repromise.resolve(wordsAllocated);
          }
          else {
            promiseLoop(Repromise.resolve(n - 1))
          }});

      promiseLoop(Repromise.resolve(n));
    };

    doesNotLeakMemory(instrumentedPromiseLoop, 1000);
  }),

  /* The fix for the above memory leak carries a potential pitfall: the fix is
     to merge the inner promise returned to andThen into andThen's outer promise.
     After that, all operations on the inner promise reference are actually
     performed on the outer promise.

     This carries the danger that a tower of these merged promises can build
     up. If a pending promise is repeatedly returned to andThen, it will
     gradually become the head of a growing chain of forwarding promises, that
     point to the outer promise created in the last call to andThen.

     To avoid this, the implementation has to perform union-find: each time it
     traverses a chain of merged promises, it has to set the head promise to
     point directly to the final outer promise, cutting out all intermediate
     merged promises. Then, any of these merged promises that aren't being
     referenced by the user program can be garbage-collected. */
  test("promise tower memory leak", () => {
    let instrumentedPromiseTower = n => {
      let (foreverPendingPromise, _) = Repromise.make();

      let initialWords = countAllocatedWords();

      let rec tryToBuildTower = n =>
        if (n == 0) {
          let wordsAllocated = countAllocatedWords() - initialWords;
          Repromise.resolve(wordsAllocated);
        }
        else {
          /* The purpose of the delay promise is to make sure the second call to
             andThen runs after the first. */
          let delay = Repromise.resolve();

          /* If union-find is not implemented, we will leak memory here. */
          delay
          |> Repromise.andThen(() => foreverPendingPromise)
          |> ignore;

          delay
          |> Repromise.andThen(() => tryToBuildTower(n - 1));
        };

      tryToBuildTower(n);
    };

    doesNotLeakMemory(instrumentedPromiseTower, 1000);
  }),
]);



/* The skeleton of a test for memory safety of Repromise.race. Creates a
   long-lived promise, and repeatedly calls the body function on it, which is
   customized by each test. */
let raceTest = (name, body) =>
  test(name, () => {
    let instrumentedLoop = n => {
      let (foreverPendingPromise, _, _) = Repromise.Rejectable.make();

      let initialWords = countAllocatedWords();

      let rec theLoop: int => Repromise.Rejectable.t(int, unit) = n =>
        if (n == 0) {
          let wordsAllocated = countAllocatedWords() - initialWords;
          Repromise.Rejectable.resolve(wordsAllocated);
        }
        else {
          let nextIteration = () => theLoop(n - 1);
          body(foreverPendingPromise, nextIteration);
        };
      theLoop(n);
    };

    doesNotLeakMemory(instrumentedLoop, 100)
    |> Repromise.Rejectable.catch(() => assert(false));
  });

let raceLoopTests = Framework.suite("race loop", [
  /* To implement p3 = Repromise.race([p1, p2]), Repromise has to attach
     callbacks to p1 and p2, so that whichever of them is the first to resolve
     will cause the resolution of p3. This means that p1 and p2 hold references
     to p3.

     If, say, p1 is a promise that remains pending for a really long time, and
     it is raced with many other promises in a loop, i.e.

       p3 = Repromise.race([p1, p2])
       p3' = Repromise.race([p1, p2'])
       etc.

     Then p1 will accumulate callbacks with references to p3, p3', etc. This
     will be a memory leak, that grows in proportion to the number of times the
     race loop has run.

     Since this is a common usage pattern, a reasonable implementation has to
     remove callbacks from p1  when p3, p3', etc. are resolved by race. This
     test checks for such an implementation. */
  raceTest("race loop memory leak", (foreverPendingPromise, nextIteration) => {
    let (shortLivedPromise, resolveShortLivedPromise, _) =
      Repromise.Rejectable.make();

    let racePromise =
      Repromise.Rejectable.race([foreverPendingPromise, shortLivedPromise]);

    resolveShortLivedPromise();

    racePromise |> Repromise.Rejectable.andThen(nextIteration);
  }),

  raceTest("race loop memory leak, with already-resolved promises",
      (foreverPendingPromise, nextIteration) => {
    let resolvedPromise = Repromise.Rejectable.resolve();

    let racePromise =
      Repromise.Rejectable.race([foreverPendingPromise, resolvedPromise]);

    racePromise |> Repromise.Rejectable.andThen(nextIteration);
  }),

  raceTest("race loop memory leak, with rejection",
      (foreverPendingPromise, nextIteration) => {
    let (shortLivedPromise, _, rejectShortLivedPromise) =
      Repromise.Rejectable.make();

    let racePromise =
      Repromise.Rejectable.race([foreverPendingPromise, shortLivedPromise]);

    rejectShortLivedPromise();

    racePromise
    |> Repromise.Rejectable.andThen(() => assert(false))
    |> Repromise.Rejectable.catch(nextIteration);
  }),

  raceTest("race loop memory leak, with already-rejected promises",
      (foreverPendingPromise, nextIteration) => {
    let rejectedPromise = Repromise.Rejectable.reject();

    let racePromise =
      Repromise.Rejectable.race([foreverPendingPromise, rejectedPromise]);

    racePromise
    |> Repromise.Rejectable.andThen(() => assert(false))
    |> Repromise.Rejectable.catch(nextIteration);
  }),

  /* This test is like the first, but it tests for the interaction of the fixes
     for the andThen and race loop memory leaks. The danger is:

     - The andThen fix "wants" to merge callback lists when an inner pending
       promise is returned from the callback of andThen.
     - The race fix "wants" to delete callbacks from a callback list, when a
       promise "loses" to another one that resolved sooner.

     It is important that the callback list merging performed by andThen doesn't
     prevent race from finding and deleting the correct callbacks in the merged
     lists. */
  raceTest("race loop memory leak with andThen merging",
      (foreverPendingPromise, nextIteration) => {
    let (shortLivedPromise, resolveShortLivedPromise, _) =
      Repromise.Rejectable.make();

    let racePromise =
      Repromise.Rejectable.race([foreverPendingPromise, shortLivedPromise]);

    /* Return foreverPendingPromise from the callback of andThen. This causes all
       of its callbacks to be moved to the outer promise of the andThen (which we
       don't give a name to). The delay promise is just used to make the second
       call to andThen definitely run after the first. */
    let delay = Repromise.Rejectable.resolve();

    delay
    |> Repromise.Rejectable.andThen(() => foreverPendingPromise)
    |> ignore;

    delay
    |> Repromise.Rejectable.andThen(() => {
      /* Now, we resolve the short-lived promise. If that doesn't delete the
         callback that was merged away from foreverPendingPromise, then this is
         where we will accumulate the memory leak. */
      resolveShortLivedPromise();
      racePromise |> Repromise.Rejectable.andThen(nextIteration);
    });
  }),
]);



let allLoopTests = Framework.suite("all loop", [
  /* Like Repromise.race, there is a danger of memory leak in Repromise.all.
     When one of the promises in Repromise.all is rejected, the final promise is
     rejected immediately. If callbacks attached to still-pending promises are
     not removed, a memory leak will accumulate.

     We reuse the raceTest helper, because the tests are structurally the same.
     race remains the function with the most opportunities to leak memory. */
  raceTest("all loop memory leak", (foreverPendingPromise, nextIteration) => {
    let (shortLivedPromise, _, rejectShortLivedPromise) =
      Repromise.Rejectable.make();

    let allPromise =
      Repromise.Rejectable.all([foreverPendingPromise, shortLivedPromise]);

    rejectShortLivedPromise();

    allPromise
    |> Repromise.Rejectable.andThen((_) => assert false)
    |> Repromise.Rejectable.catch(nextIteration);
  }),

  raceTest("all loop memory leak, with already-rejected promises",
      (foreverPendingPromise, nextIteration) => {
    let rejectedPromise = Repromise.Rejectable.reject();

    let allPromise =
      Repromise.Rejectable.all([foreverPendingPromise, rejectedPromise]);

    allPromise
    |> Repromise.Rejectable.andThen((_) => assert false)
    |> Repromise.Rejectable.catch(nextIteration);
  }),

  /* Tests the interaction of the memory-leak fixes in all and andThen, as tested
     for race and andThen above. */
  raceTest("race loop memory leak with andThen merging",
      (foreverPendingPromise, nextIteration) => {
    let (shortLivedPromise, _, rejectShortLivedPromise) =
      Repromise.Rejectable.make();

    let allPromise =
      Repromise.Rejectable.all([foreverPendingPromise, shortLivedPromise]);

    let delay = Repromise.Rejectable.resolve();

    delay
    |> Repromise.Rejectable.catch((_) => assert(false))
    |> Repromise.Rejectable.andThen(() => foreverPendingPromise)
    |> ignore;

    delay
    |> Repromise.Rejectable.catch((_) => assert(false))
    |> Repromise.Rejectable.andThen(() => {
      rejectShortLivedPromise();
      allPromise
      |> Repromise.Rejectable.andThen((_) => assert false)
      |> Repromise.Rejectable.catch(nextIteration);
    });
  }),
]);



let suites = [promiseLoopTests, raceLoopTests, allLoopTests];
