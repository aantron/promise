module C = Ctypes;
let foreign = Foreign.foreign;



type t = C.ptr(unit);
let representation = C.(ptr(void));

let default =
  foreign("uv_default_loop", C.(void @-> returning(representation)));

let run = {
  let uv_run = foreign("uv_run", C.(representation @-> int @-> returning(int)));

  let uv_run_default = 0;
  let uv_run_once = 1;
  let uv_run_nowait = 2;

  (loop, mode) => {
    let mode =
      switch mode {
      | `Until_stop => uv_run_default
      | `One_iteration => uv_run_once
      | `Poll_only => uv_run_nowait
      };

    let result = uv_run(loop, mode);
    
    switch result {
    | 0 => `Done
    | _ => `Requests_outstanding
    };
  };
};