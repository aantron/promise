/* Mutable doubly-linked lists, like in a typical imperative language. These are
   used for callback lists, because Repromise needs fast deletions of any node
   in the list (the reference to the target node will already be held by the
   deleting code), and fast append. */
module MutableList = {
  type node('a) = {
    mutable previous: option(node('a)),
    mutable next: option(node('a)),
    content: 'a,
  };

  type listEnds('a) = {
    mutable first: node('a),
    mutable last: node('a),
  };

  type list('a) = ref([
    | `Empty
    | `NonEmpty(listEnds('a))
  ]);

  let create = () =>
    ref(`Empty);

  let isEmpty = list =>
    list^ == `Empty;

  let append = (list, value) =>
    switch (list^) {
    | `Empty =>
      let node = {
        previous: None,
        next: None,
        content: value,
      };
      list := `NonEmpty({first: node, last: node});
      node;

    | `NonEmpty(ends) =>
      let node = {
        previous: Some(ends.last),
        next: None,
        content: value,
      };
      ends.last.next = Some(node);
      ends.last = node;
      node;
    };

  /* Concatenates list1 and list2. Afterwards, the reference list1 has a
     correct internal list structure, and the reference list2 should not be
     used anymore. */
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
        /* If the second list is non-empty, but the first list is empty, we
           need to change the end-of-list references in the first list to point
           to the structure of the second list. This is because the caller
           depends on the first list having the correct structure after the
           call. */
        list1 := list2^;

      | `NonEmpty(list1Ends) =>
        /* Otherwise, we have to splice the ending nodes of the two lists. */

        list1Ends.last.next = Some(list2Ends.first);
        list2Ends.first.previous = Some(list1Ends.last);
        list1Ends.last = list2Ends.last;
      }
    };

  let iter = (callback, list) =>
    switch (list^) {
    | `Empty =>
      ()

    | `NonEmpty(ends) =>
      let rec loop = node => {
        callback(node.content);
        switch (node.next) {
        | None =>
          ();
        | Some(nextNode) =>
          loop(nextNode);
        };
      };

      loop(ends.first);
    };

  let remove = (list, node) => {
    /* This function is difficult enough to implement and use that it is
       probably time to switch representations for callback lists soon. */
    switch (list^) {
    | `Empty =>
      ()

    | `NonEmpty(ends) =>
      switch (node.previous) {
      | None =>
        if (ends.first === node) {
          switch (node.next) {
          | None =>
            list := `Empty
          | Some(secondNode) =>
            ends.first = secondNode
          }
        }

      | Some(previousNode) =>
        previousNode.next = node.next
      };

      switch (node.next) {
      | None =>
        if (ends.last === node) {
          switch (node.previous) {
          | None =>
            list := `Empty
          | Some(secondToLastNode) =>
            ends.last = secondToLastNode
          }
        }

      | Some(nextNode) =>
        nextNode.previous = node.previous
      };
    };

    node.previous = None;
    node.next = None;
  };
};



type callbacks('a, 'e) = {
  onResolve: MutableList.list('a => unit),
  onReject: MutableList.list('e => unit),
};



type promise('a, 'e) =
  ref([
    | `Fulfilled('a)
    | `Rejected('e)
    | `Pending(callbacks('a, 'e))
    | `Merged(promise('a, 'e))
  ]);

type t('a, 'e) = promise('a, 'e);

type never;



/* The `Merged constructor and this function, underlying, are used to avoid a
   memory leak that arises when then_ is called on promises in a loop. See the
   description in the associated test "promise loop memory leak". The rest of
   this comment is based on that description.

   The solution to the memory leak is to merge nested promises created on the
   second and subsequent iterations of loops into the outer promise created on
   the first iteration. This is performed by the internal helper
   makePromiseBehaveAs, below.

   When promises are merged, the callback lists of the nested promise are
   merged into the callback lists of the outer promise, and afterwards the
   nested promise object becomes just a proxy that refers to the outer promise.
   As a result, most internal operations on promises have to first call
   underlying, in order to find the true merged (outer) promise on which
   operations should be performed, rather than working directly on proxies. */
let rec underlying = p =>
  switch p^ {
  | `Fulfilled(_)
  | `Rejected(_)
  | `Pending(_) =>
    p;

  | `Merged(p') =>
    let p'' = underlying(p');
    if (p'' !== p') {
      p := `Merged(p'')
    };
    p'';
  };



let onUnhandledException = ref(exn => {
  prerr_endline("Unhandled exception in promise callback:");
  prerr_endline(Printexc.to_string(exn));
  Printexc.print_backtrace(stderr);
  exit(2);
});



module ReadyCallbacks = {
  let callbacks: ref(MutableList.list(unit => unit)) =
    ref(MutableList.create());

  let callbacksPending = () =>
    not(MutableList.isEmpty(callbacks^));

  let defer = (callback, value) =>
    MutableList.append(callbacks^, () => callback(value)) |> ignore;

  let deferMultiple = (newCallbacks, value) =>
    newCallbacks
    |> MutableList.iter(callback => defer(callback, value));

  type snapshot = MutableList.list(unit => unit);

  let snapshot = () => {
    let theSnapshot = callbacks^;
    callbacks := MutableList.create();
    theSnapshot;
  };

  let isEmpty = snapshot =>
    MutableList.isEmpty(snapshot);

  let call = snapshot =>
    snapshot |> MutableList.iter(callback => callback());
};



let newInternal = () =>
  ref(`Pending({
    onResolve: MutableList.create(),
    onReject: MutableList.create()
  }));

let resolveInternal = p => value =>
  switch (underlying(p))^ {
  | `Fulfilled(_)
  | `Rejected(_) =>
    ()
  | `Pending(callbacks) =>
    ReadyCallbacks.deferMultiple(callbacks.onResolve, value);
    p := `Fulfilled(value);
  | `Merged(_) =>
    /* This case is impossible, because we called underyling on the promise,
       above. */
    assert(false);
  };

let rejectInternal = p => error =>
  switch (underlying(p))^ {
  | `Fulfilled(_)
  | `Rejected(_) =>
    ()
  | `Pending(callbacks) =>
    ReadyCallbacks.deferMultiple(callbacks.onReject, error);
    p := `Rejected(error);
  | `Merged(_) =>
    /* This case is impossible, because we called underyling on the promise,
       above. */
    assert(false);
  };



let new_ = executor => {
  let p = newInternal();
  executor(resolveInternal(p), rejectInternal(p));
  p
};



let resolve = value =>
  ref(`Fulfilled(value));

let reject = error =>
  ref(`Rejected(error));



let makePromiseBehaveAs = (outerPromise, nestedPromise) => {
  let underlyingNested = underlying(nestedPromise);

  switch underlyingNested^ {
  | `Fulfilled(value) =>
    resolveInternal(outerPromise, value);
  | `Rejected(error) =>
    rejectInternal(outerPromise, error);

  | `Pending(callbacks) =>
    let underlyingOuter = underlying(outerPromise);
    switch underlyingOuter^ {
    | `Fulfilled(_)
    | `Rejected(_) =>
      /* These two cases are impossible, because if makePromiseBehaveAs is
         called, then_ or catch_ called the callback that was passed to it, so
         the outer promise is still pending. It is this function which resolves
         the outer promise. */
      assert(false);

    | `Pending(outerCallbacks) =>
      MutableList.concatenate(outerCallbacks.onResolve, callbacks.onResolve);
      MutableList.concatenate(outerCallbacks.onReject, callbacks.onReject);
      underlyingNested := `Merged(underlyingOuter);

    | `Merged(_) =>
      /* This case is impossible, because we called underlying above. */
      assert(false);
    };

  | `Merged(_) =>
    /* Impossible because we are working on the underlying promise. */
    assert(false);
  };
};

let then_ = (callback, promise) => {
  let outerPromise = newInternal();

  let onResolve = value =>
    switch (callback(value)) {
    | exception exn =>
      ignore(onUnhandledException^(exn));
    | nestedPromise =>
      makePromiseBehaveAs(outerPromise, nestedPromise);
    };

  switch (underlying(promise))^ {
  | `Fulfilled(value) =>
    ReadyCallbacks.defer(onResolve, value);
  | `Rejected(error) =>
    rejectInternal(outerPromise, error)

  | `Pending(callbacks) =>
    MutableList.append(callbacks.onResolve, onResolve) |> ignore;
    MutableList.append(callbacks.onReject, rejectInternal(outerPromise))
    |> ignore;

  | `Merged(_) =>
    /* This case is impossible, cause of the call to underlying above. */
    assert(false);
  };

  outerPromise;
};

let map = (mapper, promise) =>
  then_(value => resolve(mapper(value)), promise);

let catch = (callback, promise) => {
  let outerPromise = newInternal();

  let onReject = error =>
    switch (callback(error)) {
    | exception exn =>
      ignore(onUnhandledException^(exn));
    | nestedPromise =>
      makePromiseBehaveAs(outerPromise, nestedPromise);
    };

  switch (underlying(promise))^ {
  | `Fulfilled(value) =>
    resolveInternal(outerPromise, value);
  | `Rejected(error) =>
    ReadyCallbacks.defer(onReject, error);

  | `Pending(callbacks) =>
    MutableList.append(callbacks.onResolve, resolveInternal(outerPromise))
    |> ignore;
    MutableList.append(callbacks.onReject, onReject) |> ignore;

  | `Merged(_) =>
    /* This case is impossible, because of the call to underlying above. */
    assert(false);
  };

  outerPromise;
};



/* Repromise.all and Repromise.race have to remove callbacks in some
   circumstances; see test/native/test_ffi.re for details. */
module CallbackRemovers = {
  let empty = () =>
    ref([]);

  let call = removers => {
    removers^ |> List.iter(remover => remover());
    removers := [];
  };

  let add = (removers, promise, whichList, callbackNode) => {
    let remover = () =>
      switch (underlying(promise))^ {
      | `Pending(callbacks) =>
        MutableList.remove(whichList(callbacks), callbackNode);
      | _ =>
        ();
      };

    removers := [remover, ...removers^];
  };
};



let all = promises => {
  let callbackRemovers = CallbackRemovers.empty();

  let finalPromise = newInternal();
  let unresolvedPromiseCount = ref(List.length(promises));
  let results = ref([]);

  let onResolve = (cell, value) => {
    cell := Some(value);
    unresolvedPromiseCount := unresolvedPromiseCount^ - 1;
    if (unresolvedPromiseCount^ == 0) {
      results^
      |> List.map(cell =>
        switch cell^ {
        | None => assert(false)
        | Some(value) => value
        })
      |> resolveInternal(finalPromise);
    };
  };

  let rejectFinalPromise = error => {
    CallbackRemovers.call(callbackRemovers);
    rejectInternal(finalPromise, error);
  };

  results :=
    promises |> List.map(promise => {
      let cell = ref(None);

      switch (underlying(promise))^ {
      | `Fulfilled(value) =>
      /* It's very important to defer here instead of resolving the final
         promise immediately. Doing the latter will cause the callback removal
         mechanism to forget about removing callbacks which will be added later
         in the iteration over the promise list. It is possible to resolve
         immediately but then the code has to be changed, probably to perform
         two passes over the promise list. */
        ReadyCallbacks.defer(onResolve(cell), value);
      | `Rejected(error) =>
        ReadyCallbacks.defer(rejectFinalPromise, error);

      | `Pending(callbacks) =>
        let callbackNode =
          MutableList.append(callbacks.onResolve, onResolve(cell));
        CallbackRemovers.add(
          callbackRemovers,
          promise,
          callbacks => callbacks.onResolve,
          callbackNode);

        let callbackNode =
          MutableList.append(callbacks.onReject, rejectFinalPromise);
        CallbackRemovers.add(
          callbackRemovers,
          promise,
          callbacks => callbacks.onReject,
          callbackNode);

      | `Merged(_) =>
        /* Impossible because of the call to underlying above. */
        assert(false);
      };

      cell;
    });

  finalPromise;
};



let race = promises => {
  let callbackRemovers = CallbackRemovers.empty();

  let finalPromise = newInternal();
  let resolveFinalPromise = value => {
    CallbackRemovers.call(callbackRemovers);
    resolveInternal(finalPromise, value);
  };
  let rejectFinalPromise = error => {
    CallbackRemovers.call(callbackRemovers);
    rejectInternal(finalPromise, error);
  };

  promises |> List.iter(promise =>
    switch (underlying(promise))^ {
    | `Fulfilled(value) =>
      ReadyCallbacks.defer(resolveFinalPromise, value);
    | `Rejected(error) =>
      ReadyCallbacks.defer(rejectFinalPromise, error);

    | `Pending(callbacks) =>
      let callbackNode =
        MutableList.append(callbacks.onResolve, resolveFinalPromise);
      CallbackRemovers.add(
          callbackRemovers,
          promise,
          callbacks => callbacks.onResolve,
          callbackNode);

      let callbackNode =
        MutableList.append(callbacks.onReject, rejectFinalPromise);
      CallbackRemovers.add(
          callbackRemovers,
          promise,
          callbacks => callbacks.onReject,
          callbackNode);

    | `Merged(_) =>
      /* Impossible, because of the call to underlying above. */
      assert false;
    });

  finalPromise;
};
