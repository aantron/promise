/* This file is part of Repromise, released under the MIT license. See
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

  Repromise.resolved(true);
};

let resolved = Framework.suite("resolved", [
  test("Js.Promise.resolve", () => {
    measure_resolved("Js.Promise.resolve", () =>
      for (_ in 1 to resolved_repetitions) {
        ignore(Js.Promise.resolve(1));
      });
  }),

  test("Repromise.resolved", () => {
    measure_resolved("Repromise.resolved", () =>
      for (_ in 1 to resolved_repetitions) {
        ignore(Repromise.resolved(1));
      });
  }),

  test("Js.Promise.resolve, nested promise", () => {
    let p = Js.Promise.resolve(1);
    measure_resolved("Js.Promise.resolve, nested", () =>
      for (_ in 1 to resolved_repetitions) {
        ignore(Js.Promise.resolve(p));
      });
  }),

  test("Repromise.resolved, nested promise", () => {
    let p = Repromise.resolved(1);
    measure_resolved("Repromise.resolved, nested", () =>
      for (_ in 1 to resolved_repetitions) {
        ignore(Repromise.resolved(p))
      });
  }),
]);

/* The number of "thens" we can schedule is limited by the size of the heap,
   because each one's callback is queued for calling on the next tick.

   With a number of repetitions that *almost* exhausts the heap (1M, with my
   setup), we *have* to run multiple ticks. Otherwise, we don't trigger a
   garbage collection during the Js.Promise measurement, and *do* trigger GC
   during the Repromise measurement, invalidating its result. By running many
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
      Repromise.resolved()
      |> Repromise.andThen(() => iteration(iterations_remaining - 1));
    }
    else {
      let elapsed = hrtime() -. start_time;
      let nanoseconds =
        elapsed
        /. float_of_int(then_repetitions)
        /. float_of_int(then_ticks)
        *. 1e9;
      Printf.printf("%s   %f\n", label, nanoseconds);

      Repromise.resolved(true);
    }
  };
  iteration(then_ticks);
};

let andThen = Framework.suite("andThen", [
  test("Js.Promise.then_", () => {
    let p = Js.Promise.resolve(1);
    measure_then("Js.Promise.then_", () =>
      for (_ in 1 to then_repetitions) {
        p
        |> Js.Promise.then_(_ => p)
        |> ignore
      });
  }),

  test("Repromise.andThen", () => {
    let p = Repromise.resolved(1);
    measure_then("Repromise.andThen", () =>
      for (_ in 1 to then_repetitions) {
        p
        |> Repromise.andThen(_ => p)
        |> ignore
      });
  }),
]);



let suites = [resolved, andThen];

let () =
  Framework.run("benchmark", suites);
