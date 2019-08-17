let tests =
  Test_repromise.suites
  @ Test_ffi.suites;

let () =
  Framework.run("repromise", tests);

[@coverage exclude_file]
