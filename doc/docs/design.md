---
id: DesignFAQ
title: Design FAQ
---

## Why are JS promises not type-safe?

Consider [`Js.Promise.resolve`][Js.Promise.resolve], the BuckleScript binding to [`Promise.resolve`][Promise.resolve]. It takes a value, and creates a promise that contains that value:

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
let p = Js.Promise.resolve(1);

Js.Promise.resolve(p) |> Js.log;
/* Promise { Promise { 1 } } */
/* resolve: Js.Promise.t(int) => Js.Promise.t(Js.Promise.t(int)) */
```

However, the actual output is

```reason
Js.Promise.resolve(p) |> Js.log;
/* Promise { 1 } */
```

This is because [`Promise.resolve` has a special check][Js.Promise.resolve], for whether its argument is a promise (or any "thenable", `p` in this case). If so, `Promise.resolve` takes the value **inside** `p` out, and puts only that value into the final promise. If not for this special check, `Promise.resolve` would put the **whole** promise `p` into the final promise.

The Reason type system has no way to express this special case (most type systems don't), so the type

```reason
resolve: Js.Promise.t(int) => Js.Promise.t(Js.Promise.t(int))
```

is incorrect, because the return type says that `p` will be nested.

This is why Repromise provides its own [`Repromise.resolve`](API#resolve), which basically prevents the check from running.

Repromise also has to provide [its own versions](API#then) of [`then`][Promise.then] and the other `Promise` functions, because they have to know how to deal with the workaround used by Repromise.

<br/>

## Why does JS have only `then`, but Repromise has `then_`, `map`, and `wait`?

In JavaScript, [`Promise.then`][Promise.then] behaves differently depending on the (dynamic) type of the value returned by its callback. These different behaviors can't be assigned one type, so `then` is split into `then_`, `map`, and `wait`.

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

    This corresponds to [`Repromise.then_`](API#then), whose callback returns a promise.

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

    [`Repromise.then_`](API#then) can't be used for this `.then`, because the callback of [`Repromise.then_`](API#then) must return a promise. Reason's type system won't allow returning a non-promise value.

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

<br/>

## Why do `new_` and `then_` have underscores?

The regular `new` is a keyword in Reason.

`then` is not a keyword in Reason, but Reason is supposed to still parse when transformed to OCaml, and `then` is a keyword in OCaml.

<br/>



[Js.Promise.resolve]: https://bucklescript.github.io/bucklescript/api/Js.Promise.html#VALresolve
[Promise.resolve]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/resolve
[Promise.then]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/then
