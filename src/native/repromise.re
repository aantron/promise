type callbacks('a, 'e) = {
  mutable onResolve: list('a => unit),
  mutable onReject: list('e => unit),
};

type promise('a, 'e) =
  ref([
    | `Resolved('a)
    | `Rejected('e)
    | `Pending(callbacks('a, 'e))
  ]);

type t('a, 'e) = promise('a, 'e);

type never;

let readyCallbacks: ref(list(unit => unit)) =
  ref([]);

let defer = (callbacks, value) => {
  let callbacks = callbacks |> List.map(callback => (() => callback(value)));
  readyCallbacks := readyCallbacks^ @ callbacks;
};

let new_ = executor => {
  let promise = ref(`Pending({onResolve: [], onReject: []}));

  let resolve = (value) =>
    switch promise^ {
    | `Resolved(_)
    | `Rejected(_) => ()
    | `Pending(callbacks) =>
      defer(callbacks.onResolve, value);
      promise := `Resolved(value);
    };

  let reject = (error) =>
    switch promise^ {
    | `Resolved(_)
    | `Rejected(_) => ()
    | `Pending(callbacks) =>
      defer(callbacks.onReject, error);
      promise := `Rejected(error);
    };

  executor(resolve, reject);

  promise;
};

let resolve = value =>
  ref(`Resolved(value));

let reject = error =>
  ref(`Rejected(error));

let makePromiseBehaveAs = (resolveOuter, rejectOuter, nestedPromise) =>
  switch nestedPromise^ {
  | `Resolved(value) =>
    resolveOuter(value);
  | `Rejected(error) =>
    rejectOuter(error);
  | `Pending(callbacks) =>
    callbacks.onResolve = callbacks.onResolve @ [resolveOuter];
    callbacks.onReject = callbacks.onReject @ [rejectOuter];
  };

let then_ = (callback, promise) =>
  new_((resolveOuter, rejectOuter) => {
    let onResolve = (value) => {
      let nestedPromise = callback(value);
      makePromiseBehaveAs(resolveOuter, rejectOuter, nestedPromise);
    };

    switch promise^ {
    | `Resolved(value) =>
      defer([onResolve], value);
    | `Rejected(error) =>
      rejectOuter(error);
    | `Pending(callbacks) =>
      callbacks.onResolve = callbacks.onResolve @ [onResolve];
      callbacks.onReject = callbacks.onReject @ [rejectOuter];
    };
  });

let catch = (callback, promise) =>
  new_((resolveOuter, rejectOuter) => {
    let onReject = (error) => {
      let nestedPromise = callback(error);
      makePromiseBehaveAs(resolveOuter, rejectOuter, nestedPromise);
    };

    switch promise^ {
    | `Resolved(value) =>
      resolveOuter(value);
    | `Rejected(error) =>
      defer([onReject], error);
    | `Pending(callbacks) =>
      callbacks.onResolve = callbacks.onResolve @ [resolveOuter];
      callbacks.onReject = callbacks.onReject @ [onReject];
    };
  });
