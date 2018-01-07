module Sync: {
  /* TODO Type of flags? */
  /* TODO Type of mode? */
  /* TODO Wrap as fd; error handling. */
  let open_: (Libuv_loop.t, string, ~flags: int, ~mode: int) => int;
  /* TODO Support the generalized read. */
  let read: (Libuv_loop.t, int, bytes) => int;
};



module Async: {
  let open_:
    (Libuv_loop.t, string, ~flags: int, ~mode: int, int => unit) => unit;
  let read: (Libuv_loop.t, int, bytes, int => unit) => unit;
};