[%%bs.raw {|
function isPromise (p) {
    return (p instanceof Promise);
}
|}];

[@bs.val]
external isPromise: Repromise.t('a) => bool = "";

[@bs.val]
external jsPromiseIsPromise: Js.Promise.t('a) => bool = "isPromise";

let test = Framework.test;

let interopTests = Framework.suite("interop", [
  test("new_ is js promise", () => {
    let p = Repromise.new_((~resolve as _) => ());
    Repromise.resolve(isPromise(p));
  }),

  test("resolve is js promise", () => {
    let p = Repromise.resolve();
    Repromise.resolve(isPromise(p));
  }),

  test("then_ is js promise", () => {
    let p =
      Repromise.new_((~resolve as _) => ())
      |> Repromise.then_((_) => Repromise.resolve());
    Repromise.resolve(isPromise(p));
  }),

  test("js promise is repromise", () => {
    let js_promise: Repromise.t(int) = [%bs.raw {|Promise.resolve(1)|}];
    js_promise
    |> Repromise.then_(n => Repromise.resolve(n + 1))
    |> Repromise.then_(n => Repromise.resolve(n == 2));
  }),

  test("repromsie as js argument", () => {
    module Then = {
      [@bs.send.pipe: Repromise.t('a)]
      external js_then: ('a => Repromise.t('b)) => Repromise.t('b) = "then";
    };
    Repromise.resolve(1)
    |> Then.js_then(n => Repromise.resolve(n + 1))
    |> Repromise.then_(n => Repromise.resolve(n == 2));
  }),

  test("new_ does not collapse", () => {
    Repromise.new_((~resolve) => resolve(Repromise.resolve()))
    |> Repromise.then_(p => Repromise.resolve(isPromise(p)));
  }),

  test("then_ extracts nested promise (new_)", () => {
    Repromise.new_((~resolve) => resolve(Repromise.resolve(42)))
    |> Repromise.then_(p =>
      p |> Repromise.then_(n =>
        Repromise.resolve(n == 42)));
  }),

  test("resolve does not collapse", () => {
    Repromise.resolve(Repromise.resolve())
    |> Repromise.then_(p => Repromise.resolve(isPromise(p)));
  }),

  test("then_ extracts nested promise (resolve)", () => {
    Repromise.resolve(Repromise.resolve(42))
    |> Repromise.then_(p =>
      p |> Repromise.then_(n =>
        Repromise.resolve(n == 42)));
  }),

  test("then_ does not collapse", () => {
    Repromise.resolve()
    |> Repromise.then_(() => Repromise.resolve(Repromise.resolve()))
    |> Repromise.then_(p => Repromise.resolve(isPromise(p)));
  }),

  test("resolve does not collapse, JS promise", () => {
    Repromise.new_((~resolve) => resolve(Js.Promise.resolve()))
    |> Repromise.then_(p => Repromise.resolve(jsPromiseIsPromise(p)));
  }),

  test("resolve does not collapse, JS promise", () => {
    Repromise.resolve(Js.Promise.resolve(42))
    |> Repromise.then_(p => Repromise.resolve(jsPromiseIsPromise(p)));
  }),
]);

let nodeTests = Framework.suite("node", [
  test("path.delimiter", () => {
    switch Node_path.delimiter {
    | ":" | ";" => Repromise.resolve(true)
    | _ => Repromise.resolve(false)
    };
  }),
]);

let suites = [interopTests, nodeTests];
