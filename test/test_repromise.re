let test = Framework.test;



let basicTests = Framework.suite("basic", [
  /* The basic [resolve]-[then_] tests are a bit useless, because the testing
     framework itself already uses both [resolved] and [then], i.e. every test
     implicitly tests those. However, we include these for completeness, in case
     we become enlightened and rewrite the framework in CPS or something.  */
  test("resolve", () => {
    Repromise.resolve(true);
  }),

  test("wait", () => {
    let correct = ref(false);
    Repromise.resolve(1)
    |> Repromise.wait(n => correct := (n == 1));
    Repromise.resolve()
    |> Repromise.map(() => correct^)
  }),

  test("then_", () => {
    Repromise.resolve(1)
    |> Repromise.then_ (n => Repromise.resolve (n == 1));
  }),

  test("map", () => {
    let p = Repromise.resolve(6) |> Repromise.map(v => v * 7);
    p |> Repromise.then_(r => Repromise.resolve(r == 42));
  }),

  test("map chain", () => {
    let p =
      Repromise.resolve(6)
      |> Repromise.map(v => v * 7)
      |> Repromise.map(r => r * 10);
    p |> Repromise.then_(r => Repromise.resolve(r == 420));
  }),

  test("map soundness", () => {
      Repromise.resolve(6)
      |> Repromise.map(v => v * 7)
      |> Repromise.map(x => Repromise.resolve(x == 42))
      |> Repromise.then_(r => r);
  }),

  test("then_ chain", () => {
    Repromise.resolve(1)
    |> Repromise.then_ (n => Repromise.resolve (n + 1))
    |> Repromise.then_ (n => Repromise.resolve (n == 2));
  }),

  test("then_ nested", () => {
    Repromise.resolve(1)
    |> Repromise.then_ (n =>
      Repromise.resolve (n + 1)
      |> Repromise.then_ (n => Repromise.resolve (n + 1)))
    |> Repromise.then_ (n => Repromise.resolve (n == 3));
  }),

  /* If promises are implemented on JS directly as ordinary JS promises,
     [resolve(resolve(42))] will collapse to just a [promise(int)], even though
     the Reason type is [promise(promise(int))]. This causes a soundness bug,
     because, due to the type, the callback of [then_] will expect the nested
     value to be a [promise(int)]. A correct implementation of Reason promises
     on JS will avoid this bug. */
  test("no collapsing", () => {
    Repromise.resolve(Repromise.resolve(1))
    |> Repromise.then_ (p =>
      p |> Repromise.then_ (n => Repromise.resolve (n == 1)));
  }),

  test("new_", () => {
    let (p, resolve) = Repromise.new_();
    resolve(true);
    p;
  }),

  test("defer", () => {
    let (p, resolve) = Repromise.new_();
    let p' = p |> Repromise.then_(n => Repromise.resolve(n == 1));
    resolve(1);
    p';
  }),

  test("double resolve", () => {
    let (p, resolve) = Repromise.new_();
    resolve(42);
    p |> Repromise.then_(n => {
      resolve(43);
      p |> Repromise.map(n' =>
        n == 42 && n' == 42)});
  }),

  test("callback order (already resolved)", () => {
    let firstCallbackCalled = ref(false);
    let p = Repromise.resolve();
    p |> Repromise.map(() => firstCallbackCalled := true) |> ignore;
    p |> Repromise.map(() => firstCallbackCalled^);
  }),

  test("callback order (resolved later)", () => {
    let firstCallbackCalled = ref(false);
    let secondCallbackCalledSecond = ref(false);
    let (p, resolve) = Repromise.new_();
    p |> Repromise.map(() => firstCallbackCalled := true) |> ignore;
    p |> Repromise.map(() =>
      secondCallbackCalledSecond := firstCallbackCalled^) |> ignore;
    resolve();
    p |> Repromise.map(() => secondCallbackCalledSecond^);
  }),

  test("relax", () => {
    let p = Repromise.resolve();
    Repromise.resolve(Repromise.Rejectable.relax(p) === p);
  }),
]);



let rejectTests = Framework.suite("reject", [
  test("new_", () => {
    let (p, _, reject) = Repromise.Rejectable.new_();
    reject(1);
    p |> Repromise.Rejectable.catch(n => Repromise.resolve(n == 1));
  }),

  test("reject, catch", () => {
    Repromise.Rejectable.reject("foo")
    |> Repromise.Rejectable.catch(s => Repromise.resolve(s == "foo"));
  }),

  test("catch chosen", () => {
    Repromise.Rejectable.reject("foo")
    |> Repromise.Rejectable.catch(s => Repromise.resolve(s == "foo"));
  }),

  test("then_, reject, catch", () => {
    Repromise.Rejectable.resolve(1)
    |> Repromise.Rejectable.then_(n => Repromise.Rejectable.reject(n + 1))
    |> Repromise.Rejectable.catch(n => Repromise.resolve(n == 2));
  }),

  test("reject, catch, then_", () => {
    Repromise.Rejectable.reject(1)
    |> Repromise.Rejectable.catch(n => Repromise.resolve(n + 1))
    |> Repromise.then_(n => Repromise.resolve(n == 2));
  }),

  test("no double catch", () => {
    Repromise.Rejectable.reject("foo")
    |> Repromise.Rejectable.catch(s => Repromise.resolve(s == "foo"))
    |> Repromise.Rejectable.catch((_) => Repromise.resolve(false));
  }),

  test("catch chain", () => {
    Repromise.Rejectable.reject(1)
    |> Repromise.Rejectable.catch(n => Repromise.Rejectable.reject(n + 1))
    |> Repromise.Rejectable.catch(n => Repromise.resolve(n == 2));
  }),

  test("no catching resolved", () => {
    Repromise.resolve(true)
    |> Repromise.Rejectable.catch((_) => Repromise.resolve(false));
  }),

  test("no catching resolved, after then_", () => {
    Repromise.resolve()
    |> Repromise.then_(() => Repromise.resolve(true))
    |> Repromise.Rejectable.catch((_) => Repromise.resolve(false));
  }),
]);



let remainsPending = (p, dummyValue) => {
  let rec repeat = (n, f) =>
    if (n == 0) {
      Repromise.resolve(true);
    }
    else {
      f ()
      |> Repromise.then_(result =>
        if (not(result)) {
          Repromise.resolve(false);
        }
        else {
          repeat(n - 1, f);
        })
    };

  repeat(10, () =>
    Repromise.race([p, Repromise.resolve(dummyValue)])
    |> Repromise.then_(v1 =>
      Repromise.race([Repromise.resolve(dummyValue), p])
      |> Repromise.map(v2 =>
        v1 == dummyValue && v2 == dummyValue)));
};

let allTests = Framework.suite("all", [
  test("already resolved", () => {
    Repromise.all([Repromise.resolve(42), Repromise.resolve(43)])
    |> Repromise.map(results => results == [42, 43]);
  }),

  test("resolved later", () => {
    let (p1, resolveP1) = Repromise.new_();
    let (p2, resolveP2) = Repromise.new_();
    let p3 = Repromise.all([p1, p2]);
    resolveP1(42);
    resolveP2(43);
    p3 |> Repromise.map(results => results == [42, 43]);
  }),

  test("not all resolved", () => {
    let (p1, resolveP1) = Repromise.new_();
    let (p2, _) = Repromise.new_();
    let p3 = Repromise.all([p1, p2]);
    resolveP1(42);
    remainsPending(p3, []);
  }),

  test("simultaneous resolve", () => {
    let (p1, resolveP1) = Repromise.new_();
    let p2 = Repromise.all([p1, p1]);
    resolveP1(42);
    p2 |> Repromise.map(results => results == [42, 42]);
  }),

  test("already rejected", () => {
    let (p1, _, _) = Repromise.Rejectable.new_();
    let p2 = Repromise.Rejectable.all([p1, Repromise.Rejectable.reject(43)]);
    p2
    |> Repromise.Rejectable.then_((_) => Repromise.Rejectable.resolve(false))
    |> Repromise.Rejectable.catch(n => Repromise.resolve(n == 43));
  }),

  test("rejected later", () => {
    let (p1, _, rejectP1) = Repromise.Rejectable.new_();
    let (p2, _, _) = Repromise.Rejectable.new_();
    let p3 = Repromise.Rejectable.all([p1, p2]);
    rejectP1(42);
    p3
    |> Repromise.Rejectable.then_((_) => Repromise.Rejectable.resolve(false))
    |> Repromise.Rejectable.catch(n => Repromise.resolve(n == 42));
  }),

  test("remains rejected", () => {
    let (p1, _, rejectP1) = Repromise.Rejectable.new_();
    let (p2, resolveP2, _) = Repromise.Rejectable.new_();
    let p3 = Repromise.Rejectable.all([p1, p2]);
    rejectP1(42);
    resolveP2(43);
    p2
    |> Repromise.Rejectable.catch((_) => assert false)
    |> Repromise.Rejectable.then_((_) =>
      p3
      |> Repromise.Rejectable.then_((_) => Repromise.Rejectable.resolve(false))
      |> Repromise.Rejectable.catch(n => Repromise.resolve(n == 42)));
  }),

  test("empty", () => {
    let p = Repromise.all([]);
    remainsPending(p, []);
  }),
]);



let raceTests = Framework.suite("race", [
  test("first resolves", () => {
    let (p1, resolveP1) = Repromise.new_();
    let (p2, _) = Repromise.new_();
    let p3 = Repromise.race([p1, p2]);
    resolveP1(42);
    p3 |> Repromise.map(n => n == 42);
  }),

  test("second resolves", () => {
    let (p1, _) = Repromise.new_();
    let (p2, resolveP2) = Repromise.new_();
    let p3 = Repromise.race([p1, p2]);
    resolveP2(43);
    p3 |> Repromise.map(n => n == 43);
  }),

  test("first resolves first", () => {
    let (p1, resolveP1) = Repromise.new_();
    let (p2, resolveP2) = Repromise.new_();
    let p3 = Repromise.race([p1, p2]);
    resolveP1(42);
    resolveP2(43);
    p3 |> Repromise.map(n => n == 42);
  }),

  test("second resolves first", () => {
    let (p1, resolveP1) = Repromise.new_();
    let (p2, resolveP2) = Repromise.new_();
    let p3 = Repromise.race([p1, p2]);
    resolveP2(43);
    resolveP1(42);
    p3 |> Repromise.map(n => n == 43);
  }),

  test("rejection", () => {
    let (p1, _, rejectP1) = Repromise.Rejectable.new_();
    let (p2, _, _) = Repromise.Rejectable.new_();
    let p3 = Repromise.Rejectable.race([p1, p2]);
    rejectP1(42);
    p3 |> Repromise.Rejectable.catch(n => Repromise.resolve(n == 42));
  }),

  test("already resolved", () => {
    let (p1, _) = Repromise.new_();
    let p2 = Repromise.resolve(43);
    let p3 = Repromise.race([p1, p2]);
    p3 |> Repromise.map(n => n == 43);
  }),

  test("already rejected", () => {
    let (p1, _, _) = Repromise.Rejectable.new_();
    let p2 = Repromise.Rejectable.reject(43);
    let p3 = Repromise.Rejectable.race([p1, p2]);
    p3 |> Repromise.Rejectable.catch(n => Repromise.resolve(n == 43));
  }),

  test("two resolved", () => {
    let p1 = Repromise.resolve(42);
    let p2 = Repromise.resolve(43);
    let p3 = Repromise.race([p1, p2]);
    p3 |> Repromise.map(n => n == 42 || n == 43);
  }),

  test("forever pending", () => {
    let (p1, _) = Repromise.new_();
    let (p2, _) = Repromise.new_();
    let p3 = Repromise.race([p1, p2]);
    remainsPending(p3, 43);
  }),

  test("simultaneous resolve", () => {
    let (p1, resolveP1) = Repromise.new_();
    let p2 = Repromise.race([p1, p1]);
    resolveP1(42);
    p2 |> Repromise.map(n => n == 42);
  }),

  /* This test is temporarily disabled due to
       https://github.com/BuckleScript/bucklescript/issues/2692

  test("empty", () => {
    try ({ ignore(Repromise.race([])); Repromise.resolve(false); }) {
    | Invalid_argument(_) => Repromise.resolve(true);
    };
  }), */

  /* This test is for an implementation detail. When a pending promise p is
     returned by the callback of then_, the native implementation (and
     non-memory-leaking JavaScript implementations) will move the callbacks
     attached to p into the list attached to the outer promise of then_. We want
     to make sure that callbacks attached by race survive this moving. For that,
     p has to be involved in a call to race. */
  test("race, then callbacks moved", () => {
    let (p, resolve) = Repromise.new_();
    let final = Repromise.race([p]);

    /* We are using this resolve() just so we can call then_ on it, guaranteeing
       that the second time will run after the first time.. */
    let delay = Repromise.resolve();

    delay
    |> Repromise.then_(fun () => p)
    |> ignore;

    delay
    |> Repromise.then_(fun () => {
      resolve(42);
      /* This tests now succeeds only if resolving p resolved final^, despite
         the fact that p was returned to then_ while still a pending promise. */
      final |> Repromise.map(n => n == 42);
    });
  }),

  /* Similar to the preceding test, but the race callback is attached to p after
     its callback list has been merged with the outer promise of then_. */
  test("callbacks moved, then race", () => {
    let (p, resolve) = Repromise.new_();

    let delay = Repromise.resolve();

    delay
    |> Repromise.then_(fun () => p)
    |> ignore;

    delay
    |> Repromise.then_(fun () => {
      let final = Repromise.race([p]);
      resolve(42);
      final |> Repromise.map(n => n == 42);
    });
  }),
]);



let suites = [basicTests, rejectTests, allTests, raceTests];
