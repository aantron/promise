/* TODO Make this abstract at all costs. */
type t = Ctypes.ptr(unit);

let default: unit => t;
/* TODO This can fail, should it return an option then? I think the failure is
   extremely rare, and probably program-stopping, so it may be wiser to raise an
   exception. */
   
let run:
  (t, [ | `Until_stop | `One_iteration | `Poll_only]) =>
  [ | `Done | `Requests_outstanding];

/* TODO Ideally, hide this. */

let representation: Ctypes.typ(t);