[%%bs.raw {|
function isPromise (p) {
    return (p instanceof Promise);
}

function isPromiseLike(v) {
  return v && v.then && typeof(v.then) === 'function';
};
|}];

[@bs.val]
external isPromise: Repromise.t(_, _) => bool = "";

[@bs.val]
external jsPromiseIsPromise: Js.Promise.t(_) => bool = "isPromise";

[@bs.val]
external jsPromiseIsPromiseLike: Js.Promise.t(_) => bool = "isPromiseLike";



let test = Framework.test;



let interopTests = Framework.suite("interop", [
  test("new_ is js promise", () => {
    let p = Repromise.new_((_, _) => ());
    Repromise.resolve(isPromise(p));
  }),

  test("resolve is js promise", () => {
    let p = Repromise.resolve();
    Repromise.resolve(isPromise(p));
  }),

  test("reject is js promise", () => {
    let p = Repromise.reject();
    let _ = p |> Repromise.catch(() => Repromise.resolve());
    Repromise.resolve(isPromise(p));
  }),

  test("then_ is js promise", () => {
    let p =
      Repromise.new_((_, _) => ())
      |> Repromise.then_((_) => Repromise.resolve());
    Repromise.resolve(isPromise(p));
  }),

  test("map is js promise", () => {
    let p = Repromise.new_((_, _) => ()) |> Repromise.map(v => v);
    Repromise.resolve(isPromise(p));
  }),

  test("catch is js promise", () => {
    let p =
      Repromise.new_((_, _) => ())
      |> Repromise.catch((_) => Repromise.resolve());
    Repromise.resolve(isPromise(p));
  }),

  test("js promise is repromise", () => {
    let js_promise: Repromise.t(int, _) = [%bs.raw {|Promise.resolve(1)|}];
    js_promise
    |> Repromise.then_(n => Repromise.resolve(n + 1))
    |> Repromise.then_(n => Repromise.resolve(n == 2));
  }),

  test("repromise as js argument", () => {
    module Then = {
      [@bs.send.pipe: Repromise.t('a, 'e)]
      external js_then: ('a => Repromise.t('b, 'e)) => Repromise.t('b, 'e) =
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
    prerr_endline("not a promise");
    Js.log(p);
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
    p |> Repromise.catch(n => Repromise.resolve(n == 42));
  };

let soundnessTests = Framework.suite("soundness", [
  test("new_: resolve, resolve", () => {
    Repromise.new_((resolve, _) => resolve(Repromise.resolve(42)))
    |> Repromise.then_(isPromiseResolvedWith42);
  }),

  test("new_: resolve, reject", () => {
    Repromise.new_((_, reject) => reject(Repromise.resolve(42)))
    |> Repromise.catch(isPromiseResolvedWith42);
  }),

  test("new_: reject, resolve", () => {
    Repromise.new_((resolve, _) => resolve(Repromise.reject(42)))
    |> Repromise.then_(isPromiseRejectedWith42);
  }),

  test("new_: reject, reject", () => {
    Repromise.new_((_, reject) => reject(Repromise.reject(42)))
    |> Repromise.catch(isPromiseRejectedWith42);
  }),

  test("resolve: resolve", () => {
    Repromise.resolve(Repromise.resolve(42))
    |> Repromise.then_(isPromiseResolvedWith42);
  }),

  test("resolve: reject", () => {
    Repromise.resolve(Repromise.reject(42))
    |> Repromise.then_(isPromiseRejectedWith42);
  }),

  test("reject: resolve", () => {
    Repromise.reject(Repromise.resolve(42))
    |> Repromise.catch(isPromiseResolvedWith42);
  }),

  test("reject: reject", () => {
    Repromise.reject(Repromise.reject(42))
    |> Repromise.catch(isPromiseRejectedWith42);
  }),

  test("then_: resolve", () => {
    Repromise.resolve()
    |> Repromise.then_(() => Repromise.resolve(Repromise.resolve(42)))
    |> Repromise.then_(isPromiseResolvedWith42);
  }),

  test("then_: reject", () => {
    Repromise.resolve()
    |> Repromise.then_(() => Repromise.reject(Repromise.reject(42)))
    |> Repromise.catch(isPromiseRejectedWith42);
  }),

  test("catch: resolve", () => {
    Repromise.reject()
    |> Repromise.catch(() => Repromise.resolve(Repromise.resolve(42)))
    |> Repromise.then_(isPromiseResolvedWith42);
  }),

  test("catch: reject", () => {
    Repromise.reject()
    |> Repromise.catch(() => Repromise.reject(Repromise.reject(42)))
    |> Repromise.catch(isPromiseRejectedWith42);
  }),

  test("new_: JS promise", () => {
    Repromise.new_((resolve, _) => resolve(Js.Promise.resolve()))
    |> Repromise.then_(p => Repromise.resolve(jsPromiseIsPromise(p)));
  }),

  test("resolve: JS promise", () => {
    Repromise.resolve(Js.Promise.resolve())
    |> Repromise.then_(p => Repromise.resolve(jsPromiseIsPromise(p)));
  }),

  test("reject: JS promise", () => {
    Repromise.reject(Js.Promise.resolve(42))
    |> Repromise.catch(p => Repromise.resolve(jsPromiseIsPromise(p)));
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
    let resolveP1 = ref(ignore);
    let p1 = Repromise.new_((resolve, _reject) => resolveP1 := resolve);
    let p2 = Repromise.all([p1]);
    resolveP1^(Repromise.resolve(42));
    p2 |> Repromise.then_(results =>
      switch (results) {
      | [maybePromise] => isPromiseResolvedWith42(maybePromise)
      | _ => Repromise.resolve(false)
      });
  }),

  test("all, rejection", () => {
    let rejectP1 = ref(ignore);
    let p1 = Repromise.new_((_resolve, reject) => rejectP1 := reject);
    let p2 = Repromise.all([p1]);
    rejectP1^(Repromise.resolve(42));
    p2
    |> Repromise.map((_) => false)
    |> Repromise.catch(isPromiseResolvedWith42);
  }),

  test("race", () => {
    let resolveP1 = ref(ignore);
    let p1 = Repromise.new_((resolve, _reject) => resolveP1 := resolve);
    let p2 = Repromise.race([p1]);
    resolveP1^(Repromise.resolve(42));
    p2 |> Repromise.then_(isPromiseResolvedWith42);
  }),

  test("race, rejection", () => {
    let rejectP1 = ref(ignore);
    let p1 = Repromise.new_((_resolve, reject) => rejectP1 := reject);
    let p2 = Repromise.race([p1]);
    rejectP1^(Repromise.resolve(42));
    p2
    |> Repromise.map((_) => false)
    |> Repromise.catch(isPromiseResolvedWith42);
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
