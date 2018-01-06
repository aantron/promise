module C = Ctypes
let foreign = Foreign.foreign



type t = unit C.ptr
let representation = C.(ptr void)



let default =
  foreign "uv_default_loop" C.(void @-> returning representation)

let run =
  let uv_run = foreign "uv_run" C.(representation @-> int @-> returning int) in

  let uv_run_default = 0 in
  let uv_run_once = 1 in
  let uv_run_nowait = 2 in

  fun loop mode ->
    let mode =
      match mode with
      | `Until_stop -> uv_run_default
      | `One_iteration -> uv_run_once
      | `Poll_only -> uv_run_nowait
    in

    let result = uv_run loop mode in

    match result with
    | 0 -> `Done
    | _ -> `Requests_outstanding
