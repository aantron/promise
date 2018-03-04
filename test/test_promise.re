let test = Framework.test;



let basicTests = Framework.suite("basic", [
  /* The basic [resolve]-[then_] tests are a bit useless, because the testing
     framework itself already uses both [resolved] and [then], i.e. every test
     implicitly tests those. However, we include these for completeness, in case
     we become enlightened and rewrite the framework in CPS or something.  */
  test("resolve", () => {
    Repromise.resolve(true);
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

  test("await", () => {
    let%await n = Repromise.resolve(1);
    Repromise.resolve(n == 1);
  }),

  test("then_ chain", () => {
    Repromise.resolve(1)
    |> Repromise.then_ (n => Repromise.resolve (n + 1))
    |> Repromise.then_ (n => Repromise.resolve (n == 2));
  }),

  test("await chain", () => {
    let%await n = Repromise.resolve(1);
    let%await n = Repromise.resolve(n + 1);
    Repromise.resolve(n == 2);
  }),

  test("then_ nested", () => {
    Repromise.resolve(1)
    |> Repromise.then_ (n =>
      Repromise.resolve (n + 1)
      |> Repromise.then_ (n => Repromise.resolve (n + 1)))
    |> Repromise.then_ (n => Repromise.resolve (n == 3));
  }),

  test("await nested", () => {
    let%await n = {
      let%await n = Repromise.resolve(1);
      Repromise.resolve(n + 1);
    };
    Repromise.resolve(n == 2);
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
    Repromise.new_((resolve, _) => resolve(true));
  }),

  test("defer", () => {
    let resolve_p = ref(ignore);
    let p =
      Repromise.new_((resolve, _) => resolve_p := resolve)
      |> Repromise.then_(n => Repromise.resolve(n == 1));
    resolve_p^(1);
    p;
  }),

  test("await defer", () => {
    let resolve_p = ref(ignore);
    let p = {
      let%await n = Repromise.new_((resolve, _) => resolve_p := resolve);
      Repromise.resolve(n == 1);
    };
    resolve_p^(1);
    p;
  }),
]);



let rejectTests = Framework.suite("reject", [
  test("new_", () => {
    Repromise.new_((_, reject) => reject(1))
    |> Repromise.catch(n => Repromise.resolve(n == 1));
  }),

  test("reject, catch", () => {
    Repromise.reject("foo")
    |> Repromise.catch(s => Repromise.resolve(s == "foo"));
  }),

  test("catch chosen", () => {
    Repromise.reject("foo")
    |> Repromise.then_((_) => Repromise.resolve(false))
    |> Repromise.catch(s => Repromise.resolve(s == "foo"));
  }),

  test("then_, reject, catch", () => {
    Repromise.resolve(1)
    |> Repromise.then_(n => Repromise.reject(n + 1))
    |> Repromise.catch(n => Repromise.resolve(n == 2));
  }),

  test("reject, catch, then_", () => {
    Repromise.reject(1)
    |> Repromise.catch(n => Repromise.resolve(n + 1))
    |> Repromise.then_(n => Repromise.resolve(n == 2));
  }),

  test("no double catch", () => {
    Repromise.reject("foo")
    |> Repromise.catch(s => Repromise.resolve(s == "foo"))
    |> Repromise.catch((_) => Repromise.resolve(false));
  }),

  test("catch chain", () => {
    Repromise.reject(1)
    |> Repromise.catch(n => Repromise.reject(n + 1))
    |> Repromise.catch(n => Repromise.resolve(n == 2));
  }),

  test("no catching resolved", () => {
    Repromise.resolve(true)
    |> Repromise.catch((_) => Repromise.resolve(false));
  }),

  test("no catching resolved, after then_", () => {
    Repromise.resolve()
    |> Repromise.then_(() => Repromise.resolve(true))
    |> Repromise.catch((_) => Repromise.resolve(false));
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

let raceTests = Framework.suite("race", [
  test("first resolves", () => {
    let resolveP1 = ref(ignore);
    let p1 = Repromise.new_((resolve, _reject) => resolveP1 := resolve);
    let p2 = Repromise.new_((_resolve, _reject) => ());
    let p3 = Repromise.race([p1, p2]);
    resolveP1^(42);
    p3 |> Repromise.map(n => n == 42);
  }),

  test("second resolves", () => {
    let resolveP2 = ref(ignore);
    let p1 = Repromise.new_((_resolve, _reject) => ());
    let p2 = Repromise.new_((resolve, _reject) => resolveP2 := resolve);
    let p3 = Repromise.race([p1, p2]);
    resolveP2^(43);
    p3 |> Repromise.map(n => n == 43);
  }),

  test("first resolves first", () => {
    let resolveP1 = ref(ignore);
    let resolveP2 = ref(ignore);
    let p1 = Repromise.new_((resolve, _reject) => resolveP1 := resolve);
    let p2 = Repromise.new_((resolve, _reject) => resolveP2 := resolve);
    let p3 = Repromise.race([p1, p2]);
    resolveP1^(42);
    resolveP2^(43);
    p3 |> Repromise.map(n => n == 42);
  }),

  test("second resolves first", () => {
    let resolveP1 = ref(ignore);
    let resolveP2 = ref(ignore);
    let p1 = Repromise.new_((resolve, _reject) => resolveP1 := resolve);
    let p2 = Repromise.new_((resolve, _reject) => resolveP2 := resolve);
    let p3 = Repromise.race([p1, p2]);
    resolveP2^(43);
    resolveP1^(42);
    p3 |> Repromise.map(n => n == 43);
  }),

  test("rejection", () => {
    let rejectP1 = ref(ignore);
    let p1 = Repromise.new_((_resolve, reject) => rejectP1 := reject);
    let p2 = Repromise.new_((_resolve, _reject) => ());
    let p3 = Repromise.race([p1, p2]);
    rejectP1^(42);
    p3 |> Repromise.catch(n => Repromise.resolve(n == 42));
  }),

  test("already resolved", () => {
    let p1 = Repromise.new_((_resolve, _reject) => ());
    let p2 = Repromise.resolve(43);
    let p3 = Repromise.race([p1, p2]);
    p3 |> Repromise.map(n => n == 43);
  }),

  test("already rejected", () => {
    let p1 = Repromise.new_((_resolve, _reject) => ());
    let p2 = Repromise.reject(43);
    let p3 = Repromise.race([p1, p2]);
    p3 |> Repromise.catch(n => Repromise.resolve(n == 43));
  }),

  test("two resolved", () => {
    let p1 = Repromise.resolve(42);
    let p2 = Repromise.resolve(43);
    let p3 = Repromise.race([p1, p2]);
    p3 |> Repromise.map(n => n == 42 || n == 43);
  }),

  test("forever pending", () => {
    let p1 = Repromise.new_((_resolve, _reject) => ());
    let p2 = Repromise.new_((_resolve, _reject) => ());
    let p3 = Repromise.race([p1, p2]);
    remainsPending(p3, 43);
  }),

  test("simultaneous resolve", () => {
    let resolveP1 = ref(ignore);
    let p1 = Repromise.new_((resolve, _reject) => resolveP1 := resolve);
    let p2 = Repromise.race([p1, p1]);
    resolveP1^(42);
    p2 |> Repromise.map(n => n == 42);
  }),

  test("empty", () => {
    let p = Repromise.race([]);
    remainsPending(p, ());
  }),
]);



let suites = [basicTests, rejectTests, raceTests];
