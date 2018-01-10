/* OCaml promise library
 * http://www.ocsigen.org/lwt
 * Copyright (C) 2009 Jérémie Dimino
 * Copyright (C) 2017-2018 Anton Bachin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, with linking exceptions;
 * either version 2.1 of the License, or (at your option) any later
 * version. See COPYING file for details.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */



/* This is a vendored copy/port of Lwt's tester – it is the only OCaml
   fully-asynchronous tester already available. For the file's history and blame
   before import, see
     https://github.com/ocsigen/lwt/blob/c7ad8b3/test/test.ml */

/* There are a few features in here that we are currently not using. We should
   probably delete them eventually. It's a bit of a blind conversion. A few
   syntactic constructs were well-adapted to OCaml, but don't look so legible in
   Reason, meanwhile Reason offers other syntax advantages. */



type test = {
  test_name: string,
  skip_if_this_is_false: unit => bool,
  run: unit => Promise.t(bool),
};

type outcome =
  | Passed
  | Failed
  | Skipped;

let test = (test_name, ~only_if = () => true, run) =>
  {test_name, skip_if_this_is_false: only_if, run};

let run_test = test =>
  if (test.skip_if_this_is_false() == false) {
    Promise.resolve(Skipped)
  }
  else {
    test.run()
    |> Promise.then_(test_did_pass =>
      if (test_did_pass) {
        Promise.resolve(Passed)
      }
      else {
        Promise.resolve(Failed)
      })
  };

/* We don't support exception handling in the tester for now, largely because
   the [Promise] module doesn't know what to do about exceptions at this point.
   Future work. */
let outcome_to_character = fun
  | Passed => '.'
  | Failed => 'F'
  | Skipped => 'S';



type suite = {
  suite_name: string,
  suite_tests: list(test),
  skip_entire_suite_if_this_is_false: unit => bool,
};

/* Test names paired with the corresponding outcomes. */
type suite_outcomes = list((string, outcome));

let suite = (name, ~only_if = () => true, tests) =>
  {suite_name: name,
   suite_tests: tests,
   skip_entire_suite_if_this_is_false: only_if};

let run_test_suite: suite => Promise.t(suite_outcomes) = suite =>
  if (suite.skip_entire_suite_if_this_is_false() == false) {
    /* For the outcome list, list all tests in the suite as skipped. */
    let outcomes =
      suite.suite_tests
      |> List.map(({test_name, _}) => (test_name, Skipped));

    /* Print a number of Skipped (S) symbols equal to the number of tests in the
       suite. */
    outcome_to_character(Skipped)
    |> String.make(List.length(outcomes))
    |> print_string;
    Pervasives.flush(stdout);

    Promise.resolve(outcomes);
  }
  else {
    let rec run_each_test(tests, reversed_outcomes) =
      switch tests {
      | [] => Promise.resolve(List.rev(reversed_outcomes))
      | [test, ...more_tests] =>
        run_test(test)
        |> Promise.then_(new_outcome => {
          new_outcome |> outcome_to_character |> print_char;
          Pervasives.flush(stdout);
          let outcome_with_name = (test.test_name, new_outcome);
          run_each_test(more_tests, [outcome_with_name, ...reversed_outcomes]);
        })
      };
    run_each_test(suite.suite_tests, []);
  };

let outcomes_all_ok =
  List.for_all(((_test_name, outcome)) =>
    switch outcome {
    | Passed | Skipped => true
    | Failed => false
    });

let show_failures =
  List.iter(((test_name, outcome)) =>
    switch outcome {
    | Passed | Skipped => ()
    | Failed => Printf.eprintf("Test '%s' produced 'false'\n", test_name)
    });



/* Suite names paired with all the outcomes from all the tests in each suite. */
type aggregated_outcomes = list((string, suite_outcomes));

let fold_over_outcomes = (init, f, aggregated_outcomes) => {
  let apply_to_single_test_outcome =
    suite_name => (accumulator, (test_name, outcome)) =>
      f(accumulator, ~suite_name, ~test_name, outcome);

  let apply_to_suite_outcomes = (accumulator, (suite_name, suite_outcomes)) =>
    List.fold_left(
      apply_to_single_test_outcome(suite_name), accumulator, suite_outcomes);

  List.fold_left(apply_to_suite_outcomes, init, aggregated_outcomes);
};

let count_ran: aggregated_outcomes => int =
  fold_over_outcomes(0, (count, ~suite_name as _, ~test_name as _) => fun
    | Skipped => count
    | _ => count + 1);

let count_skipped: aggregated_outcomes => int =
  fold_over_outcomes(0, (count, ~suite_name as _, ~test_name as _) => fun
    | Skipped => count + 1
    | _ => count);

/* Runs a series of test suites. If one of the test suites fails, does not run
   subsequent suites. */
let run = (library_name, suites) => {
  Printexc.register_printer(fun
    | Failure(message) => Some(Printf.sprintf("Failure(%S)", message))
    | _ => None);

  Printf.printf("Testing library '%s'...\n", library_name);

  let rec loop_over_suites = (aggregated_outcomes, suites) =>
    switch suites {
    | [] =>
      Printf.printf(
        "\nOk. %i tests ran, %i tests skipped\n",
        count_ran(aggregated_outcomes),
        count_skipped(aggregated_outcomes));
      Promise.resolve();

    | [suite, ...rest] =>
      run_test_suite(suite)
      |> Promise.then_(outcomes =>
        if (not(outcomes_all_ok(outcomes))) {
          print_newline();
          Pervasives.flush(stdout);
          Printf.eprintf("Failures in test suite '%s':\n", suite.suite_name);
          show_failures(outcomes);
          Pervasives.exit(1);
        }
        else {
          loop_over_suites(
            [(suite.suite_name, outcomes), ...aggregated_outcomes], rest);
        })
    };

  loop_over_suites([], suites) |> ignore;

  Io.run();
};
