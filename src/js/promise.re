/* This file is part of reason-promise, released under the MIT license. See
   LICENSE.md for details, or visit
   https://github.com/aantron/promise/blob/master/LICENSE.md. */



type rejectable('a, 'e);
type never;

type promise('a) = rejectable('a, never);
type t('a) = promise('a);



let onUnhandledException = ref(exn => {
  prerr_endline("Unhandled exception in promise callback:");
  Js.Console.error(exn);
});



[%%bs.raw {|
function WrappedPromise(p) {
    this.wrapped = p;
};

function unwrap(value) {
    if (value instanceof WrappedPromise)
        return value.wrapped;
    else
        return value;
}

function wrap(value) {
    if (value != null && typeof value.then === 'function')
        return new WrappedPromise(value);
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

function then(promise, callback) {
    return promise.then(function (value) {
        try {
            return callback(unwrap(value));
        }
        catch (exception) {
            onUnhandledException[0](exception);
        }
    });
};

function catch_(promise, callback) {
    var safeCallback = function (error) {
        try {
            return callback(error);
        }
        catch (exception) {
            onUnhandledException[0](exception);
        }
    };

    return promise.catch(safeCallback);
};

function arrayToList(array) {
  var list = 0;
  for (var index = array.length - 1; index >= 0; --index) {
    list = [array[index], list];
  }
  return list;
};

function listToArray(list) {
  var array = [];
  while (list !== 0) {
    array.push(list[0]);
    list = list[1];
  }
  return array;
};

function mapArray(f, a) {
  return a.map(f);
};
|}];

[@bs.val]
external arrayToList: array('a) => list('a) = "arrayToList";

[@bs.val]
external listToArray: list('a) => array('a) = "listToArray";

[@bs.val]
external mapArray: ('a => 'b, array('a)) => array('b) = "mapArray";



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
  external unwrap: 'a => 'a = "unwrap";

  [@bs.scope "Promise"]
  [@bs.val]
  external jsAll: 'a => 'b = "all";

  let arrayAll = promises =>
    map(jsAll(promises), mapArray(unwrap));

  let all = promises =>
    map(arrayAll(listToArray(promises)), results => arrayToList(results));

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
      jsRace(listToArray(promises));
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
let arrayAll = Js_.arrayAll;
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



module FastPipe = {
};

module Js = Js_;
