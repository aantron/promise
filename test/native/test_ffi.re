let test = Framework.test;

let loop = Libuv_loop.default ();

let suite = Framework.suite("libuv", [
  /* These are janky proof-of-concept test. We are not even trying to close the
     file. */
  test("open_ sync", () => {
    let fd = Libuv_fs.Sync.open_(loop, "test/test.re", ~flags = 0, ~mode = 0);
    Repromise.resolve(fd > 0);
  }),

  test("open_ async", () => {
    Repromise.new_((~resolve) =>
      Libuv_fs.Async.open_(loop, "test/test.re", ~flags = 0, ~mode = 0, fd =>
        resolve(fd > 0)));
  }),
]);
