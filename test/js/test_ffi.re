let test = Framework.test;

let interop_tests = Framework.suite("interop", [
  test("js return", () => {
    let js_promise: Repromise.t(int) = [%bs.raw {|Promise.resolve(1)|}];
    js_promise
    |> Repromise.then_(n => Repromise.resolve(n + 1))
    |> Repromise.then_(n => Repromise.resolve(n == 2));
  }),

  test("js argument", () => {
    module Then = {
      [@bs.send.pipe: Repromise.t('a)]
      external js_then:
        ('a => [@bs.uncurry] Repromise.t('b)) => Repromise.t('b) = "then";
    };
    Repromise.resolve(1)
    |> Then.js_then(n => Repromise.resolve(n + 1))
    |> Repromise.then_(n => Repromise.resolve(n == 2));
  }),
]);

let node_tests = Framework.suite("node", [
  test("path.delimiter", () => {
    switch Node_path.delimiter {
    | ":" | ";" => Repromise.resolve(true)
    | _ => Repromise.resolve(false)
    };
  }),
]);

let suites = [interop_tests, node_tests];
