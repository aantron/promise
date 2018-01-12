type never = Repromise.never;

let loop = Libuv_loop.default ();

let rec run = () => {
  /* The first task in each "tick" is to figure out the full set of callbacks to
     be called in that tick. That set is:

     1. all the ready callbacks that were added in the previous tick by the
        [Repromise] module, which are already sitting in
        [Repromise.ready_callbacks^], and
     2. any callbacks that result from completed I/O.

     To get the I/O callbacks (2), we first have to drive the libuv loop. The
     way we do that depends on whether we already have ready callbacks from the
     previous tick (1):

     - If we do, we want to give I/O callbacks (2) a chance to be added to the
       callback queue, but we don't want to wait for I/O that has not already
       completed, because we need to call the existing ready callbacks as soon
       as possible. So, in this case, we want to only poll for I/O.
     - If we don't have callbacks from the previous tick (1), then we are out of
       pure Reason/OCaml code to call, so the only way to ever get a callback
       again is if some I/O completes. So, in this case, we want to put our
       thread to sleep until then. */
  let libuv_loop_run_mode =
    switch Repromise.readyCallbacks^ {
      | [] => `One_iteration
      | _ => `Poll_only
    };

  let io_status = Libuv_loop.run(loop, libuv_loop_run_mode);
  let callbacks_for_this_tick = Repromise.readyCallbacks^;

  /* If we don't have any callbacks to run, and also libuv says that no I/O is
     pending, then we should stop the loop by returning from this function
     [run], which typically terminates the program, because [run] is typically
     the last thing called by the user. */
  switch (io_status, callbacks_for_this_tick) {
    | (`Done, []) => ()
    | _ => {
      /* We have either callbacks or more I/O waiting. Call all the callbacks.

         While the callbacks are running, they can schedule more I/O, and they
         can also directly add new callbacks to the ready callback queue.
         However, we won't run those new callbacks until the next tick, because
         we snapshotted the callback list into [callbacks_for_this_tick]. */
      Repromise.readyCallbacks := [];
      callbacks_for_this_tick |> List.iter(callback => callback());

      /* Repeat the loop. */
      run();
    }
  }
};

type fd = int;

let open_ = filename =>
  Repromise.new_((resolve, _) =>
    Libuv_fs.Async.open_(loop, filename, ~flags = 0, ~mode = 0, resolve));

let read = (~fd, ~length) => {
  let buffer = Bytes.create(length);
  Repromise.new_((resolve, _) =>
    Libuv_fs.Async.read(loop, fd, buffer, bytes_read =>
      resolve(Bytes.sub_string(buffer, 0, bytes_read))))
};
