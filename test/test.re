let tests =
  Test_promise.suites
  @ Test_ffi.suites;

let () =
  Framework.run("repromise", tests);
