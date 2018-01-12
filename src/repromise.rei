type promise('a);
type t('a) = promise('a);

let new_: (('a => unit) => unit) => promise('a);

let resolve: 'a => promise('a);

let then_: ('a => promise('b), promise('a)) => promise('b);

/* This shouldn't really be visible in the API. It is used by the native Io to
   drive the native promise callbacks. It is not used on JavaScript, because
   Node and browsers call callbacks themselves. */
let ready_callbacks: ref(list(unit => unit));
