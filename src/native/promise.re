/* This file is part of reason-promise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/promise/blob/master/LICENSE.md. */

type callbacks('a, 'e) = {
  onResolve: MutableList.list('a => unit),
  onReject: MutableList.list('e => unit),
};

type rejectable('a, 'e) =
  ref(
    [
      | `Fulfilled('a)
      | `Rejected('e)
      | `Pending(callbacks('a, 'e))
      | `Merged(rejectable('a, 'e))
    ],
  );

type never;

type promise('a) = rejectable('a, never);
type t('a) = promise('a);

/* The `Merged constructor and this function, underlying, are used to avoid a
   memory leak that arises when flatMap is called on promises in a loop. See the
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
  switch (p^) {
  | `Fulfilled(_)
  | `Rejected(_)
  | `Pending(_) => p

  | `Merged(p') =>
    let p'' = underlying(p');
    if (p'' !== p') {
      p := `Merged(p'');
    };
    p'';
  };

let onUnhandledException =
  ref(exn => {
    prerr_endline("Unhandled exception in promise callback:");
    prerr_endline(Printexc.to_string(exn));
    Printexc.print_backtrace(stderr);
  });

module ReadyCallbacks = {
  let callbacks: ref(MutableList.list(unit => unit)) =
    ref(MutableList.create());

  let callbacksPending = () => !MutableList.isEmpty(callbacks^);

  let defer = (callback, value) =>
    MutableList.append(callbacks^, () => callback(value)) |> ignore;

  let deferMultiple = (newCallbacks, value) =>
    newCallbacks |> MutableList.iter(callback => defer(callback, value));

  type snapshot = MutableList.list(unit => unit);

  let snapshot = () => {
    let theSnapshot = callbacks^;
    callbacks := MutableList.create();
    theSnapshot;
  };

  let isEmpty = snapshot => MutableList.isEmpty(snapshot);

  let call = snapshot => snapshot |> MutableList.iter(callback => callback());
};

let newInternal = () =>
  ref(
    `Pending({
      onResolve: MutableList.create(),
      onReject: MutableList.create(),
    }),
  );

let resolveInternal = (p, value) =>
  switch ((underlying(p))^) {
  | `Fulfilled(_)
  | `Rejected(_) => ()
  | `Pending(callbacks) =>
    ReadyCallbacks.deferMultiple(callbacks.onResolve, value);
    p := `Fulfilled(value);
  | `Merged(_) =>
    /* This case is impossible, because we called underyling on the promise,
       above. */
    assert(false)
  };

let rejectInternal = (p, error) =>
  switch ((underlying(p))^) {
  | `Fulfilled(_)
  | `Rejected(_) => ()
  | `Pending(callbacks) =>
    ReadyCallbacks.deferMultiple(callbacks.onReject, error);
    p := `Rejected(error);
  | `Merged(_) =>
    /* This case is impossible, because we called underyling on the promise,
       above. */
    assert(false)
  };

let resolved = value => ref(`Fulfilled(value));

let rejected = error => ref(`Rejected(error));

let makePromiseBehaveAs = (outerPromise, nestedPromise) => {
  let underlyingNested = underlying(nestedPromise);

  switch (underlyingNested^) {
  | `Fulfilled(value) => resolveInternal(outerPromise, value)
  | `Rejected(error) => rejectInternal(outerPromise, error)

  | `Pending(callbacks) =>
    let underlyingOuter = underlying(outerPromise);
    switch (underlyingOuter^) {
    | `Fulfilled(_)
    | `Rejected(_) =>
      /* These two cases are impossible, because if makePromiseBehaveAs is
         called, flatMap or catch_ called the callback that was passed to it, so
         the outer promise is still pending. It is this function which resolves
         the outer promise. */
      assert(false)

    | `Pending(outerCallbacks) =>
      MutableList.concatenate(outerCallbacks.onResolve, callbacks.onResolve);
      MutableList.concatenate(outerCallbacks.onReject, callbacks.onReject);
      underlyingNested := `Merged(underlyingOuter);

    | `Merged(_) =>
      /* This case is impossible, because we called underlying above. */
      assert(false)
    };

  | `Merged(_) =>
    /* Impossible because we are working on the underlying promise. */
    assert(false)
  };
};

let flatMap = (promise, callback) => {
  let outerPromise = newInternal();

  let onResolve = value =>
    switch (callback(value)) {
    | exception exn => ignore(onUnhandledException^(exn))
    | nestedPromise => makePromiseBehaveAs(outerPromise, nestedPromise)
    };

  switch ((underlying(promise))^) {
  | `Fulfilled(value) => ReadyCallbacks.defer(onResolve, value)
  | `Rejected(error) => rejectInternal(outerPromise, error)

  | `Pending(callbacks) =>
    MutableList.append(callbacks.onResolve, onResolve) |> ignore;
    MutableList.append(callbacks.onReject, rejectInternal(outerPromise))
    |> ignore;

  | `Merged(_) =>
    /* This case is impossible, cause of the call to underlying above. */
    assert(false)
  };

  outerPromise;
};

let map = (promise, mapper) =>
  flatMap(promise, value => resolved(mapper(value)));

let get = (promise, callback) => ignore(map(promise, callback));

let tap = (promise, callback) => {
  get(promise, callback);
  promise;
};

let catch = (promise, callback) => {
  let outerPromise = newInternal();

  let onReject = error =>
    switch (callback(error)) {
    | exception exn => ignore(onUnhandledException^(exn))
    | nestedPromise => makePromiseBehaveAs(outerPromise, nestedPromise)
    };

  switch ((underlying(promise))^) {
  | `Fulfilled(value) => resolveInternal(outerPromise, value)
  | `Rejected(error) => ReadyCallbacks.defer(onReject, error)

  | `Pending(callbacks) =>
    MutableList.append(callbacks.onResolve, resolveInternal(outerPromise))
    |> ignore;
    MutableList.append(callbacks.onReject, onReject) |> ignore;

  | `Merged(_) =>
    /* This case is impossible, because of the call to underlying above. */
    assert(false)
  };

  outerPromise;
};

/* Promise.all and Promise.race have to remove callbacks in some circumstances;
   see test/native/test_ffi.re for details. */
module CallbackRemovers = {
  let empty = () => ref([]);

  let call = removers => {
    removers^ |> List.iter(remover => remover());
    removers := [];
  };

  let add = (removers, promise, whichList, callbackNode) => {
    let remover = () =>
      switch ((underlying(promise))^) {
      | `Pending(callbacks) =>
        MutableList.remove(whichList(callbacks), callbackNode)
      | _ => ()
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
           switch (cell^) {
           | None => assert(false)
           | Some(value) => value
           }
         )
      |> resolveInternal(finalPromise);
    };
  };

  let rejectFinalPromise = error => {
    CallbackRemovers.call(callbackRemovers);
    rejectInternal(finalPromise, error);
  };

  results :=
    promises
    |> List.map(promise => {
         let cell = ref(None);

         switch ((underlying(promise))^) {
         | `Fulfilled(value) =>
           /* It's very important to defer here instead of resolving the final
              promise immediately. Doing the latter will cause the callback removal
              mechanism to forget about removing callbacks which will be added later
              in the iteration over the promise list. It is possible to resolve
              immediately but then the code has to be changed, probably to perform
              two passes over the promise list. */
           ReadyCallbacks.defer(onResolve(cell), value)
         | `Rejected(error) =>
           ReadyCallbacks.defer(rejectFinalPromise, error)

         | `Pending(callbacks) =>
           let callbackNode =
             MutableList.append(callbacks.onResolve, onResolve(cell));
           CallbackRemovers.add(
             callbackRemovers,
             promise,
             callbacks => callbacks.onResolve,
             callbackNode,
           );

           let callbackNode =
             MutableList.append(callbacks.onReject, rejectFinalPromise);
           CallbackRemovers.add(
             callbackRemovers,
             promise,
             callbacks => callbacks.onReject,
             callbackNode,
           );

         | `Merged(_) =>
           /* Impossible because of the call to underlying above. */
           assert(false)
         };

         cell;
       });

  finalPromise;
};

let allArray = promises => map(all(Array.to_list(promises)), Array.of_list);

/* Not a "legitimate" implementation. To get a legitimate one, the tricky parts
   of "all," above, should be factoed out. */
let all2 = (p1, p2) => {
  let promises = [Obj.magic(p1), Obj.magic(p2)];
  map(
    all(promises),
    fun
    | [v1, v2] => (Obj.magic(v1), Obj.magic(v2))
    | _ => assert(false),
  );
};

let all3 = (p1, p2, p3) => {
  let promises = [Obj.magic(p1), Obj.magic(p2), Obj.magic(p3)];
  map(
    all(promises),
    fun
    | [v1, v2, v3] => (Obj.magic(v1), Obj.magic(v2), Obj.magic(v3))
    | _ => assert(false),
  );
};

let all4 = (p1, p2, p3, p4) => {
  let promises = [
    Obj.magic(p1),
    Obj.magic(p2),
    Obj.magic(p3),
    Obj.magic(p4),
  ];
  map(
    all(promises),
    fun
    | [v1, v2, v3, v4] => (
        Obj.magic(v1),
        Obj.magic(v2),
        Obj.magic(v3),
        Obj.magic(v4),
      )
    | _ => assert(false),
  );
};

let all5 = (p1, p2, p3, p4, p5) => {
  let promises = [
    Obj.magic(p1),
    Obj.magic(p2),
    Obj.magic(p3),
    Obj.magic(p4),
    Obj.magic(p5),
  ];
  map(
    all(promises),
    fun
    | [v1, v2, v3, v4, v5] => (
        Obj.magic(v1),
        Obj.magic(v2),
        Obj.magic(v3),
        Obj.magic(v4),
        Obj.magic(v5),
      )
    | _ => assert(false),
  );
};

let all6 = (p1, p2, p3, p4, p5, p6) => {
  let promises = [
    Obj.magic(p1),
    Obj.magic(p2),
    Obj.magic(p3),
    Obj.magic(p4),
    Obj.magic(p5),
    Obj.magic(p6),
  ];
  map(
    all(promises),
    fun
    | [v1, v2, v3, v4, v5, v6] => (
        Obj.magic(v1),
        Obj.magic(v2),
        Obj.magic(v3),
        Obj.magic(v4),
        Obj.magic(v5),
        Obj.magic(v6),
      )
    | _ => assert(false),
  );
};

let race = promises => {
  if (promises == []) {
    raise(Invalid_argument("Promise.race([]) would be pending forever"));
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

  promises
  |> List.iter(promise =>
       switch ((underlying(promise))^) {
       | `Fulfilled(value) =>
         ReadyCallbacks.defer(resolveFinalPromise, value)
       | `Rejected(error) => ReadyCallbacks.defer(rejectFinalPromise, error)

       | `Pending(callbacks) =>
         let callbackNode =
           MutableList.append(callbacks.onResolve, resolveFinalPromise);
         CallbackRemovers.add(
           callbackRemovers,
           promise,
           callbacks => callbacks.onResolve,
           callbackNode,
         );

         let callbackNode =
           MutableList.append(callbacks.onReject, rejectFinalPromise);
         CallbackRemovers.add(
           callbackRemovers,
           promise,
           callbacks => callbacks.onReject,
           callbackNode,
         );

       | `Merged(_) =>
         /* Impossible, because of the call to underlying above. */
         assert(false)
       }
     );

  finalPromise;
};

type result('a, 'e) = Result.result('a, 'e);

open Result;

let flatMapOk = (promise, callback) =>
  flatMap(
    promise,
    fun
    | Ok(value) => callback(value)
    | Error(_) as error => resolved(error),
  );

let flatMapError = (promise, callback) =>
  flatMap(
    promise,
    fun
    | Ok(_) as ok => resolved(ok)
    | Error(error) => callback(error),
  );

let mapOk = (promise, callback) =>
  map(
    promise,
    fun
    | Ok(value) => Ok(callback(value))
    | Error(_) as error => error,
  );

let mapError = (promise, callback) =>
  map(
    promise,
    fun
    | Ok(_) as ok => ok
    | Error(error) => Error(callback(error)),
  );

let getOk = (promise, callback) =>
  get(
    promise,
    fun
    | Ok(value) => callback(value)
    | Error(_) => (),
  );

let getError = (promise, callback) =>
  get(
    promise,
    fun
    | Ok(_) => ()
    | Error(error) => callback(error),
  );

let tapOk = (promise, callback) => {
  getOk(promise, callback);
  promise;
};

let tapError = (promise, callback) => {
  getError(promise, callback);
  promise;
};

module Operators = {
  let (>|=) = mapOk;
  let (>>=) = flatMapOk;
};

let flatMapSome = (promise, callback) =>
  flatMap(
    promise,
    fun
    | Some(value) => callback(value)
    | None => resolved(None),
  );

let mapSome = (promise, callback) =>
  map(
    promise,
    fun
    | Some(value) => Some(callback(value))
    | None => None,
  );

let getSome = (promise, callback) =>
  get(
    promise,
    fun
    | Some(value) => callback(value)
    | None => (),
  );

let tapSome = (promise, callback) => {
  getSome(promise, callback);
  promise;
};

module Js = {
  type t('a, 'e) = rejectable('a, 'e);

  external relax: promise('a) => rejectable('a, _) = "%identity";

  let pending = () => {
    let p = newInternal();
    let resolve = resolveInternal(p);
    let reject = rejectInternal(p);
    (p, resolve, reject);
  };

  let resolved = resolved;
  let rejected = rejected;
  let flatMap = flatMap;
  let map = map;
  let get = get;
  let tap = tap;
  let catch = catch;
  let all = all;
  let race = race;

  let toResult = promise =>
    catch(map(promise, v => Ok(v)), e => resolved(Error(e)));

  let fromResult = promise =>
    flatMap(
      relax(promise),
      fun
      | Ok(v) => resolved(v)
      | Error(e) => rejected(e),
    );
};

let pending = () => {
  let (p, resolve, _) = Js.pending();
  (p, resolve);
};

let exec = executor => {
  let (p, resolve) = pending();
  executor(resolve);
  p;
};

module FastPipe = {
  let (|.) = (v, f) => f(v);
};
