/* This file is part of reason-promise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/promise/blob/master/LICENSE.md. */



type rejectable(+'a, +'e);
type never;

type promise(+'a) = rejectable('a, never);
type t(+'a) = promise('a);



let onUnhandledException = ref(exn => {
  prerr_endline("Unhandled exception in promise callback:");
  Js.Console.error(exn);
});



[%%bs.raw {|
function PromiseBox(p) {
    this.nested = p;
};

function unbox(value) {
    if (value instanceof PromiseBox)
        return value.nested;
    else
        return value;
}

function box(value) {
    if (value != null && typeof value.then === 'function')
        return new PromiseBox(value);
    else
        return value;
}

function make(executor) {
    return new Promise(function (resolve, reject) {
        var boxingResolve = function(value) {
            resolve(box(value));
        };
        executor(boxingResolve, reject);
    });
};

function resolved(value) {
    return Promise.resolve(box(value));
};

function then(promise, callback) {
    return promise.then(function (value) {
        try {
            return callback(unbox(value));
        }
        catch (exception) {
            onUnhandledException.contents(exception);
        }
    });
};

function catch_(promise, callback) {
    var safeCallback = function (error) {
        try {
            return callback(error);
        }
        catch (exception) {
            onUnhandledException.contents(exception);
        }
    };

    return promise.catch(safeCallback);
};
|}];



/* Compatibility with BukleScript < 6. */
type result('a, 'e) = Belt.Result.t('a, 'e) = Ok('a) | Error('e);



module Js_ = {
  type t('a, 'e) = rejectable('a, 'e);

  external relax: promise('a) => rejectable('a, _) = "%identity";

  [@bs.val]
  external jsNew:
    (('a => unit) => ('e => unit) => unit) => rejectable('a, 'e) = "make";

  let pending = () => {
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
  external resolved: 'a => rejectable('a, _) = "resolved";

  [@bs.val]
  external flatMap:
    (rejectable('a, 'e), 'a => rejectable('b, 'e)) => rejectable('b, 'e) =
      "then";

  let map = (promise, callback) =>
    flatMap(promise, v => resolved(callback(v)));

  let get = (promise, callback) =>
    ignore(map(promise, callback));

  let tap = (promise, callback) => {
    get(promise, callback);
    promise;
  };

  [@bs.scope "Promise"]
  [@bs.val]
  external rejected: 'e => rejectable(_, 'e) = "reject";

  [@bs.val]
  external catch:
    (rejectable('a, 'e), 'e => rejectable('a, 'e2)) => rejectable('a, 'e2) =
      "catch_";

  [@bs.val]
  external unbox: 'a => 'a = "unbox";

  [@bs.scope "Promise"]
  [@bs.val]
  external jsAll: 'a => 'b = "all";

  let allArray = promises =>
    map(jsAll(promises), promises => Belt.Array.map(promises, unbox));

  let all = promises =>
    map(allArray(Belt.List.toArray(promises)), Belt.List.fromArray);

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
      raise(Invalid_argument("Promise.race([]) would be pending forever"));
    }
    else {
      jsRace(Belt.List.toArray(promises));
    };

  let toResult = promise =>
    catch(map(promise, v => Ok(v)), e => resolved(Error(e)));

  let fromResult = promise =>
    flatMap(relax(promise), fun
      | Ok(v) => resolved(v)
      | Error(e) => rejected(e));

  external fromBsPromise:
    Js.Promise.t('a) => rejectable('a, Js.Promise.error) = "%identity";

  external toBsPromise:
    rejectable('a, _) => Js.Promise.t('a) = "%identity";
};



let pending = () => {
  let (p, resolve, _) = Js_.pending();
  (p, resolve);
};

let exec = executor => {
  let (p, resolve) = pending();
  executor(resolve);
  p;
};

let resolved = Js_.resolved;
let flatMap = Js_.flatMap;
let map = Js_.map;
let get = Js_.get;
let tap = Js_.tap;
let all = Js_.all;
let all2 = Js_.all2;
let all3 = Js_.all3;
let all4 = Js_.all4;
let all5 = Js_.all5;
let all6 = Js_.all6;
let allArray = Js_.allArray;
let race = Js_.race;



let flatMapOk = (promise, callback) =>
  flatMap(promise, result =>
    switch (result) {
    | Ok(v) => callback(v)
    | Error(_) as error => resolved(error)
    });

let flatMapError = (promise, callback) =>
  flatMap(promise, result =>
    switch (result) {
    | Ok(_) as ok => resolved(ok)
    | Error(e) => callback(e)
    });

let mapOk = (promise, callback) =>
  map(promise, result =>
    switch (result) {
    | Ok(v) => Ok(callback(v))
    | Error(_) as error => error
    });

let mapError = (promise, callback) =>
  map(promise, result =>
    switch (result) {
    | Ok(_) as ok => ok
    | Error(e) => Error(callback(e))
    });

