let tests = [
  Test_promise.suite,
  Test_ffi.suite,
  Test_io.suite,
];

let () =
  Framework.run("repromise", tests);
