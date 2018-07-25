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
  test("make is js promise", () => {
    let (p, _) = Repromise.make();
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

  test("andThen is js promise", () => {
    let p =
      Repromise.make()
      |> fst
      |> Repromise.andThen((_) => Repromise.resolve());
    Repromise.resolve(isPromise(p));
  }),

  test("map is js promise", () => {
    let p =
      Repromise.make()
      |> fst
      |> Repromise.map(v => v);
    Repromise.resolve(isPromise(p));
  }),

  test("catch is js promise", () => {
    let p =
      Repromise.make()
      |> fst
      |> Repromise.Rejectable.catch((_) => Repromise.resolve());
    Repromise.resolve(isPromise(p));
  }),

  test("js promise is repromise", () => {
    let js_promise: Repromise.t(int) = [%bs.raw {|Promise.resolve(1)|}];
    js_promise
    |> Repromise.andThen(n => Repromise.resolve(n + 1))
    |> Repromise.andThen(n => Repromise.resolve(n == 2));
  }),

  test("repromise as js argument", () => {
    module Then = {
      [@bs.send.pipe: Repromise.t('a)]
      external js_then: ('a => Repromise.t('b)) => Repromise.t('b) =
        "then";
    };
    Repromise.resolve(1)
    |> Then.js_then(n => Repromise.resolve(n + 1))
    |> Repromise.andThen(n => Repromise.resolve(n == 2));
  }),

  test("coerce from Js.Promise", () => {
    Js.Promise.resolve(42)
    |> Repromise.Rejectable.fromJsPromise
    |> Repromise.Rejectable.catch(_ => assert(false))
    |> Repromise.map(n => n == 42);
  }),

  test("coerce to Js.Promise", () => {
    Repromise.resolve(42)
    |> Repromise.Rejectable.toJsPromise
    |> Js.Promise.andThen(n => Js.Promise.resolve(n + 1))
    |> Repromise.Rejectable.fromJsPromise
    |> Repromise.Rejectable.catch(_ => assert(false))
    |> Repromise.map(n => n == 43);
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
    p |> Repromise.andThen(n => Repromise.resolve(n == 42));
  };

let isPromiseRejectedWith42 = p =>
  if (not(isPromise(p))) {
    Repromise.resolve(false);
  }
  else {
    p |> Repromise.Rejectable.catch(n => Repromise.resolve(n == 42));
  };

let soundnessTests = Framework.suite("soundness", [
  test("make: resolve, resolve", () => {
    let (p, resolve) = Repromise.make();
    resolve(Repromise.resolve(42));
    p |> Repromise.andThen(isPromiseResolvedWith42);
  }),

  test("make: resolve, reject", () => {
    let (p, _, reject) = Repromise.Rejectable.make();
    reject(Repromise.resolve(42));
    p |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("make: reject, resolve", () => {
    let (p, resolve) = Repromise.make();
    resolve(Repromise.Rejectable.reject(42));
    p |> Repromise.andThen(isPromiseRejectedWith42);
  }),

  test("make: reject, reject", () => {
    let (p, _, reject) = Repromise.Rejectable.make();
    reject(Repromise.Rejectable.reject(42));
    p |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("resolve: resolve", () => {
    Repromise.resolve(Repromise.resolve(42))
    |> Repromise.andThen(isPromiseResolvedWith42);
  }),

  test("resolve: reject", () => {
    Repromise.resolve(Repromise.Rejectable.reject(42))
    |> Repromise.andThen(isPromiseRejectedWith42);
  }),

  test("reject: resolve", () => {
    Repromise.Rejectable.reject(Repromise.resolve(42))
    |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("reject: reject", () => {
    Repromise.Rejectable.reject(Repromise.Rejectable.reject(42))
    |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("andThen: resolve", () => {
    Repromise.resolve()
    |> Repromise.andThen(() => Repromise.resolve(Repromise.resolve(42)))
    |> Repromise.andThen(isPromiseResolvedWith42);
  }),

  test("andThen: reject", () => {
    Repromise.Rejectable.resolve()
    |> Repromise.Rejectable.andThen(() =>
      Repromise.Rejectable.reject(Repromise.Rejectable.reject(42)))
    |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("map: resolve", () => {
    Repromise.resolve()
    |> Repromise.map(() => Repromise.resolve(42))
    |> Repromise.andThen(isPromiseResolvedWith42);
  }),

  test("map: reject", () => {
    Repromise.resolve()
    |> Repromise.map(() => Repromise.Rejectable.reject(42))
    |> Repromise.andThen(isPromiseRejectedWith42);
  }),

  test("catch: resolve", () => {
    Repromise.Rejectable.reject()
    |> Repromise.Rejectable.catch(() =>
      Repromise.resolve(Repromise.resolve(42)))
    |> Repromise.andThen(isPromiseResolvedWith42);
  }),

  test("catch: reject", () => {
    Repromise.Rejectable.reject()
    |> Repromise.Rejectable.catch(() =>
      Repromise.Rejectable.reject(Repromise.Rejectable.reject(42)))
    |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("make: JS promise", () => {
    let (p, resolve) = Repromise.make();
    resolve(Js.Promise.resolve());
    p |> Repromise.andThen(p => Repromise.resolve(jsPromiseIsPromise(p)));
  }),

  test("resolve: JS promise", () => {
    Repromise.resolve(Js.Promise.resolve())
    |> Repromise.andThen(p => Repromise.resolve(jsPromiseIsPromise(p)));
  }),

  test("reject: JS promise", () => {
    Repromise.Rejectable.reject(Js.Promise.resolve(42))
    |> Repromise.Rejectable.catch(p =>
      Repromise.resolve(jsPromiseIsPromise(p)));
  }),

  test("resolve: Promise-like", () => {
    Repromise.resolve(makePromiseLike())
    |> Repromise.andThen(p => Repromise.resolve(jsPromiseIsPromiseLike(p)));
  }),

  test("resolve: Almost-Promise-like", () => {
    Repromise.resolve(makeAlmostPromiseLike(42))
    |> Repromise.andThen(x => Repromise.resolve(x##_then == 42))
  }),

  test("all", () => {
    let (p1, resolve) = Repromise.make();
    let p2 = Repromise.all([p1]);
    resolve(Repromise.resolve(42));
    p2 |> Repromise.andThen(results =>
      switch (results) {
      | [maybePromise] => isPromiseResolvedWith42(maybePromise)
      | _ => Repromise.resolve(false)
      });
  }),

  test("all, rejection", () => {
    let (p1, _, reject) = Repromise.Rejectable.make();
    let p2 = Repromise.Rejectable.all([p1]);
    reject(Repromise.resolve(42));
    p2
    |> Repromise.Rejectable.map((_) => false)
    |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("race", () => {
    let (p1, resolve) = Repromise.make();
    let p2 = Repromise.race([p1]);
    resolve(Repromise.resolve(42));
    p2 |> Repromise.andThen(isPromiseResolvedWith42);
  }),

  test("race, rejection", () => {
    let (p1, _, reject) = Repromise.Rejectable.make();
    let p2 = Repromise.Rejectable.race([p1]);
    reject(Repromise.resolve(42));
    p2
    |> Repromise.Rejectable.map((_) => false)
    |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
  }),
]);



let suites = [interopTests, soundnessTests];