let getOk = (promise, callback) =>
  get(promise, result =>
    switch (result) {
    | Ok(v) => callback(v)
    | Error(_) => ()
    });

let getError = (promise, callback) =>
  get(promise, result =>
    switch (result) {
    | Ok(_) => ()
    | Error(e) => callback(e)
    });

let tapOk = (promise, callback) => {
  getOk(promise, callback);
  promise;
};

let tapError = (promise, callback) => {
  getError(promise, callback);
  promise;
};

let allOkArray = promises => {
  let promiseCount = Array.length(promises);

  let resultValues = Array.make(promiseCount, None);
  let resultCount = ref(0);
  let (resultPromise, resolve) = pending();

  let (callbackRemover, removeCallbacks) = pending();

  promises |> Array.iteri((index, promise) =>
    /* Because callbacks are added to the user's promises through calls to the
       JS runtime's Promise.race, this function leaks memory if and only if the
       JS runtime's Promise functions leak memory. In particular, if one of the
       promises resolves with Error(_), the callbacks on the other promises
       should be removed. If not done, and long-pending promises are repeatedly
       passed to allOk in a loop, they will gradually accumulate huge lists of
       stale callbacks. This is also true of Promise.race, so we rely on the
       quality of the runtime's Promise.race implementation to proactively
       remove these callbacks. */
    race([promise, callbackRemover])
    |> wrapped => get(wrapped, result =>
      switch (result) {
      | Ok(v) =>
        resultValues[index] = Some(v);
        incr(resultCount);
        if (resultCount^ >= promiseCount) {
          resultValues
          |> Array.map(v =>
            switch (v) {
            | Some(v) => v
            | None => assert(false)
            })
          |> values => resolve(Ok(values))
        };
      | Error(e) =>
        resolve(Error(e));
        removeCallbacks(Error(e));
      }));

  resultPromise
};

let allOk = promises =>
  mapOk(allOkArray(Array.of_list(promises)), Array.to_list);

let allOk2 = (p1, p2) => {
  let promises = [|Obj.magic(p1), Obj.magic(p2)|];
  mapOk(allOkArray(promises), fun
  | [|v1, v2|] => (Obj.magic(v1), Obj.magic(v2))
  | _ => assert(false))
};

let allOk3 = (p1, p2, p3) => {
  let promises = [|Obj.magic(p1), Obj.magic(p2), Obj.magic(p3)|];
  mapOk(allOkArray(promises), fun
  | [|v1, v2, v3|] => (Obj.magic(v1), Obj.magic(v2), Obj.magic(v3))
  | _ => assert(false))
};

let allOk4 = (p1, p2, p3, p4) => {
  let promises = [|Obj.magic(p1), Obj.magic(p2), Obj.magic(p3), Obj.magic(p4)|];
  mapOk(allOkArray(promises), fun
  | [|v1, v2, v3, v4|] =>
    (Obj.magic(v1), Obj.magic(v2), Obj.magic(v3), Obj.magic(v4))
  | _ =>
    assert(false))
};

let allOk5 = (p1, p2, p3, p4, p5) => {
  let promises = [|
    Obj.magic(p1),
    Obj.magic(p2),
    Obj.magic(p3),
    Obj.magic(p4),
    Obj.magic(p5)
  |];
  mapOk(allOkArray(promises), fun
  | [|v1, v2, v3, v4, v5|] =>
    (Obj.magic(v1), Obj.magic(v2), Obj.magic(v3), Obj.magic(v4), Obj.magic(v5))
  | _ =>
    assert(false))
};

let allOk6 = (p1, p2, p3, p4, p5, p6) => {
  let promises = [|
    Obj.magic(p1),
    Obj.magic(p2),
    Obj.magic(p3),
    Obj.magic(p4),
    Obj.magic(p5),
    Obj.magic(p6)
  |];
  mapOk(allOkArray(promises), fun
  | [|v1, v2, v3, v4, v5, v6|] =>
    (
      Obj.magic(v1),
      Obj.magic(v2),
      Obj.magic(v3),
      Obj.magic(v4),
      Obj.magic(v5),
      Obj.magic(v6)
    )
  | _ =>
    assert(false))
};

module Operators = {
  let (>|=) = mapOk;
  let (>>=) = flatMapOk;
};



let flatMapSome = (promise, callback) =>
  flatMap(promise, option =>
    switch (option) {
    | Some(v) => callback(v)
    | None => resolved(None)
    });

let mapSome = (promise, callback) =>
  map(promise, option =>
    switch (option) {
    | Some(v) => Some(callback(v))
    | None => None
    });

let getSome = (promise, callback) =>
  get(promise, option =>
    switch (option) {
    | Some(v) => callback(v)
    | None => ()
    });

let tapSome = (promise, callback) => {
  getSome(promise, callback);
  promise;
};



module PipeFirst = {
};

module Js = Js_;
