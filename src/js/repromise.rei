/* This file is part of Repromise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/repromise/blob/master/LICENSE.md. */



type rejectable('a, 'e);
type never;

type promise('a) = rejectable('a, never);
type t('a) = promise('a);



/* Main API. */
let make: unit => (promise('a), 'a => unit);

let exec: (('a => unit) => unit) => promise('a);

let resolved: 'a => promise('a);

let map: ('a => 'b, promise('a)) => promise('b);

let wait: ('a => unit, promise('a)) => unit;

let andThen: ('a => promise('b), promise('a)) => promise('b);

let all: list(promise('a)) => promise(list('a));

let all2: (promise('a), promise('b)) => promise(('a, 'b));

let all3: (promise('a), promise('b), promise('c)) => promise(('a, 'b, 'c));

let all4:
  (promise('a), promise('b), promise('c), promise('d)) =>
    promise(('a, 'b, 'c, 'd));

let all5:
  (promise('a), promise('b), promise('c), promise('d), promise('e)) =>
    promise(('a, 'b, 'c, 'd, 'e));

let all6:
  (promise('a),
   promise('b),
   promise('c),
   promise('d),
   promise('e),
   promise('f)) =>
      promise(('a, 'b, 'c, 'd, 'e, 'f));

let arrayAll: array(promise('a)) => promise(array('a));

let race: list(promise('a)) => promise('a);



/* Results. */

/* Compatibility with BuckleScript < 6. */
type result('a, 'e) = Belt.Result.t('a, 'e);

let mapOk:
  ('a => 'b, promise(result('a, 'e))) => promise(result('b, 'e));

let mapError:
  ('e => 'e2, promise(result('a, 'e))) => promise(result('a, 'e2));

let waitOk:
  ('a => unit, promise(result('a, _))) => unit;

let waitError:
  ('e => unit, promise(result(_, 'e))) => unit;

let andThenOk:
  ('a => promise(result('b, 'e)), promise(result('a, 'e))) =>
    promise(result('b, 'e));

let andThenError:
  ('e => promise(result('a, 'e2)), promise(result('a, 'e))) =>
    promise(result('a, 'e2));

module Operators: {
  let (>|=):
    (promise(result('a, 'e)), 'a => 'b) => promise(result('b, 'e));

  let (>>=):
    (promise(result('a, 'e)), 'a => promise(result('b, 'e))) =>
      promise(result('b, 'e));
};



/* Options. */
let mapSome:
  ('a => 'b, promise(option('a))) => promise(option('b));

let waitSome:
  ('a => unit, promise(option('a))) => unit;

let andThenSome:
  ('a => promise(option('b)), promise(option('a))) => promise(option('b));



/* For writing bindings. */
module Rejectable: {
  type t('a, 'e) = rejectable('a, 'e);

  let relax: promise('a) => rejectable('a, _);

  let make: unit => (rejectable('a, 'e), 'a => unit, 'e => unit);

  let resolved: 'a => rejectable('a, _);

  let rejected: 'e => rejectable(_, 'e);

  let map: ('a => 'b, rejectable('a, 'e)) => rejectable('b, 'e);

  let wait: ('a => unit, rejectable('a, _)) => unit;

  let andThen:
    ('a => rejectable('b, 'e), rejectable('a, 'e)) => rejectable('b, 'e);

  let catch:
    ('e => rejectable('a, 'e2), rejectable('a, 'e)) => rejectable('a, 'e2);

  let all: list(rejectable('a, 'e)) => rejectable(list('a), 'e);

  let race: list(rejectable('a, 'e)) => rejectable('a, 'e);

  let fromJsPromise: Js.Promise.t('a) => rejectable('a, Js.Promise.error);

  let toJsPromise: rejectable('a, _) => Js.Promise.t('a);
};



let onUnhandledException: ref(exn => unit);
