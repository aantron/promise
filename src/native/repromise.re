/* This file is part of Repromise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/repromise/blob/master/LICENSE.md. */



type callbacks('a, 'e) = {
  onResolve: MutableList.list('a => unit),
  onReject: MutableList.list('e => unit),
};



type rejectable('a, 'e) =
  ref([
    | `Fulfilled('a)
    | `Rejected('e)
    | `Pending(callbacks('a, 'e))
    | `Merged(rejectable('a, 'e))
  ]);

type never;

type promise('a) = rejectable('a, never);
type t('a) = promise('a);



/* The `Merged constructor and this function, underlying, are used to avoid a
   memory leak that arises when andThen is called on promises in a loop. See the
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
});



module ReadyCallbacks = {
  let callbacks: ref(MutableList.list(unit => unit)) =
    ref(MutableList.create());

  let callbacksPending = () =>
    !MutableList.isEmpty(callbacks^);

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



let resolved = value =>
  ref(`Fulfilled(value));

let rejected = error =>
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
         called, andThen or catch_ called the callback that was passed to it, so
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

let andThen = (callback, promise) => {
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
  andThen(value => resolved(mapper(value)), promise);

let wait = (callback, promise) =>
  map(callback, promise) |> ignore;

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

let arrayAll = promises =>
  promises
  |> Array.to_list
  |> all
  |> map(Array.of_list)

/* Not a "legitimate" implementation. To get a legitimate one, the tricky parts
   of "all," above, should be factoed out. */
let all2 = (p1, p2) => {
  [Obj.magic(p1), Obj.magic(p2)]
  |> all
  |> map (fun
  | [v1, v2] => (Obj.magic(v1), Obj.magic(v2))
  | _ => assert(false))
};

let all3 = (p1, p2, p3) => {
  [Obj.magic(p1), Obj.magic(p2), Obj.magic(p3)]
  |> all
  |> map (fun
  | [v1, v2, v3] => (Obj.magic(v1), Obj.magic(v2), Obj.magic(v3))
  | _ => assert(false))
};

let all4 = (p1, p2, p3, p4) => {
  [Obj.magic(p1), Obj.magic(p2), Obj.magic(p3), Obj.magic(p4)]
  |> all
  |> map (fun
  | [v1, v2, v3, v4] =>
    (Obj.magic(v1), Obj.magic(v2), Obj.magic(v3), Obj.magic(v4))
  | _ => assert(false))
};

let all5 = (p1, p2, p3, p4, p5) => {
  [Obj.magic(p1), Obj.magic(p2), Obj.magic(p3), Obj.magic(p4), Obj.magic(p5)]
  |> all
  |> map (fun
  | [v1, v2, v3, v4, v5] =>
    (Obj.magic(v1), Obj.magic(v2), Obj.magic(v3), Obj.magic(v4), Obj.magic(v5))
  | _ => assert(false))
};

let all6 = (p1, p2, p3, p4, p5, p6) => {
  [
    Obj.magic(p1),
    Obj.magic(p2),
    Obj.magic(p3),
    Obj.magic(p4),
    Obj.magic(p5),
    Obj.magic(p6)
  ]
  |> all
  |> map (fun
  | [v1, v2, v3, v4, v5, v6] =>
    (
      Obj.magic(v1),
      Obj.magic(v2),
      Obj.magic(v3),
      Obj.magic(v4),
      Obj.magic(v5),
      Obj.magic(v6)
    )
  | _ => assert(false))
};



let race = promises => {
  if (promises == []) {
    raise(Invalid_argument("Repromise.race([]) would be pending forever"));
  };

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



let andThenOk = (callback, promise) =>
  promise |> andThen(fun
    | Ok(value) => callback(value)
    | Error(_) as error => resolved(error));

let andThenError = (callback, promise) =>
  promise |> andThen(fun
    | Ok(_) as ok => resolved(ok)
    | Error(error) => callback(error));

let mapOk = (callback, promise) =>
  promise |> map(fun
    | Ok(value) => Ok(callback(value))
    | Error(_) as error => error);

let mapError = (callback, promise) =>
  promise |> map(fun
    | Ok(_) as ok => ok
    | Error(error) => Error(callback(error)));

let waitOk = (callback, promise) =>
  promise |> wait(fun
    | Ok(value) => callback(value)
    | Error(_) => ());

let waitError = (callback, promise) =>
  promise |> wait(fun
    | Ok(_) => ()
    | Error(error) => callback(error));



let andThenSome = (callback, promise) =>
  promise |> andThen(fun
    | Some(value) => callback(value)
    | None => resolved(None));

let mapSome = (callback, promise) =>
  promise |> map(fun
    | Some(value) => Some(callback(value))
    | None => None);

let waitSome = (callback, promise) =>
  promise |> wait(fun
    | Some(value) => callback(value)
    | None => ());



module Rejectable = {
  type t('a, 'e) = rejectable('a, 'e);

  external relax: promise('a) => rejectable('a, _) = "%identity";

  let make = () => {
    let p = newInternal();
    let resolve = resolveInternal(p);
    let reject = rejectInternal(p);
    (p, resolve, reject);
  };

  let resolved = resolved;
  let rejected = rejected;
  let andThen = andThen;
  let map = map;
  let wait = wait;
  let catch = catch;
  let all = all;
  let race = race;
};



let make = () => {
  let (p, resolve, _) = Rejectable.make();
  (p, resolve);
}
