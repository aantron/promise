type promise('a, 'e);
type t('a, 'e) = promise('a, 'e);

type never;

let new_: (('a => unit) => ('e => unit) => unit) => promise('a, 'e);

let resolve: 'a => promise('a, _);

let reject: 'e => promise(_, 'e);

let then_: ('a => promise('b, 'e), promise('a, 'e)) => promise('b, 'e);

let map: ('a => 'b, promise('a, 'e)) => promise('b, 'e);

let catch: ('e => promise('a, 'e2), promise('a, 'e)) => promise('a, 'e2);

let onUnhandledException: ref(exn => never);

/* This shouldn't really be visible in the API. It is used by the native Io to
   drive the native promise callbacks. It is not used on JavaScript, because
   Node and browsers call callbacks themselves. */
let readyCallbacks: ref(list(unit => unit));
