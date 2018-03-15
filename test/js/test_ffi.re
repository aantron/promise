[%%bs.raw {|
function isPromise (p) {
    return (p instanceof Promise);
}

function isPromiseLike(v) {
  return v && v.then && typeof(v.then) === 'function';
};
|}];

[@bs.val]
external isPromise: Repromise.Rejectable.t(_, _) => bool = "";

[@bs.val]
external jsPromiseIsPromise: Js.Promise.t(_) => bool = "isPromise";

[@bs.val]
external jsPromiseIsPromiseLike: Js.Promise.t(_) => bool = "isPromiseLike";



let test = Framework.test;



let interopTests = Framework.suite("interop", [
  test("new_ is js promise", () => {
    let (p, _) = Repromise.new_();
    Repromise.resolve(isPromise(p));
  }),

  test("resolve is js promise", () => {
    let p = Repromise.resolve();
    Repromise.resolve(isPromise(p));
  }),

  test("reject is js promise", () => {
    let p = Repromise.Rejectable.reject();
    let _ = p |> Repromise.Rejectable.catch(() => Repromise.resolve());
    Repromise.resolve(isPromise(p));
  }),

  test("then_ is js promise", () => {
    let p =
      Repromise.new_()
      |> fst
      |> Repromise.then_((_) => Repromise.resolve());
    Repromise.resolve(isPromise(p));
  }),

  test("map is js promise", () => {
    let p =
      Repromise.new_()
      |> fst
      |> Repromise.map(v => v);
    Repromise.resolve(isPromise(p));
  }),

  test("catch is js promise", () => {
    let p =
      Repromise.new_()
      |> fst
      |> Repromise.Rejectable.catch((_) => Repromise.resolve());
    Repromise.resolve(isPromise(p));
  }),

  test("js promise is repromise", () => {
    let js_promise: Repromise.t(int) = [%bs.raw {|Promise.resolve(1)|}];
    js_promise
    |> Repromise.then_(n => Repromise.resolve(n + 1))
    |> Repromise.then_(n => Repromise.resolve(n == 2));
  }),

  test("repromise as js argument", () => {
    module Then = {
      [@bs.send.pipe: Repromise.t('a)]
      external js_then: ('a => Repromise.t('b)) => Repromise.t('b) =
        "then";
    };
    Repromise.resolve(1)
    |> Then.js_then(n => Repromise.resolve(n + 1))
    |> Repromise.then_(n => Repromise.resolve(n == 2));
  }),
]);



/* The method name "_then," below, is actually resolved to "then" in JavaScript.
   The leading underscore is removed by BuckleScript. This mangling is for
   avoiding collision with the OCaml keyword "then." */

external castToPromise:
  {."_then": ('a => unit, 'e => unit) => unit} => Js.Promise.t('a) =
  "%identity";

let makePromiseLike: 'a => Js.Promise.t('a) = v =>
  {"_then": (resolve, _) => resolve(v)} |> castToPromise;

let makeAlmostPromiseLike = v =>
  {"_then": v};

let isPromiseResolvedWith42 = p =>
  if (not(isPromise(p))) {
    Repromise.resolve(false);
  }
  else {
    p |> Repromise.then_(n => Repromise.resolve(n == 42));
  };

let isPromiseRejectedWith42 = p =>
  if (not(isPromise(p))) {
    Repromise.resolve(false);
  }
  else {
    p |> Repromise.Rejectable.catch(n => Repromise.resolve(n == 42));
  };

let soundnessTests = Framework.suite("soundness", [
  test("new_: resolve, resolve", () => {
    let (p, resolve) = Repromise.new_();
    resolve(Repromise.resolve(42));
    p |> Repromise.then_(isPromiseResolvedWith42);
  }),

  test("new_: resolve, reject", () => {
    let (p, _, reject) = Repromise.Rejectable.new_();
    reject(Repromise.resolve(42));
    p |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("new_: reject, resolve", () => {
    let (p, resolve) = Repromise.new_();
    resolve(Repromise.Rejectable.reject(42));
    p |> Repromise.then_(isPromiseRejectedWith42);
  }),

  test("new_: reject, reject", () => {
    let (p, _, reject) = Repromise.Rejectable.new_();
    reject(Repromise.Rejectable.reject(42));
    p |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("resolve: resolve", () => {
    Repromise.resolve(Repromise.resolve(42))
    |> Repromise.then_(isPromiseResolvedWith42);
  }),

  test("resolve: reject", () => {
    Repromise.resolve(Repromise.Rejectable.reject(42))
    |> Repromise.then_(isPromiseRejectedWith42);
  }),

  test("reject: resolve", () => {
    Repromise.Rejectable.reject(Repromise.resolve(42))
    |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("reject: reject", () => {
    Repromise.Rejectable.reject(Repromise.Rejectable.reject(42))
    |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("then_: resolve", () => {
    Repromise.resolve()
    |> Repromise.then_(() => Repromise.resolve(Repromise.resolve(42)))
    |> Repromise.then_(isPromiseResolvedWith42);
  }),

  test("then_: reject", () => {
    Repromise.Rejectable.resolve()
    |> Repromise.Rejectable.then_(() =>
      Repromise.Rejectable.reject(Repromise.Rejectable.reject(42)))
    |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("map: resolve", () => {
    Repromise.resolve()
    |> Repromise.map(() => Repromise.resolve(42))
    |> Repromise.then_(isPromiseResolvedWith42);
  }),

  test("map: reject", () => {
    Repromise.resolve()
    |> Repromise.map(() => Repromise.Rejectable.reject(42))
    |> Repromise.then_(isPromiseRejectedWith42);
  }),

  test("catch: resolve", () => {
    Repromise.Rejectable.reject()
    |> Repromise.Rejectable.catch(() =>
      Repromise.resolve(Repromise.resolve(42)))
    |> Repromise.then_(isPromiseResolvedWith42);
  }),

  test("catch: reject", () => {
    Repromise.Rejectable.reject()
    |> Repromise.Rejectable.catch(() =>
      Repromise.Rejectable.reject(Repromise.Rejectable.reject(42)))
    |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("new_: JS promise", () => {
    let (p, resolve) = Repromise.new_();
    resolve(Js.Promise.resolve());
    p |> Repromise.then_(p => Repromise.resolve(jsPromiseIsPromise(p)));
  }),

  test("resolve: JS promise", () => {
    Repromise.resolve(Js.Promise.resolve())
    |> Repromise.then_(p => Repromise.resolve(jsPromiseIsPromise(p)));
  }),

  test("reject: JS promise", () => {
    Repromise.Rejectable.reject(Js.Promise.resolve(42))
    |> Repromise.Rejectable.catch(p =>
      Repromise.resolve(jsPromiseIsPromise(p)));
  }),

  test("resolve: Promise-like", () => {
    Repromise.resolve(makePromiseLike())
    |> Repromise.then_(p => Repromise.resolve(jsPromiseIsPromiseLike(p)));
  }),

  test("resolve: Almost-Promise-like", () => {
    Repromise.resolve(makeAlmostPromiseLike(42))
    |> Repromise.then_(x => Repromise.resolve(x##_then == 42))
  }),

  test("all", () => {
    let (p1, resolve) = Repromise.new_();
    let p2 = Repromise.all([p1]);
    resolve(Repromise.resolve(42));
    p2 |> Repromise.then_(results =>
      switch (results) {
      | [maybePromise] => isPromiseResolvedWith42(maybePromise)
      | _ => Repromise.resolve(false)
      });
  }),

  test("all, rejection", () => {
    let (p1, _, reject) = Repromise.Rejectable.new_();
    let p2 = Repromise.Rejectable.all([p1]);
    reject(Repromise.resolve(42));
    p2
    |> Repromise.Rejectable.map((_) => false)
    |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("race", () => {
    let (p1, resolve) = Repromise.new_();
    let p2 = Repromise.race([p1]);
    resolve(Repromise.resolve(42));
    p2 |> Repromise.then_(isPromiseResolvedWith42);
  }),

  test("race, rejection", () => {
    let (p1, _, reject) = Repromise.Rejectable.new_();
    let p2 = Repromise.Rejectable.race([p1]);
    reject(Repromise.resolve(42));
    p2
    |> Repromise.Rejectable.map((_) => false)
    |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
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



let suites = [interopTests, soundnessTests, nodeTests];
