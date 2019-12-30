/* This file is part of reason-promise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/promise/blob/master/LICENSE.md. */



[%%bs.raw {|
function isPromise (p) {
    return (p instanceof Promise);
}

function isPromiseLike(v) {
  return v && v.then && typeof(v.then) === 'function';
};
|}];

[@bs.val]
external isPromise: Promise.Js.t(_, _) => bool = "isPromise";

[@bs.val]
external jsPromiseIsPromise: Js.Promise.t(_) => bool = "isPromise";

[@bs.val]
external jsPromiseIsPromiseLike: Js.Promise.t(_) => bool = "isPromiseLike";



let test = Framework.test;



let interopTests = Framework.suite("interop", [
  test("pending is js promise", () => {
    let (p, _) = Promise.pending();
    Promise.resolved(isPromise(p));
  }),

  test("resolved is js promise", () => {
    let p = Promise.resolved();
    Promise.resolved(isPromise(p));
  }),

  test("rejected is js promise", () => {
    let p = Promise.Js.rejected();
    let _ = p->Promise.Js.catch(() => Promise.resolved());
    Promise.resolved(isPromise(p));
  }),

  test("flatMap is js promise", () => {
    let p =
      Promise.pending()
      ->fst
      ->Promise.flatMap((_) => Promise.resolved());
    Promise.resolved(isPromise(p));
  }),

  test("map is js promise", () => {
    let p =
      fst(Promise.pending())
      ->Promise.map(v => v);
    Promise.resolved(isPromise(p));
  }),

  test("catch is js promise", () => {
    let p =
      Promise.pending()
      ->fst
      ->Promise.Js.catch((_) => Promise.resolved());
    Promise.resolved(isPromise(p));
  }),

  test("js promise is reason-promise", () => {
    let js_promise: Promise.t(int) = [%bs.raw {|Promise.resolve(1)|}];
    js_promise
    ->Promise.flatMap(n => Promise.resolved(n + 1))
    ->Promise.flatMap(n => Promise.resolved(n == 2));
  }),

  test("reason-promise as js argument", () => {
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
    Js.Promise.resolve(42)
    ->Promise.Js.fromBsPromise
    ->Promise.Js.catch(_ => assert(false))
    ->Promise.map(n => n == 42);
  }),

  test("coerce to Js.Promise", () => {
    (Promise.resolved(42)
    ->Promise.Js.toBsPromise
    |> Js.Promise.then_(n => Js.Promise.resolve(n + 1)))
    ->Promise.Js.fromBsPromise
    ->Promise.Js.catch(_ => assert(false))
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
    p->Promise.Js.catch(n => Promise.resolved(n == 42));
  };

let soundnessTests = Framework.suite("soundness", [
  test("pending: resolved, resolve", () => {
    let (p, resolve) = Promise.pending();
    resolve(Promise.resolved(42));
    p->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("pending: resolve, reject", () => {
    let (p, _, reject) = Promise.Js.pending();
    reject(Promise.resolved(42));
    p->Promise.Js.catch(isPromiseResolvedWith42);
  }),

  test("pending: rejected, resolve", () => {
    let (p, resolve) = Promise.pending();
    resolve(Promise.Js.rejected(42));
    p->Promise.flatMap(isPromiseRejectedWith42);
  }),

  test("pending: rejected, reject", () => {
    let (p, _, reject) = Promise.Js.pending();
    reject(Promise.Js.rejected(42));
    p->Promise.Js.catch(isPromiseRejectedWith42);
  }),

  test("resolve: resolved", () => {
    Promise.resolved(Promise.resolved(42))
    ->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("resolve: rejected", () => {
    Promise.resolved(Promise.Js.rejected(42))
    ->Promise.flatMap(isPromiseRejectedWith42);
  }),

  test("rejected: resolved", () => {
    Promise.Js.rejected(Promise.resolved(42))
    ->Promise.Js.catch(isPromiseResolvedWith42);
  }),

  test("rejected: rejected", () => {
    Promise.Js.rejected(Promise.Js.rejected(42))
    ->Promise.Js.catch(isPromiseRejectedWith42);
  }),

  test("flatMap: resolved", () => {
    Promise.resolved()
    ->Promise.flatMap(() => Promise.resolved(Promise.resolved(42)))
    ->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("flatMap: rejected", () => {
    Promise.Js.resolved()
    ->Promise.Js.flatMap(() =>
      Promise.Js.rejected(Promise.Js.rejected(42)))
    ->Promise.Js.catch(isPromiseRejectedWith42);
  }),

  test("map: resolved", () => {
    Promise.resolved()
    ->Promise.map(() => Promise.resolved(42))
    ->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("map: rejected", () => {
    Promise.resolved()
    ->Promise.map(() => Promise.Js.rejected(42))
    ->Promise.flatMap(isPromiseRejectedWith42);
  }),

  test("catch: resolved", () => {
    Promise.Js.rejected()
    ->Promise.Js.catch(() => Promise.resolved(Promise.resolved(42)))
    ->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("catch: rejected", () => {
    Promise.Js.rejected()
    ->Promise.Js.catch(() =>
      Promise.Js.rejected(Promise.Js.rejected(42)))
    ->Promise.Js.catch(isPromiseRejectedWith42);
  }),

  test("pending: JS promise", () => {
    let (p, resolve) = Promise.pending();
    resolve(Js.Promise.resolve());
    p->Promise.flatMap(p => Promise.resolved(jsPromiseIsPromise(p)));
  }),

  test("resolved: JS promise", () => {
    Promise.resolved(Js.Promise.resolve())
    ->Promise.flatMap(p => Promise.resolved(jsPromiseIsPromise(p)));
  }),

  test("rejected: JS promise", () => {
    Promise.Js.rejected(Js.Promise.resolve(42))
    ->Promise.Js.catch(p =>
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
    let (p1, resolve) = Promise.pending();
    let p2 = Promise.all([p1]);
    resolve(Promise.resolved(42));
    p2->Promise.flatMap(results =>
      switch (results) {
      | [maybePromise] => isPromiseResolvedWith42(maybePromise)
      | _ => Promise.resolved(false)
      });
  }),

  test("all, rejection", () => {
    let (p1, _, reject) = Promise.Js.pending();
    let p2 = Promise.Js.all([p1]);
    reject(Promise.resolved(42));
    p2
    ->Promise.Js.map((_) => false)
    ->Promise.Js.catch(isPromiseResolvedWith42);
  }),

  test("race", () => {
    let (p1, resolve) = Promise.pending();
    let p2 = Promise.race([p1]);
    resolve(Promise.resolved(42));
    p2->Promise.flatMap(isPromiseResolvedWith42);
  }),

  test("race, rejection", () => {
    let (p1, _, reject) = Promise.Js.pending();
    let p2 = Promise.Js.race([p1]);
    reject(Promise.resolved(42));
    p2
    ->Promise.Js.map((_) => false)
    ->Promise.Js.catch(isPromiseResolvedWith42);
  }),
]);



let curryTests = Framework.suite("curry", [
  test("partially applied", () => {
    let add = (a, b) => a + b;
    Promise.resolved(1)
    ->Promise.map(add(1))
    ->Promise.map(n => n == 2);
  }),

  test("partially applied, cascade", () => {
    let add3 = (a, b, c) => a + b + c;
    Promise.resolved(1)
    ->Promise.map(add3(2))
    ->Promise.map(f => f(3))
    ->Promise.map(n => n == 6);
  }),
]);



type type_ = [
  | `A
  | `B
]

type subtype = [
  | `A
]

let covarianceTests = Framework.suite("covariance", [
  test("promise", () => {
    let p: Promise.t(subtype) = Promise.resolved(`A);
    let p: Promise.t(type_) = (p :> Promise.t(type_));
    ignore(p);
    Promise.resolved(true);
  }),

  test("ok", () => {
    let p: Promise.t(result(subtype, unit)) = Promise.resolved(Ok(`A));
    let p: Promise.t(result(type_, unit)) =
      (p :> Promise.t(result(type_, unit)));
    ignore(p);
    Promise.resolved(true);
  }),

  test("error", () => {
    let p: Promise.t(result(unit, subtype)) = Promise.resolved(Error(`A));
    let p: Promise.t(result(unit, type_)) =
      (p :> Promise.t(result(unit, type_)));
    ignore(p);
    Promise.resolved(true);
  }),

  test("option", () => {
    let p: Promise.t(option(subtype)) = Promise.resolved(Some(`A));
    let p: Promise.t(option(type_)) = (p :> Promise.t(option(type_)));
    ignore(p);
    Promise.resolved(true);
  }),

  test("fulfillment", () => {
    let p: Promise.Js.t(subtype, unit) = Promise.Js.resolved(`A);
    let p: Promise.Js.t(type_, unit) = (p :> Promise.Js.t(type_, unit));
    ignore(p);
    Promise.resolved(true);
  }),

  test("rejection", () => {
    let p: Promise.Js.t(unit, subtype) = Promise.Js.rejected(`A);
    let p: Promise.Js.t(unit, type_) = (p :> Promise.Js.t(unit, type_));
    p->Promise.Js.catch(_ => Promise.resolved())->ignore;
    Promise.resolved(true);
  }),
]);



let suites = [interopTests, soundnessTests, curryTests, covarianceTests];
