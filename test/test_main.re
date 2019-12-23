/* This file is part of reason-promise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/promise/blob/master/LICENSE.md. */



let tests =
  Test_promise.suites
  @ Test_ffi.suites;

let () =
  Framework.run("reason-promise", tests);
