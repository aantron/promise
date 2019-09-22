/* This file is part of reason-promise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/repromise/blob/master/LICENSE.md. */



let test = Framework.test;



[@bs.val]
external hrtime: unit => (int, int) = "process.hrtime";

let hrtime = () => {
  let (seconds, nanoseconds) = hrtime ();
  float_of_int(seconds) +. float_of_int(nanoseconds) *. 1e-9
};

let resolved_repetitions = 100_000_000;

let measure_resolved = (label, f) => {
  let start_time = hrtime();

  f();

  let elapsed = hrtime() -. start_time;
  let nanoseconds = elapsed /. float_of_int(resolved_repetitions) *. 1e9;
  Printf.printf("%s   %f\n", label, nanoseconds);

  Promise.resolved(true);
};

let resolved = Framework.suite("resolved", [
  test("Js.Promise.resolve", () => {
    measure_resolved("Js.Promise.resolve", () =>
      for (_ in 1 to resolved_repetitions) {
        ignore(Js.Promise.resolve(1));
      });
  }),

  test("Promise.resolved", () => {
    measure_resolved("Promise.resolved", () =>
      for (_ in 1 to resolved_repetitions) {
        ignore(Promise.resolved(1));
      });
  }),

  test("Js.Promise.resolve, nested promise", () => {
    let p = Js.Promise.resolve(1);
    measure_resolved("Js.Promise.resolve, nested", () =>
      for (_ in 1 to resolved_repetitions) {
        ignore(Js.Promise.resolve(p));
      });
  }),

  test("Promise.resolved, nested promise", () => {
    let p = Promise.resolved(1);
    measure_resolved("Promise.resolved, nested", () =>
      for (_ in 1 to resolved_repetitions) {
        ignore(Promise.resolved(p))
      });
  }),
]);

/* The number of "thens" we can schedule is limited by the size of the heap,
   because each one's callback is queued for calling on the next tick.

   With a number of repetitions that *almost* exhausts the heap (1M, with my
   setup), we *have* to run multiple ticks. Otherwise, we don't trigger a
   garbage collection during the Js.Promise measurement, and *do* trigger GC
   during the Promise measurement, invalidating its result. By running many
   ticks, we suffer multiple garbage collections during each measurement, and
   the cost is fairly included in each one. */
let then_repetitions = 1_000_000;
let then_ticks = 20;

let measure_then = (label, f) => {
  let start_time = hrtime();

  let rec iteration = iterations_remaining => {
    if (iterations_remaining > 0) {
      f();

      /* The callback will be called on the next event loop iteration, after any
         callbacks scheduled by f(). */
      Promise.resolved()
      ->Promise.flatMap(() => iteration(iterations_remaining - 1));
    }
    else {
      let elapsed = hrtime() -. start_time;
      let nanoseconds =
        elapsed
        /. float_of_int(then_repetitions)
        /. float_of_int(then_ticks)
        *. 1e9;
      Printf.printf("%s   %f\n", label, nanoseconds);

      Promise.resolved(true);
    }
  };
  iteration(then_ticks);
};

let flatMap = Framework.suite("flatMap", [
  test("Js.Promise.then_", () => {
    let p = Js.Promise.resolve(1);
    measure_then("Js.Promise.then_", () =>
      for (_ in 1 to then_repetitions) {
        p
        |> Js.Promise.then_(_ => p)
        |> ignore
      });
  }),

  test("Promise.flatMap", () => {
    let p = Promise.resolved(1);
    measure_then("Promise.flatMap", () =>
      for (_ in 1 to then_repetitions) {
        ignore(p->Promise.flatMap(_ => p));
      });
  }),
]);



let suites = [resolved, flatMap];

let () =
  Framework.run("benchmark", suites);
