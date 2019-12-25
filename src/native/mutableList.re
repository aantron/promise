/* This file is part of reason-promise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/promise/blob/master/LICENSE.md. */

type node('a) = {
  mutable previous: option(node('a)),
  mutable next: option(node('a)),
  content: 'a,
};

type listEnds('a) = {
  mutable first: node('a),
  mutable last: node('a),
};

type list('a) = ref([ | `Empty | `NonEmpty(listEnds('a))]);

let create = () => ref(`Empty);

let isEmpty = list => list^ == `Empty;

let append = (list, value) =>
  switch (list^) {
  | `Empty =>
    let node = {previous: None, next: None, content: value};
    list := `NonEmpty({first: node, last: node});
    node;

  | `NonEmpty(ends) =>
    let node = {previous: Some(ends.last), next: None, content: value};
    ends.last.next = Some(node);
    ends.last = node;
    node;
  };

let concatenate = (list1, list2) =>
  switch (list2^) {
  | `Empty =>
    /* If the second list is empty, we can just return the first list, because
       it already has the correct final structure, and there is nothing to
       do. */
    ()

  | `NonEmpty(list2Ends) =>
    switch (list1^) {
    | `Empty =>
      /* If the second list is non-empty, but the first list is empty, we need
         to change the end-of-list references in the first list to point to the
         structure of the second list. This is because the caller depends on the
         first list having the correct structure after the call. */
      list1 := list2^

    | `NonEmpty(list1Ends) =>
      /* Otherwise, we have to splice the ending nodes of the two lists. */

      list1Ends.last.next = Some(list2Ends.first);
      list2Ends.first.previous = Some(list1Ends.last);
      list1Ends.last = list2Ends.last;
    }
  };

let iter = (callback, list) =>
  switch (list^) {
  | `Empty => ()

  | `NonEmpty(ends) =>
    let rec loop = node => {
      callback(node.content);
      switch (node.next) {
      | None => ()
      | Some(nextNode) => loop(nextNode)
      };
    };

    loop(ends.first);
  };

let remove = (list, node) => {
  /* This function is difficult enough to implement and use that it is
     probably time to switch representations for callback lists soon. */
  switch (list^) {
  | `Empty => ()

  | `NonEmpty(ends) =>
    switch (node.previous) {
    | None =>
      if (ends.first === node) {
        switch (node.next) {
        | None => list := `Empty
        | Some(secondNode) => ends.first = secondNode
        };
      }

    | Some(previousNode) => previousNode.next = node.next
    };

    switch (node.next) {
    | None =>
      if (ends.last === node) {
        switch (node.previous) {
        | None => list := `Empty
        | Some(secondToLastNode) => ends.last = secondToLastNode
        };
      }

    | Some(nextNode) => nextNode.previous = node.previous
    };
  };

  node.previous = None;
  node.next = None;
};
