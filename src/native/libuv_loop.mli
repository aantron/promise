(* TODO Make this abstract at all costs. *)
type t = unit Ctypes.ptr

val default : unit -> t
(* TODO This can fail, should it return an option then? I think the failure is
   extremely rare, and probably program-stopping, so it may be wiser to raise an
   exception. *)

val run :
  t -> [ `Until_stop | `One_iteration | `Poll_only ] ->
    [ `Done | `Requests_outstanding ]



(* TODO Ideally, hide this. *)
(**/**)
val representation : t Ctypes.typ
