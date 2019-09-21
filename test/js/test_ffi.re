/* This file is part of Repromise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/repromise/blob/master/LICENSE.md. */



[%%bs.raw {|
function isPromise (p) {
    return (p instanceof Promise);
}

function isPromiseLike(v) {
  return v && v.then && typeof(v.then) === 'function';
};
|}];

[@bs.val]
external isPromise: Repromise.Rejectable.t(_, _) => bool = "isPromise";

[@bs.val]
external jsPromiseIsPromise: Js.Promise.t(_) => bool = "isPromise";

[@bs.val]
external jsPromiseIsPromiseLike: Js.Promise.t(_) => bool = "isPromiseLike";



let test = Framework.test;



let interopTests = Framework.suite("interop", [
  test("make is js promise", () => {
    let (p, _) = Repromise.make();
    Repromise.resolved(isPromise(p));
  }),

  test("resolved is js promise", () => {
    let p = Repromise.resolved();
    Repromise.resolved(isPromise(p));
  }),

  test("rejected is js promise", () => {
    let p = Repromise.Rejectable.rejected();
    let _ = p |> Repromise.Rejectable.catch(() => Repromise.resolved());
    Repromise.resolved(isPromise(p));
  }),

  test("flatMap is js promise", () => {
    let p =
      Repromise.make()
      |> fst
      |> Repromise.flatMap((_) => Repromise.resolved());
    Repromise.resolved(isPromise(p));
  }),

  test("map is js promise", () => {
    let p =
      Repromise.make()
      |> fst
      |> Repromise.map(v => v);
    Repromise.resolved(isPromise(p));
  }),

  test("catch is js promise", () => {
    let p =
      Repromise.make()
      |> fst
      |> Repromise.Rejectable.catch((_) => Repromise.resolved());
    Repromise.resolved(isPromise(p));
  }),

  test("js promise is repromise", () => {
    let js_promise: Repromise.t(int) = [%bs.raw {|Promise.resolve(1)|}];
    js_promise
    |> Repromise.flatMap(n => Repromise.resolved(n + 1))
    |> Repromise.flatMap(n => Repromise.resolved(n == 2));
  }),

  test("repromise as js argument", () => {
    module Then = {
      [@bs.send.pipe: Repromise.t('a)]
      external js_then: ('a => Repromise.t('b)) => Repromise.t('b) =
        "then";
    };
    Repromise.resolved(1)
    |> Then.js_then(n => Repromise.resolved(n + 1))
    |> Repromise.flatMap(n => Repromise.resolved(n == 2));
  }),

  test("coerce from Js.Promise", () => {
    Js.Promise.resolve(42)
    |> Repromise.Rejectable.fromJsPromise
    |> Repromise.Rejectable.catch(_ => assert(false))
    |> Repromise.map(n => n == 42);
  }),

  test("coerce to Js.Promise", () => {
    Repromise.resolved(42)
    |> Repromise.Rejectable.toJsPromise
    |> Js.Promise.then_(n => Js.Promise.resolve(n + 1))
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
  if (!isPromise(p)) {
    Repromise.resolved(false);
  }
  else {
    p |> Repromise.flatMap(n => Repromise.resolved(n == 42));
  };

let isPromiseRejectedWith42 = p =>
  if (!isPromise(p)) {
    Repromise.resolved(false);
  }
  else {
    p |> Repromise.Rejectable.catch(n => Repromise.resolved(n == 42));
  };

let soundnessTests = Framework.suite("soundness", [
  test("make: resolved, resolve", () => {
    let (p, resolve) = Repromise.make();
    resolve(Repromise.resolved(42));
    p |> Repromise.flatMap(isPromiseResolvedWith42);
  }),

  test("make: resolve, reject", () => {
    let (p, _, reject) = Repromise.Rejectable.make();
    reject(Repromise.resolved(42));
    p |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("make: rejected, resolve", () => {
    let (p, resolve) = Repromise.make();
    resolve(Repromise.Rejectable.rejected(42));
    p |> Repromise.flatMap(isPromiseRejectedWith42);
  }),

  test("make: rejected, reject", () => {
    let (p, _, reject) = Repromise.Rejectable.make();
    reject(Repromise.Rejectable.rejected(42));
    p |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("resolve: resolved", () => {
    Repromise.resolved(Repromise.resolved(42))
    |> Repromise.flatMap(isPromiseResolvedWith42);
  }),

  test("resolve: rejected", () => {
    Repromise.resolved(Repromise.Rejectable.rejected(42))
    |> Repromise.flatMap(isPromiseRejectedWith42);
  }),

  test("rejected: resolved", () => {
    Repromise.Rejectable.rejected(Repromise.resolved(42))
    |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("rejected: rejected", () => {
    Repromise.Rejectable.rejected(Repromise.Rejectable.rejected(42))
    |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("flatMap: resolved", () => {
    Repromise.resolved()
    |> Repromise.flatMap(() => Repromise.resolved(Repromise.resolved(42)))
    |> Repromise.flatMap(isPromiseResolvedWith42);
  }),

  test("flatMap: rejected", () => {
    Repromise.Rejectable.resolved()
    |> Repromise.Rejectable.flatMap(() =>
      Repromise.Rejectable.rejected(Repromise.Rejectable.rejected(42)))
    |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("map: resolved", () => {
    Repromise.resolved()
    |> Repromise.map(() => Repromise.resolved(42))
    |> Repromise.flatMap(isPromiseResolvedWith42);
  }),

  test("map: rejected", () => {
    Repromise.resolved()
    |> Repromise.map(() => Repromise.Rejectable.rejected(42))
    |> Repromise.flatMap(isPromiseRejectedWith42);
  }),

  test("catch: resolved", () => {
    Repromise.Rejectable.rejected()
    |> Repromise.Rejectable.catch(() =>
      Repromise.resolved(Repromise.resolved(42)))
    |> Repromise.flatMap(isPromiseResolvedWith42);
  }),

  test("catch: rejected", () => {
    Repromise.Rejectable.rejected()
    |> Repromise.Rejectable.catch(() =>
      Repromise.Rejectable.rejected(Repromise.Rejectable.rejected(42)))
    |> Repromise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("make: JS promise", () => {
    let (p, resolve) = Repromise.make();
    resolve(Js.Promise.resolve());
    p |> Repromise.flatMap(p => Repromise.resolved(jsPromiseIsPromise(p)));
  }),

  test("resolved: JS promise", () => {
    Repromise.resolved(Js.Promise.resolve())
    |> Repromise.flatMap(p => Repromise.resolved(jsPromiseIsPromise(p)));
  }),

  test("rejected: JS promise", () => {
    Repromise.Rejectable.rejected(Js.Promise.resolve(42))
    |> Repromise.Rejectable.catch(p =>
      Repromise.resolved(jsPromiseIsPromise(p)));
  }),

  test("resolved: Promise-like", () => {
    Repromise.resolved(makePromiseLike())
    |> Repromise.flatMap(p => Repromise.resolved(jsPromiseIsPromiseLike(p)));
  }),

  test("resolved: Almost-Promise-like", () => {
    Repromise.resolved(makeAlmostPromiseLike(42))
    |> Repromise.flatMap(x => Repromise.resolved(x##_then == 42));
  }),

  test("all", () => {
    let (p1, resolve) = Repromise.make();
    let p2 = Repromise.all([p1]);
    resolve(Repromise.resolved(42));
    p2 |> Repromise.flatMap(results =>
      switch (results) {
      | [maybePromise] => isPromiseResolvedWith42(maybePromise)
      | _ => Repromise.resolved(false)
      });
  }),

  test("all, rejection", () => {
    let (p1, _, reject) = Repromise.Rejectable.make();
    let p2 = Repromise.Rejectable.all([p1]);
    reject(Repromise.resolved(42));
    p2
    |> Repromise.Rejectable.map((_) => false)
    |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("race", () => {
    let (p1, resolve) = Repromise.make();
    let p2 = Repromise.race([p1]);
    resolve(Repromise.resolved(42));
    p2 |> Repromise.flatMap(isPromiseResolvedWith42);
  }),

  test("race, rejection", () => {
    let (p1, _, reject) = Repromise.Rejectable.make();
    let p2 = Repromise.Rejectable.race([p1]);
    reject(Repromise.resolved(42));
    p2
    |> Repromise.Rejectable.map((_) => false)
    |> Repromise.Rejectable.catch(isPromiseResolvedWith42);
  }),
]);



let suites = [interopTests, soundnessTests];
