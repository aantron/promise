---
id: DesignFAQ
title: Design FAQ
---

## Why are JS promises not type-safe?

Functions like `Promise.resolve` have a [special check][Promise.resolve] for whether the value being put into a promise is another promise or not. It turns out that this check makes `Js.Promise.resolve` impossible to type in the ReasonML type system (and many other type systems).

Here are the details. [`Js.Promise.resolve`][Js.Promise.resolve], the BuckleScript binding to [`Promise.resolve`][Promise.resolve], takes a value, and creates a promise that contains that value:

```reason
Js.Promise.resolve(1) |> Js.log;
/* Promise { 1 } */

Js.Promise.resolve("foo") |> Js.log;
/* Promise { "foo" } */
```

So, we would want to give it the type

```reason
Js.Promise.resolve: 'a => Js.Promise.t('a);
/* resolve takes anything, and wraps it up in a promise. */
```

and, indeed, that is [the type it has in the docs][Js.Promise.resolve].

The type system allows replacing both `'a`s with a specific, single type each time `resolve` is used. So, in the above examples,

```reason
Js.Promise.resolve(1) |> Js.log;
/* resolve: int => Js.Promise.t(int) */

Js.Promise.resolve("foo") |> Js.log;
/* resolve: string => Js.Promise.t(string) */
```

Following this pattern, if we try to **nest** a child promise inside a promise, we would expect to get:

```reason
let child = Js.Promise.resolve(1);

Js.Promise.resolve(child) |> Js.log;
/* Promise { Promise { 1 } } */
/* resolve: Js.Promise.t(int) => Js.Promise.t(Js.Promise.t(int)) */
```

However, the actual output is

```reason
Js.Promise.resolve(child) |> Js.log;
/* Promise { 1 } */
```

This is because `Promise.resolve` [checks][Js.Promise.resolve] for whether its argument is a promise (or any "thenable", `child` in this case). If so, `Promise.resolve` takes the value **inside** `child` out, and puts only that value into the final promise. If not for this special check, `Promise.resolve` would put the **whole** promise `child` into the final promise.

The Reason type system has no way to express this special case, so the type assigned by Reason,

```reason
resolve: Js.Promise.t(int) => Js.Promise.t(Js.Promise.t(int))
```

is incorrect, because the return type says that `child` will be nested.

This is why Repromise provides its own [`Repromise.resolved`](API#resolved), which basically prevents the check from running. The type of [`Repromise.resolved`](API#resolved) is correct even when nesting promises.

Repromise also has to provide [its own versions](API#then) of [`then`][Promise.then] and the other `Promise` functions, because they have to know how to deal with the workaround used by Repromise.

<br/>

## Why does JS have only `then`, but Repromise has `andThen`, `map`, and `wait`?

In JavaScript, [`Promise.then`][Promise.then] behaves differently depending on the (dynamic) type of the value returned by its callback. These different behaviors can't be assigned one type, so `then` is split into `andThen`, `map`, and `wait`.

1. The most general behavior occurs when the callback returns another promise. This allows chaining asynchronous operations:

    ```js
    function inOneSecond(resolve) { setTimeout(resolve, 1000); };

    (new Promise(inOneSecond))
    .then(function() {
        console.log("one second elapsed");
        return new Promise(inOneSecond);
    })
    .then(function() {
        console.log("two seconds elapsed");
    });
    ```

    The first `.then` is the important one. Its callback returns a new promise, and the chain of asynchronous operations continues by waiting for that promise.

    This corresponds to [`Repromise.andThen`](API#andThen), whose callback returns a promise.

2. JS promises support another behavior, which occurs when the callback of `.then` returns a non-promise value. In this case, the value is directly passed to the next `.then` in the chain:

    ```js
    (Promise.resolve(42))
    .then(function(value) {
        return value + 1;
    })
    .then(function(value) {
        console.log(value);
    });
    ```

    Again, the first `.then` is the important one. Since its callback does not return a promise, the chain doesn't have to wait. It continues (almost) immediately into the second `.then`, which prints `43`.

    [`Repromise.andThen`](API#andThen) can't be used for this `.then`, because the callback of [`Repromise.andThen`](API#andThen) must return a promise. Reason's type system won't allow returning a non-promise value.

    So, for this case, Repromise provides [`Repromise.map`](API#map), whose callback can return a non-promise value.

3. A special case of (2) is when the callback is only used to know that a promise has resolved. This kind of callback usually doesn't have an explicit `return` statement (i.e., it returns `undefined`):

    ```
    (new Promise(function(resolve) { setTimeout(resolve, 1000); }))
    .then(function() {
        console.log("one second elapsed");
    });
    ```

    Using [`Repromise.map`](API#map) for this is awkward, because Reason's type system forces us to return a value from the callback, and also to call `ignore` on the final promise that `map` generates.

    To avoid that, Repromise provides [`Repromise.wait`](API#wait).

<br/>

## Why use `|>` instead of `|.`?

Repromise has a native implementation, which needs to compile on OCaml, and OCaml supports only `|>`.

We may upstream `|.` to OCaml, and then change Repromise over to use it.

See [this post](https://github.com/aantron/repromise/issues/22#issuecomment-405589951) and [this post](https://github.com/aantron/repromise/issues/22#issuecomment-405677694) for discussion.

<br/>

## Why is there Repromise, when there are already Lwt and Async?

[Lwt](https://github.com/ocsigen/lwt) and [Async](https://github.com/janestreet/async) are two promise libraries from OCaml's native ecosystem.

Their semantics are pretty different from JS promises, and they don't offer JS interop. This is why Repromise was created.

The semantics of Lwt are relatively close to Repromise, however, so the native implementation of Repromise may eventually have interop with Lwt.

<br/>

## Is there an `async`/`await` syntax for Repromise?

Yes. It is unreleased, but [sitting in the Repromise repo](https://github.com/aantron/repromise/tree/3253553dfc2c20990a8dce057353d4b96f3b2cf0/src/ppx). It looks like this:

```reason
let%await fd   = Io.open_("test/demo/demo.re");
let%await data = Io.read (~fd, ~length = 1024);
print_endline(data);
Repromise.resolved();
```

<br/>



[Js.Promise.resolve]: https://bucklescript.github.io/bucklescript/api/Js.Promise.html#VALresolve
[Promise.resolve]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/resolve
[Promise.then]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/then
