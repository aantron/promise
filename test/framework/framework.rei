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
     https://github.com/ocsigen/lwt/blob/c7ad8b3/test/test.mli */



/** Helpers for tests. */

type test;
type suite;

let test: (string, ~only_if: unit => bool = ?, unit => Promise.t(bool)) => test;
/** Like [test_direct], but defines a test which runs a thread. */

let suite: (string, ~only_if: unit => bool = ?, list(test)) => suite;
/** Defines a suite of tests */

let run: (string, list(suite)) => unit;
/** Run all the given tests and exit the program with an exit code
    of [0] if all tests succeeded and with [1] otherwise. */
