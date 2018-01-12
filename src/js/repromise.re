type promise('a, 'e);
type t('a, 'e) = promise('a, 'e);

type never;

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
    return promise.then(function (value) {
        if (value instanceof WrappedRepromise)
            return callback(value.wrapped);
        else
            return callback(value);
    })
};
|}];

[@bs.val]
external new_: (('a => unit) => ('e => unit) => unit) => promise('a, 'e) = "";

/* To what will this... resolve? */
[@bs.val]
external resolve: 'a => promise('a, _) = "";

[@bs.val]
external then_:
  ('a => promise('b, 'e), promise('a, _)) => promise('b, 'e) = "then";

[@bs.scope "Promise"]
[@bs.val]
external reject: 'e => promise(_, 'e) = "";

[@bs.send.pipe: promise('a, 'e)]
external catch: ('e => promise('a, 'e2)) => promise('a, 'e2) = "catch";

let readyCallbacks: ref(list(unit => unit)) = ref([]);
