type promise('a, 'e);
type t('a, 'e) = promise('a, 'e);

type never;

let onUnhandledException = ref(exn => {
  prerr_endline("Unhandled exception in promise callback:");
  prerr_endline(Printexc.to_string(exn));
  exit(2);
});

[%%bs.raw {|
function WrappedRepromise(p) {
    this.wrapped = p;
};

function unwrap(value) {
    if (value instanceof WrappedRepromise)
        return value.wrapped;
    else
        return value;
}

function wrap(value) {
    if (value != null && typeof value.then === 'function')
        return new WrappedRepromise(value);
    else
        return value;
}

function new_(executor) {
    return new Promise(function (resolve, reject) {
        var wrappingResolve = function(value) {
            resolve(wrap(value));
        };
        executor(wrappingResolve, reject);
    });
};

function resolve(value) {
    return Promise.resolve(wrap(value));
};

function then(callback, promise) {
    var safeCallback = function (value) {
        try {
            return callback(value);
        }
        catch (exception) {
            onUnhandledException[0](exception);
        }
    };

    return promise.then(function (value) {
        return safeCallback(unwrap(value));
    });
};

function catch_(callback, promise) {
    var safeCallback = function (error) {
        try {
            return callback(error);
        }
        catch (exception) {
            onUnhandledException[0](exception);
        }
    };

    return promise.catch(safeCallback);
}
|}];

[@bs.val]
external new_: (('a => unit) => ('e => unit) => unit) => promise('a, 'e) = "";

[@bs.val]
external resolve: 'a => promise('a, _) = "";

[@bs.val]
external then_:
  ('a => promise('b, 'e), promise('a, 'e)) => promise('b, 'e) = "then";

[@bs.val]
external map: ('a => 'b, promise('a, 'e)) => promise('b, 'e) = "then";

[@bs.scope "Promise"]
[@bs.val]
external reject: 'e => promise(_, 'e) = "";

[@bs.val]
external catch:
  ('e => promise('a, 'e2), promise('a, 'e)) => promise('a, 'e2) = "catch_";

[@bs.val]
external unwrap: 'a => 'a = "";

[@bs.scope "Promise"]
[@bs.val]
external jsAll: array(promise('a, 'e)) => promise(array('a), 'e) = "all";

let all = promises =>
  promises
  |> Array.of_list
  |> jsAll
  |> map (results =>
    results |> Array.map(unwrap) |> Array.to_list);

[@bs.scope "Promise"]
[@bs.val]
external jsRace: array(promise('a, 'e)) => promise('a, 'e) = "race";

let race = promises =>
  jsRace(Array.of_list(promises));



module ReadyCallbacks = {
  let callbacksPending = () =>
    failwith("unnecessary on JS");

  type snapshot;

  let snapshot = () =>
    failwith("unnecessary on JS");

  let isEmpty = _snapshot =>
    failwith("unnecessary on JS");

  let call = _snapshot =>
    failwith("unnecessary on JS");
};
