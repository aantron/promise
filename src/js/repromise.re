type rejectable('a, 'e);
type never;

type promise('a) = rejectable('a, never);
type t('a) = promise('a);



let onUnhandledException = ref(exn => {
  prerr_endline("Unhandled exception in promise callback:");
  prerr_endline(Printexc.to_string(exn));
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

function make(executor) {
    return new Promise(function (resolve, reject) {
        var wrappingResolve = function(value) {
            resolve(wrap(value));
        };
        executor(wrappingResolve, reject);
    });
};

function resolved(value) {
    return Promise.resolve(wrap(value));
};

function then(callback, promise) {
    return promise.then(function (value) {
        try {
            return callback(unwrap(value));
        }
        catch (exception) {
            onUnhandledException[0](exception);
        }
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



module Rejectable = {
  type t('a, 'e) = rejectable('a, 'e);

  external relax: promise('a) => rejectable('a, _) = "%identity";

  [@bs.val]
  external jsNew:
    (('a => unit) => ('e => unit) => unit) => rejectable('a, 'e) = "make";

  let make = () => {
    let resolve = ref(ignore);
    let reject = ref(ignore);
    let p =
      jsNew((resolve', reject') => {
        resolve := resolve';
        reject := reject';
      });
    (p, resolve^, reject^);
  };

  [@bs.val]
  external resolved: 'a => rejectable('a, _) = "";

  [@bs.val]
  external andThen:
    ('a => rejectable('b, 'e), rejectable('a, 'e)) => rejectable('b, 'e) =
      "then";

  let map = (callback, promise) =>
    promise |> andThen(value => resolved(callback(value)));

  let wait = (callback, promise) =>
    promise |> map(callback) |> ignore;

  [@bs.scope "Promise"]
  [@bs.val]
  external rejected: 'e => rejectable(_, 'e) = "reject";

  [@bs.val]
  external catch:
    ('e => rejectable('a, 'e2), rejectable('a, 'e)) => rejectable('a, 'e2) =
      "catch_";

  [@bs.val]
  external unwrap: 'a => 'a = "";

  [@bs.scope "Promise"]
  [@bs.val]
  external jsAll: 'a => 'b = "all";

  let all = promises =>
    promises
    |> Array.of_list
    |> jsAll
    |> map (results =>
      results |> Array.map(unwrap) |> Array.to_list);

  let all2 = (p1, p2) =>
    jsAll((p1, p2));

  let all3 = (p1, p2, p3) =>
    jsAll((p1, p2, p3));

  let all4 = (p1, p2, p3, p4) =>
    jsAll((p1, p2, p3, p4));

  let all5 = (p1, p2, p3, p4, p5) =>
    jsAll((p1, p2, p3, p4, p5));

  let all6 = (p1, p2, p3, p4, p5, p6) =>
    jsAll((p1, p2, p3, p4, p5, p6));

  [@bs.scope "Promise"]
  [@bs.val]
  external jsRace: array(rejectable('a, 'e)) => rejectable('a, 'e) = "race";

  let race = promises =>
    if (promises == []) {
      raise(Invalid_argument("Repromise.race([]) would be pending forever"));
    }
    else {
      jsRace(Array.of_list(promises));
    };

  external fromJsPromise:
    Js.Promise.t('a) => rejectable('a, Js.Promise.error) = "%identity";

  external toJsPromise: rejectable('a, _) => Js.Promise.t('a) = "%identity";
};



let make = () => {
  let (p, resolve, _) = Rejectable.make();
  (p, resolve);
};

let resolved = Rejectable.resolved;
let andThen = Rejectable.andThen;
let map = Rejectable.map;
let wait = Rejectable.wait;
let all = Rejectable.all;
let all2 = Rejectable.all2;
let all3 = Rejectable.all3;
let all4 = Rejectable.all4;
let all5 = Rejectable.all5;
let all6 = Rejectable.all6;
let race = Rejectable.race;
