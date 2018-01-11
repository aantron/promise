let tests =
  [Test_promise.suite]
  @ Test_ffi.suites
  @ [Test_io.suite];

let () =
  Framework.run("repromise", tests);
