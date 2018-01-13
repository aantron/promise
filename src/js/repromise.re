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

function new_(executor) {
    return new Promise(function (resolve, reject) {
        var wrappingResolve = function(value) {
            if (value && value.then && (typeof (value.then) === 'function'))
                resolve(new WrappedRepromise(value));
            else
                resolve(value);
        };
        executor(wrappingResolve, reject);
    });
};

function resolve(value) {
    if (value && value.then && (typeof (value.then) === 'function'))
        return Promise.resolve(new WrappedRepromise(value));
    else
        return Promise.resolve(value);
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
        if (value instanceof WrappedRepromise)
            return safeCallback(value.wrapped);
        else
            return safeCallback(value);
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

/* To what will this... resolve? */
[@bs.val]
external resolve: 'a => promise('a, _) = "";

[@bs.val]
external then_:
  ('a => promise('b, 'e), promise('a, 'e)) => promise('b, 'e) = "then";

[@bs.scope "Promise"]
[@bs.val]
external reject: 'e => promise(_, 'e) = "";

[@bs.val]
external catch:
  ('e => promise('a, 'e2), promise('a, 'e)) => promise('a, 'e2) = "catch_";

let readyCallbacks: ref(list(unit => unit)) = ref([]);
