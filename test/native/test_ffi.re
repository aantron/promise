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
   the second time for 10 times as many iterations as the first. IF the number
   of words allocated is not roughly constant, the computation leaks memory.

   baseIterations is used to adjust how many iterations to run. Different loops
   take different amounts of time, and we don't want to slow down the tests too
   much by running a slow loop for too many iterations. */
let doesNotLeakMemory = (loop, baseIterations) =>
  loop(baseIterations)
  |> Repromise.then_(wordsAllocated =>

    loop(baseIterations * 10)
    |> Repromise.then_(wordsAllocated' => {

      let ratio = float_of_int(wordsAllocated') /. float_of_int(wordsAllocated);
      Repromise.resolve(ratio < 2.);
    }));



let promiseLoopTests = Framework.suite("promise loop", [
  test("promise loop memory leak", () => {
    /* A pretty simple promise loop. This is just a function that takes a
       promise, and calls .then_ on it. The callback passed to .then_ calls the
       loop recursively, passing another promise to the next iteration. The
       interesting part is not the argument promise, but the result promise
       returned by each iteration.

       If Repromise is implemented naively, the iteration will result in a big
       chain of promises hanging in memory: a memory leak. Here is how:

       - At the first iteration, .then_ creates an outer pending promise p0, and
         returns it immediately to the rest of the code.
       - Later, the callback passed to .then_ runs. It again calls .then_,
         creating another pending promise p1. The callback then returns p1. This
         means that resolving p1 should resolve p0, so a naive implementation
         will store a reference in p1 to p0.
       - Later, the callback passed to p1's .then_ runs, doing the same thing:
         creating another pending promise p2, pointing to p1.
       - By iteration N, there is a chain of N pending promises set up, such
         that resolving the inner-most promise in the chain, created by the last
         .then_, will resolve the outer-most promise p0, created by the first
         .then_. This is the memory leak. */
    let instrumentedPromiseLoop = n => {
      let initialWords = countAllocatedWords();

      let rec promiseLoop: Repromise.t(int, 'e) => Repromise.t(int, 'e) =
          previousPromise =>

        previousPromise
        |> Repromise.then_(n => {
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
]);



let raceLoopTests = Framework.suite("race loop", [
  test("race loop memory leak", () => {
    /* To implement p3 = Repromise.race([p1, p2]), Repromise has to attach
       callbacks to p1 and p2, so that whichever of them is the first to resolve
       will cause the resolution of p3. This means that p1 and p2 hold
       references to p3.

       If, say, p1 is a promise that remains pending for a really long time, and
       it is raced with many other promises in a loop, i.e.

         p3 = Repromise.race([p1, p2])
         p3' = Repromise.race([p1, p2'])
         etc.

       Then p1 will accumulate callbacks with references to p3, p3', etc. This
       will be a memory leak, that grows in proportion to the number of times
       the race loop has run.

       Since this is a common usage pattern, a reasonable implementation has to
       remove callbacks from p1  when p3, p3', etc. are resolved by race. This
       test checks for such an implementation. */
    let instrumentedRaceLoop = n => {
      let foreverPendingPromise = Repromise.new_((_resolve, _reject) => ());

      let initialWords = countAllocatedWords();

      let rec raceLoop = n =>
        if (n == 0) {
          let wordsAllocated = countAllocatedWords() - initialWords;
          Repromise.resolve(wordsAllocated);
        }
        else {
          let resolveShortLivedPromise = ref(ignore);
          let shortLivedPromise = Repromise.new_((resolve, _reject) =>
            resolveShortLivedPromise := resolve);

          let racePromise =
            Repromise.race([foreverPendingPromise, shortLivedPromise]);

          resolveShortLivedPromise^();

          racePromise |> Repromise.then_(() => raceLoop(n - 1));
        };

      raceLoop(n);
    };

    doesNotLeakMemory(instrumentedRaceLoop, 100);
  }),
]);



let loop = Libuv_loop.default ();

let libuvTests = Framework.suite("libuv", [
  /* These are janky proof-of-concept tests. We are not even trying to close the
     file. */
  test("open_ sync", () => {
    let fd = Libuv_fs.Sync.open_(loop, "test/test.re", ~flags = 0, ~mode = 0);
    Repromise.resolve(fd > 0);
  }),

  test("open_ async", () => {
    Repromise.new_((resolve, _) =>
      Libuv_fs.Async.open_(loop, "test/test.re", ~flags = 0, ~mode = 0, fd =>
        resolve(fd > 0)));
  }),
]);



let suites = [promiseLoopTests, raceLoopTests, libuvTests];
