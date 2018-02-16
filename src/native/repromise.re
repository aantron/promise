type callbacks('a, 'e) = {
  mutable onResolve: list('a => unit),
  mutable onReject: list('e => unit),
};



type promise('a, 'e) =
  ref([
    | `Resolved('a)
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
  | `Resolved(_)
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



let readyCallbacks: ref(list(unit => unit)) =
  ref([]);

let defer = (callbacks, value) => {
  let callbacks = callbacks |> List.map(callback => (() => callback(value)));
  readyCallbacks := readyCallbacks^ @ callbacks;
};



let newInternal = () =>
  ref(`Pending({onResolve: [], onReject: []}));

let resolveInternal = p => value =>
  switch (underlying(p))^ {
  | `Resolved(_)
  | `Rejected(_) => ()
  | `Pending(callbacks) =>
    defer(callbacks.onResolve, value);
    p := `Resolved(value);
  | `Merged(_) =>
    /* This case is impossible, because we called underyling on the promise,
       above. */
    assert(false);
  };

let rejectInternal = p => error =>
  switch (underlying(p))^ {
  | `Resolved(_)
  | `Rejected(_) => ()
  | `Pending(callbacks) =>
    defer(callbacks.onReject, error);
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
  ref(`Resolved(value));

let reject = error =>
  ref(`Rejected(error));



let makePromiseBehaveAs = (outerPromise, nestedPromise) => {
  let underlyingNested = underlying(nestedPromise);

  switch underlyingNested^ {
  | `Resolved(value) =>
    resolveInternal(outerPromise, value);
  | `Rejected(error) =>
    rejectInternal(outerPromise, error);

  | `Pending(callbacks) =>
    let underlyingOuter = underlying(outerPromise);
    switch underlyingOuter^ {
    | `Resolved(_)
    | `Rejected(_) =>
      /* These two cases are impossible, because if makePromiseBehaveAs is
         called, then_ or catch_ called the callback that was passed to it, so
         the outer promise is still pending. It is this function which resolves
         the outer promise. */
      assert(false);

    | `Pending(outerCallbacks) =>
      outerCallbacks.onResolve = outerCallbacks.onResolve @ callbacks.onResolve;
      outerCallbacks.onReject = outerCallbacks.onReject @ callbacks.onReject;
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
  | `Resolved(value) =>
    defer([onResolve], value);
  | `Rejected(error) =>
    rejectInternal(outerPromise, error)

  | `Pending(callbacks) =>
    callbacks.onResolve = callbacks.onResolve @ [onResolve];
    callbacks.onReject = callbacks.onReject @ [rejectInternal(outerPromise)];

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
  | `Resolved(value) =>
    resolveInternal(outerPromise, value);
  | `Rejected(error) =>
    defer([onReject], error);

  | `Pending(callbacks) =>
    callbacks.onResolve = callbacks.onResolve @ [resolveInternal(outerPromise)];
    callbacks.onReject = callbacks.onReject @ [onReject];

  | `Merged(_) =>
    /* This case is impossible, cause of the call to underlying above. */
    assert(false);
  };

  outerPromise;
};
