module C = Ctypes
let foreign = Foreign.foreign



external _workaround : unit -> unit = "ocaml_luv_jbuilder_workaround"



let request = C.(ptr void)

let allocate_request =
  foreign "ocaml_luv_allocate_uv_fs_t"
    C.(void @-> returning request)

let free_request =
  foreign "free"
    C.(request @-> returning void)



let buffers = C.(ptr void)

let allocate_buffers =
  foreign "ocaml_luv_allocate_uv_buf_ts"
    C.(int @-> returning buffers)

(* TODO Fix the types up; last int should be unsigned, and probably need various
   bounds checks here. *)
let set_buffer =
  foreign "ocaml_luv_set_uv_buf_t"
    C.(buffers @-> int @-> ptr char @-> int @-> returning void)

(* TODO Need to check errors around this. *)
let duplicate_string =
  foreign "ocaml_luv_duplicate_buffer"
    C.(ocaml_bytes @-> int @-> returning (ptr char))

let memcpy =
  foreign "memcpy"
    C.(ocaml_bytes @-> ptr char @-> int @-> returning void)



module Sync =
struct
  let with_request f =
    (* TODO Need to do try-with here. *)
    let request = allocate_request () in
    let result = f request in
    free_request request;
    result

  let no_callback = C.(ptr void)

  (* TODO What is the return value in the sync case? *)
  (* TODO Check for errors. *)
  let open_ : Libuv_loop.t -> string -> flags:int -> mode:int -> int =
    let uv_fs_open =
      foreign "uv_fs_open"
        C.(Libuv_loop.representation @->
           request @->
           string @->
           int @->
           int @->
           no_callback @->
             returning int)
    in
    fun loop filename ~flags ~mode ->
      with_request (fun request ->
        uv_fs_open loop request filename flags mode C.null)

  let read : Libuv_loop.t -> int -> bytes -> int =
    let uv_fs_read =
      foreign "uv_fs_read"
        C.(Libuv_loop.representation @->
           request @->
           int @->
           buffers @->
           int @->
           int64_t @->
           no_callback @->
             returning int)
    in
    fun loop fd buffer ->
      (* TODO Need some kind of with_buffers. *)
      (* TODO Leaking memory here. *)
      let buffer' =
        duplicate_string (C.ocaml_bytes_start buffer) (Bytes.length buffer) in
      let buffers = allocate_buffers 1 in
      set_buffer buffers 0 buffer' (Bytes.length buffer);
      let result =
        with_request (fun request ->
          uv_fs_read loop request fd buffers 1 0L C.null)
      in
      memcpy (C.ocaml_bytes_start buffer) buffer' (Bytes.length buffer);
      result
end



module Async =
struct
  let callback = Foreign.funptr C.(request @-> returning void)

  let get_result =
    foreign "ocaml_luv_uv_fs_t_get_result"
      C.(request @-> returning int)

  (* TODO Error handling. *)
  let open_
      : Libuv_loop.t -> string -> flags:int -> mode:int -> (int -> unit) ->
          unit =
    let uv_fs_open =
      foreign "uv_fs_open"
        C.(Libuv_loop.representation @->
           request @->
           string @->
           int @->
           int @->
           callback @->
             returning int)
    in
    fun loop filename ~flags ~mode callback ->
      (* TODO We have a nice, juicy memory leak here. *)
      let request = allocate_request () in
      (* TODO Ignoring the return value. *)
      uv_fs_open loop request filename flags mode (fun request ->
        callback (get_result request))
      |> ignore

  let read : Libuv_loop.t -> int -> bytes -> (int -> unit) -> unit =
    let uv_fs_read =
      foreign "uv_fs_read"
        C.(Libuv_loop.representation @->
           request @->
           int @->
           buffers @->
           int @->
           int64_t @->
           callback @->
             returning int)
    in
    (* TODO No error handling, return value check, have a memory leak, etc. *)
    fun loop fd buffer callback ->
      let buffer' =
        duplicate_string (C.ocaml_bytes_start buffer) (Bytes.length buffer) in
      let buffers = allocate_buffers 1 in
      let request = allocate_request () in
      set_buffer buffers 0 buffer' (Bytes.length buffer);
      uv_fs_read loop request fd buffers 1 0L (fun request ->
        let result = get_result request in
        if result > 0 then
          memcpy (C.ocaml_bytes_start buffer) buffer' result;
        callback (get_result request))
      |> ignore
end
