/* This file is part of reason-promise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/promise/blob/master/LICENSE.md. */



let test = Framework.test;
open! Promise.PipeFirst;



let basicTests = Framework.suite("basic", [
  /* The basic [resolved]-[flatMap] tests are a bit useless, because the testing
     framework itself already uses both [resolved] and [then], i.e. every test
     implicitly tests those. However, we include these for completeness, in case
     we become enlightened and rewrite the framework in CPS or something.  */
  test("resolved", () => {
    Promise.resolved(true);
  }),

  test("get", () => {
    let correct = ref(false);
    Promise.resolved(1)->Promise.get(n => correct := (n == 1));
    Promise.resolved()->Promise.map(() => correct^);
  }),

  test("tap", () => {
    let correct = ref(false);
    Promise.resolved(1)
    ->Promise.tap(n => correct := (n == 1))
    ->Promise.map(n => n == 1 && correct^);
  }),

  test("flatMap", () => {
    Promise.resolved(1)
    ->Promise.flatMap(n => Promise.resolved(n == 1));
  }),

  test("map", () => {
    let p = Promise.resolved(6)->Promise.map(v => v * 7);
    p->Promise.flatMap(r => Promise.resolved(r == 42));
  }),

  test("map chain", () => {
    let p =
      Promise.resolved(6)
      ->Promise.map(v => v * 7)
      ->Promise.map(r => r * 10);
    p->Promise.flatMap(r => Promise.resolved(r == 420));
  }),

  test("map soundness", () => {
      Promise.resolved(6)
      ->Promise.map(v => v * 7)
      ->Promise.map(x => Promise.resolved(x == 42))
      ->Promise.flatMap(r => r);
  }),

  test("flatMap chain", () => {
    Promise.resolved(1)
    ->Promise.flatMap(n => Promise.resolved(n + 1))
    ->Promise.flatMap(n => Promise.resolved(n == 2));
  }),

  test("flatMap nested", () => {
    Promise.resolved(1)
    ->Promise.flatMap (n =>
      Promise.resolved(n + 1)
      ->Promise.flatMap(n => Promise.resolved(n + 1)))
    ->Promise.flatMap(n => Promise.resolved(n == 3));
  }),

  /* If promises are implemented on JS directly as ordinary JS promises,
     [resolved(resolved(42))] will collapse to just a [promise(int)], even
     though the Reason type is [promise(promise(int))]. This causes a soundness
     bug, because, due to the type, the callback of [flatMap] will expect the
     nested value to be a [promise(int)]. A correct implementation of Reason
     promises on JS will avoid this bug. */
  test("no collapsing", () => {
    Promise.resolved(Promise.resolved(1))
    ->Promise.flatMap(p =>
      p->Promise.flatMap(n => Promise.resolved(n == 1)));
  }),

  test("pending", () => {
    let (p, resolve) = Promise.pending();
    resolve(true);
    p;
  }),

  test("defer", () => {
    let (p, resolve) = Promise.pending();
    let p' = p->Promise.flatMap(n => Promise.resolved(n == 1));
    resolve(1);
    p';
  }),

  test("double resolve", () => {
    let (p, resolve) = Promise.pending();
    resolve(42);
    p->Promise.flatMap(n => {
      resolve(43);
      p->Promise.map(n' =>
        n == 42 && n' == 42)});
  }),

  test("exec", () => {
    Promise.exec(resolve => resolve(true));
  }),

  test("callback order (already resolved)", () => {
    let firstCallbackCalled = ref(false);
    let p = Promise.resolved();
    p->Promise.map(() => firstCallbackCalled := true) |> ignore;
    p->Promise.map(() => firstCallbackCalled^);
  }),

  test("callback order (resolved later)", () => {
    let firstCallbackCalled = ref(false);
    let secondCallbackCalledSecond = ref(false);
    let (p, resolve) = Promise.pending();
    p->Promise.map(() => firstCallbackCalled := true) |> ignore;
    p->Promise.map(() =>
      secondCallbackCalledSecond := firstCallbackCalled^) |> ignore;
    resolve();
    p->Promise.map(() => secondCallbackCalledSecond^);
  }),

  test("relax", () => {
    let p = Promise.resolved();
    Promise.resolved(Promise.Js.relax(p) === p);
  }),
]);



let rejectTests = Framework.suite("reject", [
  test("pending", () => {
    let (p, _, reject) = Promise.Js.pending();
    reject(1);
    p->Promise.Js.catch(n => Promise.resolved(n == 1));
  }),

  test("reject, catch", () => {
    Promise.Js.rejected("foo")
    ->Promise.Js.catch(s => Promise.resolved(s == "foo"));
  }),

  test("catch chosen", () => {
    Promise.Js.rejected("foo")
    ->Promise.Js.catch(s => Promise.resolved(s == "foo"));
  }),

  test("flatMap, reject, catch", () => {
    Promise.Js.resolved(1)
    ->Promise.Js.flatMap(n => Promise.Js.rejected(n + 1))
    ->Promise.Js.catch(n => Promise.resolved(n == 2));
  }),

  test("reject, catch, flatMap", () => {
    Promise.Js.rejected(1)
    ->Promise.Js.catch(n => Promise.resolved(n + 1))
    ->Promise.flatMap(n => Promise.resolved(n == 2));
  }),

  test("no double catch", () => {
    Promise.Js.rejected("foo")
    ->Promise.Js.catch(s => Promise.resolved(s == "foo"))
    ->Promise.Js.catch((_) => Promise.resolved(false));
  }),

  test("catch chain", () => {
    Promise.Js.rejected(1)
    ->Promise.Js.catch(n => Promise.Js.rejected(n + 1))
    ->Promise.Js.catch(n => Promise.resolved(n == 2));
  }),

  test("no catching resolved", () => {
    Promise.resolved(true)
    ->Promise.Js.catch((_) => Promise.resolved(false));
  }),

  test("no catching resolved, after flatMap", () => {
    Promise.resolved()
    ->Promise.flatMap(() => Promise.resolved(true))
    ->Promise.Js.catch((_) => Promise.resolved(false));
  }),

  /* See https://github.com/aantron/promise/issues/74. If tap internally calls
     map, but then returns the original promise, and the original promise gets
     rejected, then both the mapped promise and the original promise are
     rejected. The rejected mapped promise results in an unhandled promise
     rejection, because there is no way to handle that rejection - the mapped
     promise is ignored internally by tap. */
  test("tap unhandled rejetion", () => {
    Promise.Js.rejected("foo")
    ->Promise.Js.tap(ignore)
    ->Promise.Js.catch(_ => Promise.resolved(true));
  }),
]);



let remainsPending = (p, dummyValue) => {
  let rec repeat = (n, f) =>
    if (n == 0) {
      Promise.resolved(true);
    }
    else {
      f ()
      ->Promise.flatMap(result =>
        if (!result) {
          Promise.resolved(false);
        }
        else {
          repeat(n - 1, f);
        })
    };

  repeat(10, () =>
    Promise.race([p, Promise.resolved(dummyValue)])
    ->Promise.flatMap(v1 =>
      Promise.race([Promise.resolved(dummyValue), p])
      ->Promise.map(v2 =>
        v1 == dummyValue && v2 == dummyValue)));
};

let allTests = Framework.suite("all", [
  test("already resolved", () => {
    Promise.all([Promise.resolved(42), Promise.resolved(43)])
    ->Promise.map(results => results == [42, 43]);
  }),

  test("resolved later", () => {
    let (p1, resolveP1) = Promise.pending();
    let (p2, resolveP2) = Promise.pending();
    let p3 = Promise.all([p1, p2]);
    resolveP1(42);
    resolveP2(43);
    p3->Promise.map(results => results == [42, 43]);
  }),

  test("not all resolved", () => {
    let (p1, resolveP1) = Promise.pending();
    let (p2, _) = Promise.pending();
    let p3 = Promise.all([p1, p2]);
    resolveP1(42);
    remainsPending(p3, []);
  }),

  test("simultaneous resolve", () => {
    let (p1, resolveP1) = Promise.pending();
    let p2 = Promise.all([p1, p1]);
    resolveP1(42);
    p2->Promise.map(results => results == [42, 42]);
  }),

  test("already rejected", () => {
    let (p1, _, _) = Promise.Js.pending();
    let p2 = Promise.Js.all([p1, Promise.Js.rejected(43)]);
    p2
    ->Promise.Js.flatMap((_) => Promise.Js.resolved(false))
    ->Promise.Js.catch(n => Promise.resolved(n == 43));
  }),

  test("rejected later", () => {
    let (p1, _, rejectP1) = Promise.Js.pending();
    let (p2, _, _) = Promise.Js.pending();
    let p3 = Promise.Js.all([p1, p2]);
    rejectP1(42);
    p3
    ->Promise.Js.flatMap((_) => Promise.Js.resolved(false))
    ->Promise.Js.catch(n => Promise.resolved(n == 42));
  }),

  test("remains rejected", () => {
    let (p1, _, rejectP1) = Promise.Js.pending();
    let (p2, resolveP2, _) = Promise.Js.pending();
    let p3 = Promise.Js.all([p1, p2]);
    rejectP1(42);
    resolveP2(43);
    p2
    ->Promise.Js.catch((_) => assert false)
    ->Promise.Js.flatMap((_) =>
      p3
      ->Promise.Js.flatMap((_) => Promise.Js.resolved(false))
      ->Promise.Js.catch(n => Promise.resolved(n == 42)));
  }),

  test("empty", () => {
    Promise.all([])
    ->Promise.map(results => results == []);
  }),

  test("all2", () => {
    let (p1, resolveP1) = Promise.pending();
    let (p2, resolveP2) = Promise.pending();
    let result =
      Promise.all2(p1, p2)
      ->Promise.map(((x, y)) => x == 42 && y == 43);
    resolveP1(42);
    resolveP2(43);
    result;
  }),

  test("all3", () => {
    let (p1, resolveP1) = Promise.pending();
    let (p2, resolveP2) = Promise.pending();
    let (p3, resolveP3) = Promise.pending();
    let result =
      Promise.all3(p1, p2, p3)
      ->Promise.map(((x, y, z)) => x == 42 && y == 43 && z == 44);
    resolveP1(42);
    resolveP2(43);
    resolveP3(44);
    result;
  }),

  test("all4", () => {
    let (p1, resolveP1) = Promise.pending();
    let (p2, resolveP2) = Promise.pending();
    let (p3, resolveP3) = Promise.pending();
    let (p4, resolveP4) = Promise.pending();
    let result =
      Promise.all4(p1, p2, p3, p4)
      ->Promise.map(((x, y, z, u)) =>
        x == 42 && y == 43 && z == 44 && u == 45);
    resolveP1(42);
    resolveP2(43);
    resolveP3(44);
    resolveP4(45);
    result;
  }),

  test("all5", () => {
    let (p1, resolveP1) = Promise.pending();
    let (p2, resolveP2) = Promise.pending();
    let (p3, resolveP3) = Promise.pending();
    let (p4, resolveP4) = Promise.pending();
    let (p5, resolveP5) = Promise.pending();
    let result =
      Promise.all5(p1, p2, p3, p4, p5)
      ->Promise.map(((x, y, z, u, v)) =>
        x == 42 && y == 43 && z == 44 && u == 45 && v == 46);
    resolveP1(42);
    resolveP2(43);
    resolveP3(44);
    resolveP4(45);
    resolveP5(46);
    result;
  }),

  test("all6", () => {
    let (p1, resolveP1) = Promise.pending();
    let (p2, resolveP2) = Promise.pending();
    let (p3, resolveP3) = Promise.pending();
    let (p4, resolveP4) = Promise.pending();
    let (p5, resolveP5) = Promise.pending();
    let (p6, resolveP6) = Promise.pending();
    let result =
      Promise.all6(p1, p2, p3, p4, p5, p6)
      ->Promise.map(((x, y, z, u, v, w)) =>
        x == 42 && y == 43 && z == 44 && u == 45 && v == 46 && w == 47);
    resolveP1(42);
    resolveP2(43);
    resolveP3(44);
    resolveP4(45);
    resolveP5(46);
    resolveP6(47);
    result;
  }),

  test("allArray", () => {
    let (p1, resolveP1) = Promise.pending();
    let (p2, resolveP2) = Promise.pending();
    let result =
      Promise.allArray([|p1, p2|])
      ->Promise.map(fun
        | [|x, y|] => x == 42 && y == 43
        | _ => false);
    resolveP1(42);
    resolveP2(43);
    result;
  }),
]);



let raceTests = Framework.suite("race", [
  test("first resolves", () => {
    let (p1, resolveP1) = Promise.pending();
    let (p2, _) = Promise.pending();
    let p3 = Promise.race([p1, p2]);
    resolveP1(42);
    p3->Promise.map(n => n == 42);
  }),

  test("second resolves", () => {
    let (p1, _) = Promise.pending();
    let (p2, resolveP2) = Promise.pending();
    let p3 = Promise.race([p1, p2]);
    resolveP2(43);
    p3->Promise.map(n => n == 43);
  }),

  test("first resolves first", () => {
    let (p1, resolveP1) = Promise.pending();
    let (p2, resolveP2) = Promise.pending();
    let p3 = Promise.race([p1, p2]);
    resolveP1(42);
    resolveP2(43);
    p3->Promise.map(n => n == 42);
  }),

  test("second resolves first", () => {
    let (p1, resolveP1) = Promise.pending();
    let (p2, resolveP2) = Promise.pending();
    let p3 = Promise.race([p1, p2]);
    resolveP2(43);
    resolveP1(42);
    p3->Promise.map(n => n == 43);
  }),

  test("rejection", () => {
    let (p1, _, rejectP1) = Promise.Js.pending();
    let (p2, _, _) = Promise.Js.pending();
    let p3 = Promise.Js.race([p1, p2]);
    rejectP1(42);
    p3->Promise.Js.catch(n => Promise.resolved(n == 42));
  }),

  test("already resolved", () => {
    let (p1, _) = Promise.pending();
    let p2 = Promise.resolved(43);
    let p3 = Promise.race([p1, p2]);
    p3->Promise.map(n => n == 43);
  }),

  test("already rejected", () => {
    let (p1, _, _) = Promise.Js.pending();
    let p2 = Promise.Js.rejected(43);
    let p3 = Promise.Js.race([p1, p2]);
    p3->Promise.Js.catch(n => Promise.resolved(n == 43));
  }),

  test("two resolved", () => {
    let p1 = Promise.resolved(42);
    let p2 = Promise.resolved(43);
    let p3 = Promise.race([p1, p2]);
    p3->Promise.map(n => n == 42 || n == 43);
  }),

  test("forever pending", () => {
    let (p1, _) = Promise.pending();
    let (p2, _) = Promise.pending();
    let p3 = Promise.race([p1, p2]);
    remainsPending(p3, 43);
  }),

  test("simultaneous resolve", () => {
    let (p1, resolveP1) = Promise.pending();
    let p2 = Promise.race([p1, p1]);
    resolveP1(42);
    p2->Promise.map(n => n == 42);
  }),

  test("empty", () => {
    try ({ ignore(Promise.race([])); Promise.resolved(false); }) {
    | Invalid_argument(_) => Promise.resolved(true);
    };
  }),

  /* This test is for an implementation detail. When a pending promise p is
     returned by the callback of flatMap, the native implementation (and
     non-memory-leaking JavaScript implementations) will move the callbacks
     attached to p into the list attached to the outer promise of flatMap. We
     want to make sure that callbacks attached by race survive this moving. For
     that, p has to be involved in a call to race. */
  test("race, then callbacks moved", () => {
    let (p, resolve) = Promise.pending();
    let final = Promise.race([p]);

    /* We are using this resolve() just so we can call flatMap on it,
       guaranteeing that the second time will run after the first time.. */
    let delay = Promise.resolved();

    ignore(delay->Promise.flatMap(() => p));

    delay->Promise.flatMap(() => {
      resolve(42);
      /* This tests now succeeds only if resolving p resolved final^, despite
         the fact that p was returned to flatMap while still a pending
         promise. */
      final->Promise.map(n => n == 42);
    });
  }),

  /* Similar to the preceding test, but the race callback is attached to p after
     its callback list has been merged with the outer promise of flatMap. */
  test("callbacks moved, then race", () => {
    let (p, resolve) = Promise.pending();

    let delay = Promise.resolved();

    ignore(delay->Promise.flatMap(() => p));

    delay
    ->Promise.flatMap(() => {
      let final = Promise.race([p]);
      resolve(42);
      final->Promise.map(n => n == 42);
    });
  }),
]);



/* Compatibility with BukleScript < 6. */
open! Isoresult;

let resultTests = Framework.suite("result", [
  test("mapOk, ok", () => {
    Promise.resolved(Ok(42))
    ->Promise.mapOk(n => n + 1)
    ->Promise.map(v => v == Ok(43));
  }),

  test("mapOk, error", () => {
    Promise.resolved(Error(42))
    ->Promise.mapOk(n => n + 1)
    ->Promise.map(v => v == Error(42));
  }),

  test("mapError, ok", () => {
    Promise.resolved(Ok(42))
    ->Promise.mapError(n => n + 1)
    ->Promise.map(v => v == Ok(42));
  }),

  test("mapError, error", () => {
    Promise.resolved(Error(42))
    ->Promise.mapError(n => n + 1)
    ->Promise.map(v => v == Error(43));
  }),

  test("getOk, ok", () => {
    let (p, resolve) = Promise.pending();
    Promise.resolved(Ok(42))->Promise.getOk(n => resolve(n + 1));
    p->Promise.map(n => n == 43);
  }),

  test("getOk, error", () => {
    let called = ref(false);
    Promise.resolved(Error(42))->Promise.getOk(_ => called := true);
    Promise.resolved()->Promise.map(() => !called^);
  }),

  test("getError, ok", () => {
    let called = ref(false);
    Promise.resolved(Ok(42))->Promise.getError(_ => called := true);
    Promise.resolved()->Promise.map(() => !called^);
  }),

  test("getError, error", () => {
    let (p, resolve) = Promise.pending();
    Promise.resolved(Error(42))->Promise.getError(n => resolve(n + 1));
    p->Promise.map(n => n == 43);
  }),

  test("tapOk, ok", () => {
    let correct = ref(false);
    Promise.resolved(Ok(42))
    ->Promise.tapOk(n => correct := n == 42)
    ->Promise.map(result => result == Ok(42) && correct^);
  }),

  test("tapOk, error", () => {
    let called = ref(false);
    Promise.resolved(Error(42))
    ->Promise.tapOk(_ => called := true)
    ->Promise.map(result => result == Error(42) && !called^);
  }),

  test("tapError, ok", () => {
    let called = ref(false);
    Promise.resolved(Ok(42))
    ->Promise.tapError(_ => called := true)
    ->Promise.map(result => result == Ok(42) && !called^);
  }),

  test("getError, error", () => {
    let correct = ref(false);
    Promise.resolved(Error(42))
    ->Promise.tapError(n => correct := n == 42)
    ->Promise.map(result => result == Error(42) && correct^);
  }),

  test("flatMapOk, ok", () => {
    Promise.resolved(Ok(42))
    ->Promise.flatMapOk(n => Promise.resolved(Ok(n + 1)))
    ->Promise.map(v => v == Ok(43));
  }),

  test("flatMapOk, error", () => {
    Promise.resolved(Error(42))
    ->Promise.flatMapOk(n => Promise.resolved(Ok(n + 1)))
    ->Promise.map(v => v == Error(42));
  }),

  test("flatMapError, ok", () => {
    Promise.resolved(Ok(42))
    ->Promise.flatMapError(n => Promise.resolved(Error(n + 1)))
    ->Promise.map(v => v == Ok(42));
  }),

  test("flatMapError, error", () => {
    Promise.resolved(Error(42))
    ->Promise.flatMapError(n => Promise.resolved(Error(n + 1)))
    ->Promise.map(v => v == Error(43));
  }),

  [@ocaml.warning "-3"]
  test(">|=, ok", () => {
    let open Promise.Operators;
    (Promise.resolved(Ok(42))
    >|= (n => n + 1))
    ->Promise.map(v => v == Ok(43));
  }),

  [@ocaml.warning "-3"]
  test(">|=, error", () => {
    let open Promise.Operators;
    (Promise.resolved(Error(42))
    >|= (n => n + 1))
    ->Promise.map(v => v == Error(42));
  }),

  [@ocaml.warning "-3"]
  test(">>=, ok", () => {
    let open Promise.Operators;
    (Promise.resolved(Ok(42))
    >>= (n => Promise.resolved(Ok(n + 1))))
    ->Promise.map(v => v == Ok(43));
  }),

  [@ocaml.warning "-3"]
  test(">>=, error", () => {
    let open Promise.Operators;
    (Promise.resolved(Error(42))
    >>= (n => Promise.resolved(Ok(n + 1))))
    ->Promise.map(v => v == Error(42));
  }),

  test("toResult, resolved", () => {
    Promise.Js.resolved(1)
    ->Promise.Js.toResult
    ->Promise.Js.map(result => result == Ok(1));
  }),

  test("toResult, rejected", () => {
    Promise.Js.rejected(2)
    ->Promise.Js.toResult
    ->Promise.Js.map(result => result == Error(2));
  }),

  test("fromResult, ok", () => {
    Promise.resolved(Ok(3))
    ->Promise.Js.fromResult
    ->Promise.Js.map(v => v == 3);
  }),

  test("fromResult, error", () => {
    Promise.resolved(Error(4))
    ->Promise.Js.fromResult
    ->Promise.Js.catch(v => Promise.resolved(v == 4));
  }),

  test("allOk, ok", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let p3 = Promise.allOk([p1, p2]);
    r1(Ok(42));
    r2(Ok(43));
    p3->Promise.map((==)(Ok([42, 43])));
  }),

  test("allOk, error", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let p3 = Promise.allOk([p1, p2]);
    r1(Ok(42));
    r2(Error(43));
    p3->Promise.map((==)(Error(43)));
  }),

  test("allOk, fast fail", () => {
    let (p1, _) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let p3 = Promise.allOk([p1, p2]);
    r2(Error(43));
    p3->Promise.map((==)(Error(43)));
  }),

  test("allOk, multiple error", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let p3 = Promise.allOk([p1, p2]);
    r1(Error(42));
    r2(Error(43));
    p3->Promise.map((==)(Error(42)));
  }),

  test("allOk, empty", () => {
    Promise.allOk([])
    ->Promise.map(result => result == Ok([]));
  }),

  test("allOk2, ok", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let p3 = Promise.allOk2(p1, p2);
    r1(Ok(42));
    r2(Ok("43"));
    p3->Promise.map((==)(Ok((42, "43"))));
  }),

  test("allOk2, error", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let p3 = Promise.allOk2(p1, p2);
    r1(Ok(42));
    r2(Error("43"));
    p3->Promise.map((==)(Error("43")));
  }),

  test("allOk3, ok", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let (p3, r3) = Promise.pending();
    let p4 = Promise.allOk3(p1, p2, p3);
    r1(Ok(42));
    r2(Ok("43"));
    r3(Ok(44));
    p4->Promise.map((==)(Ok((42, "43", 44))));
  }),

  test("allOk3, error", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let (p3, _) = Promise.pending();
    let p4 = Promise.allOk3(p1, p2, p3);
    r1(Ok(42));
    r2(Error("43"));
    p4->Promise.map((==)(Error("43")));
  }),

  test("allOk4, ok", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let (p3, r3) = Promise.pending();
    let (p4, r4) = Promise.pending();
    let p5 = Promise.allOk4(p1, p2, p3, p4);
    r1(Ok(42));
    r2(Ok("43"));
    r3(Ok(44));
    r4(Ok(45));
    p5->Promise.map((==)(Ok((42, "43", 44, 45))));
  }),

  test("allOk4, error", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let (p3, _) = Promise.pending();
    let (p4, _) = Promise.pending();
    let p5 = Promise.allOk4(p1, p2, p3, p4);
    r1(Ok(42));
    r2(Error("43"));
    p5->Promise.map((==)(Error("43")));
  }),

  test("allOk5, ok", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let (p3, r3) = Promise.pending();
    let (p4, r4) = Promise.pending();
    let (p5, r5) = Promise.pending();
    let p6 = Promise.allOk5(p1, p2, p3, p4, p5);
    r1(Ok(42));
    r2(Ok("43"));
    r3(Ok(44));
    r4(Ok(45));
    r5(Ok(46));
    p6->Promise.map((==)(Ok((42, "43", 44, 45, 46))));
  }),

  test("allOk5, error", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let (p3, _) = Promise.pending();
    let (p4, _) = Promise.pending();
    let (p5, _) = Promise.pending();
    let p6 = Promise.allOk5(p1, p2, p3, p4, p5);
    r1(Ok(42));
    r2(Error("43"));
    p6->Promise.map((==)(Error("43")));
  }),

  test("allOk6, ok", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let (p3, r3) = Promise.pending();
    let (p4, r4) = Promise.pending();
    let (p5, r5) = Promise.pending();
    let (p6, r6) = Promise.pending();
    let p7 = Promise.allOk6(p1, p2, p3, p4, p5, p6);
    r1(Ok(42));
    r2(Ok("43"));
    r3(Ok(44));
    r4(Ok(45));
    r5(Ok(46));
    r6(Ok(47));
    p7->Promise.map((==)(Ok((42, "43", 44, 45, 46, 47))));
  }),

  test("allOk6, error", () => {
    let (p1, r1) = Promise.pending();
    let (p2, r2) = Promise.pending();
    let (p3, _) = Promise.pending();
    let (p4, _) = Promise.pending();
    let (p5, _) = Promise.pending();
    let (p6, _) = Promise.pending();
    let p7 = Promise.allOk6(p1, p2, p3, p4, p5, p6);
    r1(Ok(42));
    r2(Error("43"));
    p7->Promise.map((==)(Error("43")));
  }),
]);



