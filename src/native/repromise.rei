type rejectable('a, 'e);
type never;

type promise('a) = rejectable('a, never);
type t('a) = promise('a);



let make: unit => (promise('a), 'a => unit);

let resolved: 'a => promise('a);

let andThen: ('a => promise('b), promise('a)) => promise('b);

let map: ('a => 'b, promise('a)) => promise('b);

let wait: ('a => unit, promise('a)) => unit;

let all: list(promise('a)) => promise(list('a));

let race: list(promise('a)) => promise('a);



module Rejectable: {
  type t('a, 'e) = rejectable('a, 'e);

  let relax: promise('a) => rejectable('a, _);

  let make: unit => (rejectable('a, 'e), 'a => unit, 'e => unit);

  let resolved: 'a => rejectable('a, _);

  let rejected: 'e => rejectable(_, 'e);

  let andThen:
    ('a => rejectable('b, 'e), rejectable('a, 'e)) => rejectable('b, 'e);

  let map: ('a => 'b, rejectable('a, 'e)) => rejectable('b, 'e);

  let wait: ('a => unit, rejectable('a, _)) => unit;

  let catch:
    ('e => rejectable('a, 'e2), rejectable('a, 'e)) => rejectable('a, 'e2);

  let all: list(rejectable('a, 'e)) => rejectable(list('a), 'e);

  let race: list(rejectable('a, 'e)) => rejectable('a, 'e);
};



let onUnhandledException: ref(exn => never);



/* This shouldn't really be visible in the API. It is used by the native Io to
   drive the native promise callbacks. It is not used on JavaScript, because
   Node and browsers call callbacks themselves. */

module ReadyCallbacks: {
  let callbacksPending: unit => bool;

  /* When about to iterate over the ready callbacks, Repromise first takes a
     snapshot of them, and iterates over the snapshot. This is to prevent new
     ready callbacks, that may be created by the processing of the current ones,
     from being processed immediately. That could lead to I/O loop starvation
     and other problems. */
  type snapshot;

  let snapshot: unit => snapshot;
  let isEmpty: snapshot => bool;
  let call: snapshot => unit;
};
