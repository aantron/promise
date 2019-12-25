/* This file is part of reason-promise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/promise/blob/master/LICENSE.md. */

/* Mutable doubly-linked lists, like in a typical imperative language. These are
   used for callback lists, because reason-promise needs fast append and fast
   deletion of any node in the list, when the reference to the target node is
   already be held by the deleting code. */

type list('a);
type node('a);

let create: unit => list('a);
let isEmpty: list(_) => bool;
let append: (list('a), 'a) => node('a);
let iter: ('a => unit, list('a)) => unit;
let remove: (list('a), node('a)) => unit;

/* Concatenates list1 and list2. Afterwards, the reference list1 has a correct
   internal list structure, and the reference list2 should not be used
   anymore. */
let concatenate: (list('a), list('a)) => unit;
