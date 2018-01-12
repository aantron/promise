let test = Framework.test;

let suite = Framework.suite("promise", [
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
    Repromise.new_(resolve => resolve(true));
  }),

  test("defer", () => {
    let resolve_p = ref(ignore);
    let p =
      Repromise.new_(resolve => resolve_p := resolve)
      |> Repromise.then_(n => Repromise.resolve(n == 1));
    resolve_p^(1);
    p;
  }),

  test("await defer", () => {
    let resolve_p = ref(ignore);
    let p = {
      let%await n = Repromise.new_(resolve => resolve_p := resolve);
      Repromise.resolve(n == 1);
    };
    resolve_p^(1);
    p;
  }),
]);
