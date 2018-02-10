type promise('a);
type t('a) = promise('a);

[%%bs.raw
  {|
function WrappedRepromise(p) {
    this.wrapped = p;
};

function new_(executor) {
    return new Promise(function (resolve, reject) {
        var wrappingResolve = function(value) {
            if (value && value.then && (typeof (value.then) == 'function'))
                resolve(new WrappedRepromise(value));
            else
                resolve(value);
        };
        executor(wrappingResolve);
    });
};

function resolve(value) {
    if (value && value.then && (typeof (value.then) == 'function'))
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
|}
];

[@bs.val]
external new_: ((~resolve: 'a => unit) => unit) => promise('a) = "";

/* To what will this... resolve? */
[@bs.val]
external resolve: 'a => promise('a) = "";

[@bs.val]
external then_: ('a => promise('b), promise('a)) => promise('b) = "then";

let ready_callbacks: ref(list(unit => unit)) = ref([]);
