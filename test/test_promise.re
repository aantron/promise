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

  /* This test fails on Node.js but passes on native. This is because JS
     promises collapse: [Repromise.resolve(p)] when [p] is a promise doesn't
     create a promise that is resolved with [p]. It just returns a reference
     directly to [p]. This is one of the things we need to address. */
  /*
  test("no collapsing", () => {
    Repromise.resolve(Repromise.resolve(1))
    |> Repromise.then_ (p =>
      p |> Repromise.then_ (n => Repromise.resolve (n == 1)));
  }),
  */

  test("new_", () => {
    Repromise.new_((~resolve) => resolve(true));
  }),

  test("defer", () => {
    let resolve_p = ref(ignore);
    let p =
      Repromise.new_((~resolve) => resolve_p := resolve)
      |> Repromise.then_(n => Repromise.resolve(n == 1));
    resolve_p^(1);
    p;
  }),

  test("await defer", () => {
    let resolve_p = ref(ignore);
    let p = {
      let%await n = Repromise.new_((~resolve) => resolve_p := resolve);
      Repromise.resolve(n == 1);
    };
    resolve_p^(1);
    p;
  }),
]);
