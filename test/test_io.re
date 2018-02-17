let test = Framework.test;

let suite = Framework.suite("io", [
  /* This test is completely illegitimate and should be removed later, but
     ok. We have pretty much nothing else to test. */
  test("open_", () => {
    let%await fd = Io.open_("test/test_io.re");
    Repromise.resolve (Obj.magic(fd) > 0);
  }),

  test("read", () => {
    let%await fd = Io.open_("test/test_io.re");
    let%await data = Io.read(~fd, ~length = 3);
    Printf.eprintf("%s %i %s", data, String.length(data), Sys.getcwd());
    Repromise.resolve(data == "let");
  }),
]);
