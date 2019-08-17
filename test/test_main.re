/* This file is part of Repromise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/repromise/blob/master/LICENSE.md. */



let tests =
  Test_repromise.suites
  @ Test_ffi.suites;

let () =
  Framework.run("repromise", tests);

[@coverage exclude_file]
