/* This file is part of Repromise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/repromise/blob/master/LICENSE.md. */



let test = Framework.test;
open! Repromise.FastPipe;



let basicTests = Framework.suite("basic", [
  /* The basic [resolved]-[flatMap] tests are a bit useless, because the testing
     framework itself already uses both [resolved] and [then], i.e. every test
     implicitly tests those. However, we include these for completeness, in case
     we become enlightened and rewrite the framework in CPS or something.  */
  test("resolved", () => {
    Repromise.resolved(true);
  }),

  test("on", () => {
    let correct = ref(false);
    Repromise.resolved(1)->Repromise.on(n => correct := (n == 1));
    Repromise.resolved()->Repromise.map(() => correct^);
  }),

  test("tap", () => {
    let correct = ref(false);
    Repromise.resolved(1)
    ->Repromise.tap(n => correct := (n == 1))
    ->Repromise.map(n => n == 1 && correct^);
  }),

  test("flatMap", () => {
    Repromise.resolved(1)
    ->Repromise.flatMap(n => Repromise.resolved(n == 1));
  }),

  test("map", () => {
    let p = Repromise.resolved(6)->Repromise.map(v => v * 7);
    p->Repromise.flatMap(r => Repromise.resolved(r == 42));
  }),

  test("map chain", () => {
    let p =
      Repromise.resolved(6)
      ->Repromise.map(v => v * 7)
      ->Repromise.map(r => r * 10);
    p->Repromise.flatMap(r => Repromise.resolved(r == 420));
  }),

  test("map soundness", () => {
      Repromise.resolved(6)
      ->Repromise.map(v => v * 7)
      ->Repromise.map(x => Repromise.resolved(x == 42))
      ->Repromise.flatMap(r => r);
  }),

  test("flatMap chain", () => {
    Repromise.resolved(1)
    ->Repromise.flatMap(n => Repromise.resolved(n + 1))
    ->Repromise.flatMap(n => Repromise.resolved(n == 2));
  }),

  test("flatMap nested", () => {
    Repromise.resolved(1)
    ->Repromise.flatMap (n =>
      Repromise.resolved(n + 1)
      ->Repromise.flatMap (n => Repromise.resolved(n + 1)))
    ->Repromise.flatMap (n => Repromise.resolved(n == 3));
  }),

  /* If promises are implemented on JS directly as ordinary JS promises,
     [resolved(resolved(42))] will collapse to just a [promise(int)], even
     though the Reason type is [promise(promise(int))]. This causes a soundness
     bug, because, due to the type, the callback of [flatMap] will expect the
     nested value to be a [promise(int)]. A correct implementation of Reason
     promises on JS will avoid this bug. */
  test("no collapsing", () => {
    Repromise.resolved(Repromise.resolved(1))
    ->Repromise.flatMap(p =>
      p->Repromise.flatMap(n => Repromise.resolved(n == 1)));
  }),

  test("make", () => {
    let (p, resolve) = Repromise.make();
    resolve(true);
    p;
  }),

  test("defer", () => {
    let (p, resolve) = Repromise.make();
    let p' = p->Repromise.flatMap(n => Repromise.resolved(n == 1));
    resolve(1);
    p';
  }),

  test("double resolve", () => {
    let (p, resolve) = Repromise.make();
    resolve(42);
    p->Repromise.flatMap(n => {
      resolve(43);
      p->Repromise.map(n' =>
        n == 42 && n' == 42)});
  }),

  test("exec", () => {
    Repromise.exec(resolve => resolve(true));
  }),

  test("callback order (already resolved)", () => {
    let firstCallbackCalled = ref(false);
    let p = Repromise.resolved();
    p->Repromise.map(() => firstCallbackCalled := true) |> ignore;
    p->Repromise.map(() => firstCallbackCalled^);
  }),

  test("callback order (resolved later)", () => {
    let firstCallbackCalled = ref(false);
    let secondCallbackCalledSecond = ref(false);
    let (p, resolve) = Repromise.make();
    p->Repromise.map(() => firstCallbackCalled := true) |> ignore;
    p->Repromise.map(() =>
      secondCallbackCalledSecond := firstCallbackCalled^) |> ignore;
    resolve();
    p->Repromise.map(() => secondCallbackCalledSecond^);
  }),

  test("relax", () => {
    let p = Repromise.resolved();
    Repromise.resolved(Repromise.Rejectable.relax(p) === p);
  }),
]);



let rejectTests = Framework.suite("reject", [
  test("make", () => {
    let (p, _, reject) = Repromise.Rejectable.make();
    reject(1);
    p |> Repromise.Rejectable.catch(n => Repromise.resolved(n == 1));
  }),

  test("reject, catch", () => {
    Repromise.Rejectable.rejected("foo")
    |> Repromise.Rejectable.catch(s => Repromise.resolved(s == "foo"));
  }),

  test("catch chosen", () => {
    Repromise.Rejectable.rejected("foo")
    |> Repromise.Rejectable.catch(s => Repromise.resolved(s == "foo"));
  }),

  test("flatMap, reject, catch", () => {
    Repromise.Rejectable.resolved(1)
    ->Repromise.Rejectable.flatMap(n => Repromise.Rejectable.rejected(n + 1))
    |> Repromise.Rejectable.catch(n => Repromise.resolved(n == 2));
  }),

  test("reject, catch, flatMap", () => {
    (Repromise.Rejectable.rejected(1)
    |> Repromise.Rejectable.catch(n => Repromise.resolved(n + 1)))
    ->Repromise.flatMap(n => Repromise.resolved(n == 2));
  }),

  test("no double catch", () => {
    Repromise.Rejectable.rejected("foo")
    |> Repromise.Rejectable.catch(s => Repromise.resolved(s == "foo"))
    |> Repromise.Rejectable.catch((_) => Repromise.resolved(false));
  }),

  test("catch chain", () => {
    Repromise.Rejectable.rejected(1)
    |> Repromise.Rejectable.catch(n => Repromise.Rejectable.rejected(n + 1))
    |> Repromise.Rejectable.catch(n => Repromise.resolved(n == 2));
  }),

  test("no catching resolved", () => {
    Repromise.resolved(true)
    |> Repromise.Rejectable.catch((_) => Repromise.resolved(false));
  }),

  test("no catching resolved, after flatMap", () => {
    Repromise.resolved()
    ->Repromise.flatMap(() => Repromise.resolved(true))
    |> Repromise.Rejectable.catch((_) => Repromise.resolved(false));
  }),
]);



let remainsPending = (p, dummyValue) => {
  let rec repeat = (n, f) =>
    if (n == 0) {
      Repromise.resolved(true);
    }
    else {
      f ()
      ->Repromise.flatMap(result =>
        if (!result) {
          Repromise.resolved(false);
        }
        else {
          repeat(n - 1, f);
        })
    };

  repeat(10, () =>
    Repromise.race([p, Repromise.resolved(dummyValue)])
    ->Repromise.flatMap(v1 =>
      Repromise.race([Repromise.resolved(dummyValue), p])
      ->Repromise.map(v2 =>
        v1 == dummyValue && v2 == dummyValue)));
};

let allTests = Framework.suite("all", [
  test("already resolved", () => {
    Repromise.all([Repromise.resolved(42), Repromise.resolved(43)])
    ->Repromise.map(results => results == [42, 43]);
  }),

  test("resolved later", () => {
    let (p1, resolveP1) = Repromise.make();
    let (p2, resolveP2) = Repromise.make();
    let p3 = Repromise.all([p1, p2]);
    resolveP1(42);
    resolveP2(43);
    p3->Repromise.map(results => results == [42, 43]);
  }),

  test("not all resolved", () => {
    let (p1, resolveP1) = Repromise.make();
    let (p2, _) = Repromise.make();
    let p3 = Repromise.all([p1, p2]);
    resolveP1(42);
    remainsPending(p3, []);
  }),

  test("simultaneous resolve", () => {
    let (p1, resolveP1) = Repromise.make();
    let p2 = Repromise.all([p1, p1]);
    resolveP1(42);
    p2->Repromise.map(results => results == [42, 42]);
  }),

  test("already rejected", () => {
    let (p1, _, _) = Repromise.Rejectable.make();
    let p2 = Repromise.Rejectable.all([p1, Repromise.Rejectable.rejected(43)]);
    p2
    ->Repromise.Rejectable.flatMap((_) => Repromise.Rejectable.resolved(false))
    |> Repromise.Rejectable.catch(n => Repromise.resolved(n == 43));
  }),

  test("rejected later", () => {
    let (p1, _, rejectP1) = Repromise.Rejectable.make();
    let (p2, _, _) = Repromise.Rejectable.make();
    let p3 = Repromise.Rejectable.all([p1, p2]);
    rejectP1(42);
    p3
    ->Repromise.Rejectable.flatMap((_) => Repromise.Rejectable.resolved(false))
    |> Repromise.Rejectable.catch(n => Repromise.resolved(n == 42));
  }),

  test("remains rejected", () => {
    let (p1, _, rejectP1) = Repromise.Rejectable.make();
    let (p2, resolveP2, _) = Repromise.Rejectable.make();
    let p3 = Repromise.Rejectable.all([p1, p2]);
    rejectP1(42);
    resolveP2(43);
    (p2
    |> Repromise.Rejectable.catch((_) => assert false))
    ->Repromise.Rejectable.flatMap((_) =>
      p3
      ->Repromise.Rejectable.flatMap((_) =>
        Repromise.Rejectable.resolved(false))
      |> Repromise.Rejectable.catch(n => Repromise.resolved(n == 42)));
  }),

  test("empty", () => {
    let p = Repromise.all([]);
    remainsPending(p, []);
  }),

  test("all2", () => {
    let (p1, resolveP1) = Repromise.make();
    let (p2, resolveP2) = Repromise.make();
    let result =
      Repromise.all2(p1, p2)
      ->Repromise.map(((x, y)) => x == 42 && y == 43);
    resolveP1(42);
    resolveP2(43);
    result;
  }),

  test("all3", () => {
    let (p1, resolveP1) = Repromise.make();
    let (p2, resolveP2) = Repromise.make();
    let (p3, resolveP3) = Repromise.make();
    let result =
      Repromise.all3(p1, p2, p3)
      ->Repromise.map(((x, y, z)) => x == 42 && y == 43 && z == 44);
    resolveP1(42);
    resolveP2(43);
    resolveP3(44);
    result;
  }),

  test("all4", () => {
    let (p1, resolveP1) = Repromise.make();
    let (p2, resolveP2) = Repromise.make();
    let (p3, resolveP3) = Repromise.make();
    let (p4, resolveP4) = Repromise.make();
    let result =
      Repromise.all4(p1, p2, p3, p4)
      ->Repromise.map(((x, y, z, u)) =>
        x == 42 && y == 43 && z == 44 && u == 45);
    resolveP1(42);
    resolveP2(43);
    resolveP3(44);
    resolveP4(45);
    result;
  }),

  test("all5", () => {
    let (p1, resolveP1) = Repromise.make();
    let (p2, resolveP2) = Repromise.make();
    let (p3, resolveP3) = Repromise.make();
    let (p4, resolveP4) = Repromise.make();
    let (p5, resolveP5) = Repromise.make();
    let result =
      Repromise.all5(p1, p2, p3, p4, p5)
      ->Repromise.map(((x, y, z, u, v)) =>
        x == 42 && y == 43 && z == 44 && u == 45 && v == 46);
    resolveP1(42);
    resolveP2(43);
    resolveP3(44);
    resolveP4(45);
    resolveP5(46);
    result;
  }),

  test("all6", () => {
    let (p1, resolveP1) = Repromise.make();
    let (p2, resolveP2) = Repromise.make();
    let (p3, resolveP3) = Repromise.make();
    let (p4, resolveP4) = Repromise.make();
    let (p5, resolveP5) = Repromise.make();
    let (p6, resolveP6) = Repromise.make();
    let result =
      Repromise.all6(p1, p2, p3, p4, p5, p6)
      ->Repromise.map(((x, y, z, u, v, w)) =>
        x == 42 && y == 43 && z == 44 && u == 45 && v == 46 && w == 47);
    resolveP1(42);
    resolveP2(43);
    resolveP3(44);
    resolveP4(45);
    resolveP5(46);
    resolveP6(47);
    result;
  }),

  test("arrayAll", () => {
    let (p1, resolveP1) = Repromise.make();
    let (p2, resolveP2) = Repromise.make();
    let result =
      Repromise.arrayAll([|p1, p2|])
      ->Repromise.map(fun
        | [|x, y|] => x == 42 && y == 43
        | _ => false);
    resolveP1(42);
    resolveP2(43);
    result;
  }),
]);



let raceTests = Framework.suite("race", [
  test("first resolves", () => {
    let (p1, resolveP1) = Repromise.make();
    let (p2, _) = Repromise.make();
    let p3 = Repromise.race([p1, p2]);
    resolveP1(42);
    p3->Repromise.map(n => n == 42);
  }),

  test("second resolves", () => {
    let (p1, _) = Repromise.make();
    let (p2, resolveP2) = Repromise.make();
    let p3 = Repromise.race([p1, p2]);
    resolveP2(43);
    p3->Repromise.map(n => n == 43);
  }),

  test("first resolves first", () => {
    let (p1, resolveP1) = Repromise.make();
    let (p2, resolveP2) = Repromise.make();
    let p3 = Repromise.race([p1, p2]);
    resolveP1(42);
    resolveP2(43);
    p3->Repromise.map(n => n == 42);
  }),

  test("second resolves first", () => {
    let (p1, resolveP1) = Repromise.make();
    let (p2, resolveP2) = Repromise.make();
    let p3 = Repromise.race([p1, p2]);
    resolveP2(43);
    resolveP1(42);
    p3->Repromise.map(n => n == 43);
  }),

  test("rejection", () => {
    let (p1, _, rejectP1) = Repromise.Rejectable.make();
    let (p2, _, _) = Repromise.Rejectable.make();
    let p3 = Repromise.Rejectable.race([p1, p2]);
    rejectP1(42);
    p3 |> Repromise.Rejectable.catch(n => Repromise.resolved(n == 42));
  }),

  test("already resolved", () => {
    let (p1, _) = Repromise.make();
    let p2 = Repromise.resolved(43);
    let p3 = Repromise.race([p1, p2]);
    p3->Repromise.map(n => n == 43);
  }),

  test("already rejected", () => {
    let (p1, _, _) = Repromise.Rejectable.make();
    let p2 = Repromise.Rejectable.rejected(43);
    let p3 = Repromise.Rejectable.race([p1, p2]);
    p3 |> Repromise.Rejectable.catch(n => Repromise.resolved(n == 43));
  }),

  test("two resolved", () => {
    let p1 = Repromise.resolved(42);
    let p2 = Repromise.resolved(43);
    let p3 = Repromise.race([p1, p2]);
    p3->Repromise.map(n => n == 42 || n == 43);
  }),

  test("forever pending", () => {
    let (p1, _) = Repromise.make();
    let (p2, _) = Repromise.make();
    let p3 = Repromise.race([p1, p2]);
    remainsPending(p3, 43);
  }),

  test("simultaneous resolve", () => {
    let (p1, resolveP1) = Repromise.make();
    let p2 = Repromise.race([p1, p1]);
    resolveP1(42);
    p2->Repromise.map(n => n == 42);
  }),

  test("empty", () => {
    try ({ ignore(Repromise.race([])); Repromise.resolved(false); }) {
    | Invalid_argument(_) => Repromise.resolved(true);
    };
  }),

  /* This test is for an implementation detail. When a pending promise p is
     returned by the callback of flatMap, the native implementation (and
     non-memory-leaking JavaScript implementations) will move the callbacks
     attached to p into the list attached to the outer promise of flatMap. We
     want to make sure that callbacks attached by race survive this moving. For
     that, p has to be involved in a call to race. */
  test("race, then callbacks moved", () => {
    let (p, resolve) = Repromise.make();
    let final = Repromise.race([p]);

    /* We are using this resolve() just so we can call flatMap on it,
       guaranteeing that the second time will run after the first time.. */
    let delay = Repromise.resolved();

    ignore(delay->Repromise.flatMap(fun () => p));

    delay->Repromise.flatMap(fun () => {
      resolve(42);
      /* This tests now succeeds only if resolving p resolved final^, despite
         the fact that p was returned to flatMap while still a pending
         promise. */
      final->Repromise.map(n => n == 42);
    });
  }),

  /* Similar to the preceding test, but the race callback is attached to p after
     its callback list has been merged with the outer promise of flatMap. */
  test("callbacks moved, then race", () => {
    let (p, resolve) = Repromise.make();

    let delay = Repromise.resolved();

    ignore(delay->Repromise.flatMap(fun () => p));

    delay
    ->Repromise.flatMap(fun () => {
      let final = Repromise.race([p]);
      resolve(42);
      final->Repromise.map(n => n == 42);
    });
  }),
]);



/* Compatibility with BukleScript < 6. */
open! Isoresult;

let resultTests = Framework.suite("result", [
  test("mapOk, ok", () => {
    Repromise.resolved(Ok(42))
    ->Repromise.mapOk(n => n + 1)
    ->Repromise.map(v => v == Ok(43));
  }),

  test("mapOk, error", () => {
    Repromise.resolved(Error(42))
    ->Repromise.mapOk(n => n + 1)
    ->Repromise.map(v => v == Error(42));
  }),

  test("mapError, ok", () => {
    Repromise.resolved(Ok(42))
    ->Repromise.mapError(n => n + 1)
    ->Repromise.map(v => v == Ok(42));
  }),

  test("mapError, error", () => {
    Repromise.resolved(Error(42))
    ->Repromise.mapError(n => n + 1)
    ->Repromise.map(v => v == Error(43));
  }),

  test("onOk, ok", () => {
    let (p, resolve) = Repromise.make();
    Repromise.resolved(Ok(42))->Repromise.onOk(n => resolve(n + 1));
    p->Repromise.map(n => n == 43);
  }),

  test("onOk, error", () => {
    let called = ref(false);
    Repromise.resolved(Error(42))->Repromise.onOk(_ => called := true);
    Repromise.resolved()->Repromise.map(() => !called^);
  }),

  test("onError, ok", () => {
    let called = ref(false);
    Repromise.resolved(Ok(42))->Repromise.onError(_ => called := true);
    Repromise.resolved()->Repromise.map(() => !called^);
  }),

  test("onError, error", () => {
    let (p, resolve) = Repromise.make();
    Repromise.resolved(Error(42))->Repromise.onError(n => resolve(n + 1));
    p->Repromise.map(n => n == 43);
  }),

  test("tapOk, ok", () => {
    let correct = ref(false);
    Repromise.resolved(Ok(42))
    ->Repromise.tapOk(n => correct := n == 42)
    ->Repromise.map(result => result == Ok(42) && correct^);
  }),

  test("tapOk, error", () => {
    let called = ref(false);
    Repromise.resolved(Error(42))
    ->Repromise.tapOk(_ => called := true)
    ->Repromise.map(result => result == Error(42) && !called^);
  }),

  test("tapError, ok", () => {
    let called = ref(false);
    Repromise.resolved(Ok(42))
    ->Repromise.tapError(_ => called := true)
    ->Repromise.map(result => result == Ok(42) && !called^);
  }),

  test("onError, error", () => {
    let correct = ref(false);
    Repromise.resolved(Error(42))
    ->Repromise.tapError(n => correct := n == 42)
    ->Repromise.map(result => result == Error(42) && correct^);
  }),

  test("flatMapOk, ok", () => {
    Repromise.resolved(Ok(42))
    ->Repromise.flatMapOk(n => Repromise.resolved(Ok(n + 1)))
    ->Repromise.map(v => v == Ok(43));
  }),

  test("flatMapOk, error", () => {
    Repromise.resolved(Error(42))
    ->Repromise.flatMapOk(n => Repromise.resolved(Ok(n + 1)))
    ->Repromise.map(v => v == Error(42));
  }),

  test("flatMapError, ok", () => {
    Repromise.resolved(Ok(42))
    ->Repromise.flatMapError(n => Repromise.resolved(Error(n + 1)))
    ->Repromise.map(v => v == Ok(42));
  }),

  test("flatMapError, error", () => {
    Repromise.resolved(Error(42))
    ->Repromise.flatMapError(n => Repromise.resolved(Error(n + 1)))
    ->Repromise.map(v => v == Error(43));
  }),

  test(">|=, ok", () => {
    let open Repromise.Operators;
    (Repromise.resolved(Ok(42))
    >|= (n => n + 1))
    ->Repromise.map(v => v == Ok(43));
  }),

  test(">|=, error", () => {
    let open Repromise.Operators;
    (Repromise.resolved(Error(42))
    >|= (n => n + 1))
    ->Repromise.map(v => v == Error(42));
  }),

  test(">>=, ok", () => {
    let open Repromise.Operators;
    (Repromise.resolved(Ok(42))
    >>= (n => Repromise.resolved(Ok(n + 1))))
    ->Repromise.map(v => v == Ok(43));
  }),

  test(">>=, error", () => {
    let open Repromise.Operators;
    (Repromise.resolved(Error(42))
    >>= (n => Repromise.resolved(Ok(n + 1))))
    ->Repromise.map(v => v == Error(42));
  }),
]);



let optionTests = Framework.suite("opton", [
  test("mapSome, some", () => {
    Repromise.resolved(Some(42))
    ->Repromise.mapSome(n => n + 1)
    ->Repromise.map(v => v == Some(43));
  }),

  test("mapSome, none", () => {
    Repromise.resolved(None)
    ->Repromise.mapSome(n => n + 1)
    ->Repromise.map(v => v == None);
  }),

  test("onSome, some", () => {
    let (p, resolve) = Repromise.make();
    Repromise.resolved(Some(42))->Repromise.onSome(n => resolve(n + 1));
    p->Repromise.map(n => n == 43);
  }),

  test("onSome, none", () => {
    let called = ref(false);
    Repromise.resolved(None)->Repromise.onSome(_ => called := true);
    Repromise.resolved()->Repromise.map(() => !called^);
  }),

  test("tapSome, some", () => {
    let correct = ref(false);
    Repromise.resolved(Some(42))
    ->Repromise.tapSome(n => correct := n == 42)
    ->Repromise.map(result => result == Some(42) && correct^);
  }),

  test("tapSome, none", () => {
    let called = ref(false);
    Repromise.resolved(None)
    ->Repromise.tapSome(_ => called := true)
    ->Repromise.map(result => result == None && !called^);
  }),

  test("flatMapSome, some", () => {
    Repromise.resolved(Some(42))
    ->Repromise.flatMapSome(n => Repromise.resolved(Some(n + 1)))
    ->Repromise.map(v => v == Some(43));
  }),

  test("flatMapSome, none", () => {
    Repromise.resolved(None)
    ->Repromise.flatMapSome(n => Repromise.resolved(Some(n + 1)))
    ->Repromise.map(v => v == None);
  }),
]);



let suites = [
  basicTests,
  rejectTests,
  allTests,
  raceTests,
  resultTests,
  optionTests
];
