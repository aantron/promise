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
external isPromise: Promise.Rejectable.t(_, _) => bool = "isPromise";

[@bs.val]
external jsPromiseIsPromise: Js.Promise.t(_) => bool = "isPromise";

[@bs.val]
external jsPromiseIsPromiseLike: Js.Promise.t(_) => bool = "isPromiseLike";



let test = Framework.test;



let interopTests = Framework.suite("interop", [
  test("make is js promise", () => {
    let (p, _) = Promise.make();
    Promise.resolved(isPromise(p));
  }),

  test("resolved is js promise", () => {
    let p = Promise.resolved();
    Promise.resolved(isPromise(p));
  }),

  test("rejected is js promise", () => {
    let p = Promise.Rejectable.rejected();
    let _ = p |> Promise.Rejectable.catch(() => Promise.resolved());
    Promise.resolved(isPromise(p));
  }),

  test("flatMap is js promise", () => {
    let p =
      Promise.make()
      ->fst
      ->Promise.flatMap((_) => Promise.resolved());
    Promise.resolved(isPromise(p));
  }),

  test("map is js promise", () => {
    let p =
      fst(Promise.make())
      ->Promise.map(v => v);
    Promise.resolved(isPromise(p));
  }),

  test("catch is js promise", () => {
    let p =
      Promise.make()
      |> fst
      |> Promise.Rejectable.catch((_) => Promise.resolved());
    Promise.resolved(isPromise(p));
  }),

  test("js promise is repromise", () => {
    let js_promise: Promise.t(int) = [%bs.raw {|Promise.resolve(1)|}];
    js_promise
    ->Promise.flatMap(n => Promise.resolved(n + 1))
    ->Promise.flatMap(n => Promise.resolved(n == 2));
  }),

  test("repromise as js argument", () => {
    module Then = {
      [@bs.send.pipe: Promise.t('a)]
      external js_then: ('a => Promise.t('b)) => Promise.t('b) =
        "then";
    };
    (Promise.resolved(1)
    |> Then.js_then(n => Promise.resolved(n + 1)))
    ->Promise.flatMap(n => Promise.resolved(n == 2));
  }),

  test("coerce from Js.Promise", () => {
    (Js.Promise.resolve(42)
    |> Promise.Rejectable.fromJsPromise
    |> Promise.Rejectable.catch(_ => assert(false)))
    ->Promise.map(n => n == 42);
  }),

  test("coerce to Js.Promise", () => {
    (Promise.resolved(42)
    |> Promise.Rejectable.toJsPromise
    |> Js.Promise.then_(n => Js.Promise.resolve(n + 1))
    |> Promise.Rejectable.fromJsPromise
    |> Promise.Rejectable.catch(_ => assert(false)))
    ->Promise.map(n => n == 43);
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
    Promise.resolved(false);
  }
  else {
    p->Promise.flatMap(n => Promise.resolved(n == 42));
  };

let isPromiseRejectedWith42 = p =>
  if (!isPromise(p)) {
    Promise.resolved(false);
  }
  else {
    p |> Promise.Rejectable.catch(n => Promise.resolved(n == 42));
  };

let soundnessTests = Framework.suite("soundness", [
  test("make: resolved, resolve", () => {
    let (p, resolve) = Promise.make();
    resolve(Promise.resolved(42));
    p->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("make: resolve, reject", () => {
    let (p, _, reject) = Promise.Rejectable.make();
    reject(Promise.resolved(42));
    p |> Promise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("make: rejected, resolve", () => {
    let (p, resolve) = Promise.make();
    resolve(Promise.Rejectable.rejected(42));
    p->Promise.flatMap(isPromiseRejectedWith42);
  }),

  test("make: rejected, reject", () => {
    let (p, _, reject) = Promise.Rejectable.make();
    reject(Promise.Rejectable.rejected(42));
    p |> Promise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("resolve: resolved", () => {
    Promise.resolved(Promise.resolved(42))
    ->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("resolve: rejected", () => {
    Promise.resolved(Promise.Rejectable.rejected(42))
    ->Promise.flatMap(isPromiseRejectedWith42);
  }),

  test("rejected: resolved", () => {
    Promise.Rejectable.rejected(Promise.resolved(42))
    |> Promise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("rejected: rejected", () => {
    Promise.Rejectable.rejected(Promise.Rejectable.rejected(42))
    |> Promise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("flatMap: resolved", () => {
    Promise.resolved()
    ->Promise.flatMap(() => Promise.resolved(Promise.resolved(42)))
    ->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("flatMap: rejected", () => {
    Promise.Rejectable.resolved()
    ->Promise.Rejectable.flatMap(() =>
      Promise.Rejectable.rejected(Promise.Rejectable.rejected(42)))
    |> Promise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("map: resolved", () => {
    Promise.resolved()
    ->Promise.map(() => Promise.resolved(42))
    ->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("map: rejected", () => {
    Promise.resolved()
    ->Promise.map(() => Promise.Rejectable.rejected(42))
    ->Promise.flatMap(isPromiseRejectedWith42);
  }),

  test("catch: resolved", () => {
    (Promise.Rejectable.rejected()
    |> Promise.Rejectable.catch(() =>
      Promise.resolved(Promise.resolved(42))))
    ->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("catch: rejected", () => {
    Promise.Rejectable.rejected()
    |> Promise.Rejectable.catch(() =>
      Promise.Rejectable.rejected(Promise.Rejectable.rejected(42)))
    |> Promise.Rejectable.catch(isPromiseRejectedWith42);
  }),

  test("make: JS promise", () => {
    let (p, resolve) = Promise.make();
    resolve(Js.Promise.resolve());
    p->Promise.flatMap(p => Promise.resolved(jsPromiseIsPromise(p)));
  }),

  test("resolved: JS promise", () => {
    Promise.resolved(Js.Promise.resolve())
    ->Promise.flatMap(p => Promise.resolved(jsPromiseIsPromise(p)));
  }),

  test("rejected: JS promise", () => {
    Promise.Rejectable.rejected(Js.Promise.resolve(42))
    |> Promise.Rejectable.catch(p =>
      Promise.resolved(jsPromiseIsPromise(p)));
  }),

  test("resolved: Promise-like", () => {
    Promise.resolved(makePromiseLike())
    ->Promise.flatMap(p => Promise.resolved(jsPromiseIsPromiseLike(p)));
  }),

  test("resolved: Almost-Promise-like", () => {
    Promise.resolved(makeAlmostPromiseLike(42))
    ->Promise.flatMap(x => Promise.resolved(x##_then == 42));
  }),

  test("all", () => {
    let (p1, resolve) = Promise.make();
    let p2 = Promise.all([p1]);
    resolve(Promise.resolved(42));
    p2->Promise.flatMap(results =>
      switch (results) {
      | [maybePromise] => isPromiseResolvedWith42(maybePromise)
      | _ => Promise.resolved(false)
      });
  }),

  test("all, rejection", () => {
    let (p1, _, reject) = Promise.Rejectable.make();
    let p2 = Promise.Rejectable.all([p1]);
    reject(Promise.resolved(42));
    p2
    ->Promise.Rejectable.map((_) => false)
    |> Promise.Rejectable.catch(isPromiseResolvedWith42);
  }),

  test("race", () => {
    let (p1, resolve) = Promise.make();
    let p2 = Promise.race([p1]);
    resolve(Promise.resolved(42));
    p2->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("race, rejection", () => {
    let (p1, _, reject) = Promise.Rejectable.make();
    let p2 = Promise.Rejectable.race([p1]);
    reject(Promise.resolved(42));
    p2
    ->Promise.Rejectable.map((_) => false)
    |> Promise.Rejectable.catch(isPromiseResolvedWith42);
  }),
]);



let suites = [interopTests, soundnessTests];
