let test = Framework.test;



let promiseLoopTests = Framework.suite("promise loop", [
  test("promise loop memory leak", () => {
    /* Counts the number of live words in the heap at the time it is called. To
       get the number allocated and retained between two points, call this
       function at both points, then subtract the two results from each
       other. */
    let countAllocatedWords = () => {
      Gc.full_major();
      let stat = Gc.stat();
      stat.Gc.live_words;
    };

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

    /* Run the above promise loop for N iterations, then for k*N iterations. If
       about k times more memory was allocated during k*N iterations, then the
       loop leaks memory. */
    let n = 1337;
    let k = 10;

    instrumentedPromiseLoop(n)
    |> Repromise.then_(wordsAllocated =>

      instrumentedPromiseLoop(k * n)
      |> Repromise.then_(wordsAllocated' => {

        let ratio =
          float_of_int(wordsAllocated') /. float_of_int(wordsAllocated);
        let ratio_ok = ratio < 2.;

        Repromise.resolve(ratio_ok);
      }))
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



let suites = [promiseLoopTests, libuvTests];
