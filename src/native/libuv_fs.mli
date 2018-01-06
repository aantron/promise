module Sync :
sig
  (* TODO Type of flags? *)
  (* TODO Type of mode? *)
  (* TODO Wrap as fd; error handling. *)
  val open_ : Libuv_loop.t -> string -> flags:int -> mode:int -> int
  (* TODO Support the generalized read. *)
  val read : Libuv_loop.t -> int -> bytes -> int
end



module Async :
sig
  val open_ :
    Libuv_loop.t -> string -> flags:int -> mode:int -> (int -> unit) -> unit
  val read : Libuv_loop.t -> int -> bytes -> (int -> unit) -> unit
end