let optionTests = Framework.suite("option", [
  test("mapSome, some", () => {
    Promise.resolved(Some(42))
    ->Promise.mapSome(n => n + 1)
    ->Promise.map(v => v == Some(43));
  }),

  test("mapSome, none", () => {
    Promise.resolved(None)
    ->Promise.mapSome(n => n + 1)
    ->Promise.map(v => v == None);
  }),

  test("getSome, some", () => {
    let (p, resolve) = Promise.pending();
    Promise.resolved(Some(42))->Promise.getSome(n => resolve(n + 1));
    p->Promise.map(n => n == 43);
  }),

  test("getSome, none", () => {
    let called = ref(false);
    Promise.resolved(None)->Promise.getSome(_ => called := true);
    Promise.resolved()->Promise.map(() => !called^);
  }),

  test("tapSome, some", () => {
    let correct = ref(false);
    Promise.resolved(Some(42))
    ->Promise.tapSome(n => correct := n == 42)
    ->Promise.map(result => result == Some(42) && correct^);
  }),

  test("tapSome, none", () => {
    let called = ref(false);
    Promise.resolved(None)
    ->Promise.tapSome(_ => called := true)
    ->Promise.map(result => result == None && !called^);
  }),

  test("flatMapSome, some", () => {
    Promise.resolved(Some(42))
    ->Promise.flatMapSome(n => Promise.resolved(Some(n + 1)))
    ->Promise.map(v => v == Some(43));
  }),

  test("flatMapSome, none", () => {
    Promise.resolved(None)
    ->Promise.flatMapSome(n => Promise.resolved(Some(n + 1)))
    ->Promise.map(v => v == None);
  }),
]);



let raiseTests = Framework.suite("raise", [
  test("stops, then", () => {
    let continued = ref(false);
    let p =
      Promise.resolved()
      ->Promise.flatMap(() => raise(Exit))
      ->Promise.flatMap(() => {
        continued := true;
        Promise.resolved(42);
      });
    remainsPending(p, 43);
  }),

  test("stops, catch", () => {
    let continued = ref(false);
    let p =
      Promise.Js.rejected()
      ->Promise.Js.catch(() => raise(Exit))
      ->Promise.Js.flatMap(() => {
        continued := true;
        Promise.resolved(42);
      });
    remainsPending(p, 43);
  }),
]);



let suites = [
  basicTests,
  rejectTests,
  allTests,
  raceTests,
  resultTests,
  optionTests,
  raiseTests,
];
