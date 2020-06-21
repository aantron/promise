# Promise &nbsp;&nbsp;&nbsp; [![Version][version-img]][version] [![Travis status][travis-img]][travis] [![Coverage][coveralls-img]][coveralls]

[version-img]: https://img.shields.io/badge/version-1.1.0-blue.svg
[version]: https://github.com/aantron/promise/releases
[travis]: https://travis-ci.org/aantron/promise/branches
[travis-img]: https://img.shields.io/travis/aantron/promise/master.svg?label=travis
[coveralls]: https://coveralls.io/github/aantron/promise?branch=master
[coveralls-img]: https://img.shields.io/coveralls/aantron/promise/master.svg

A lightweight, type-safe binding to JS promises:

```reason
Js.log(Promise.resolved("Hello"));  /* Promise { 'Hello' } */

Promise.resolved("Hello")
->Promise.map(s => s ++ " world!")
->Promise.get(s => Js.log(s));      /* Hello world! */
```

As you can see on the first line, `Promise.t` maps directly to familiar JS
promises from your JS runtime. That means...

- You can use `reason-promise` directly to [write JS bindings](#Bindings).
- All JS tooling for promises immediately works with `reason-promise`.
- Even if you do something exotic, like switch out the promise implementation at
  the JS level, for, say, better stack traces, `reason-promise` still binds to
  it!

<br/>

There is only one exception to the rule that `Promise.t` maps directly to JS
promises: when there is a promise nested inside another promise. JS [breaks the
type safety](#JSPromiseFlattening) of promises in a misguided attempt to
disallow nesting. [`reason-promise` instead emulates it in a way that makes
promises type-safe again](#TypeSafety). This is in contrast to BuckleScript's
built-in `Js.Promise`, which directly exposes the JS behavior, and so is not
type-safe.

<br/>

In addition:

- `reason-promise` offers a clean functional API, which replaces rejection with
  [helpers for `Result` and `Option`](#Errors).
- `reason-promise` is tiny. It weighs in at about [1K bundled][bundle-size].
- `reason-promise` also has a full, standalone [pure-Reason
  implementation][native], which passes all the same tests. It can be used for
  native code or in JS.

[bundle-size]: https://travis-ci.org/github/aantron/promise/jobs/700562910#L210
[native]: https://github.com/aantron/promise/tree/master/src/native

<br>

## Tutorial

- [Installing](#Installing)
- [Getting started](#GettingStarted)
- [Creating new promises](#Creating)
- [Getting values from promises](#Values)
- [Transforming promises](#Transforming)
- [Tracing](#Tracing)
- [Concurrent combinations](#Combining)
- [Handling errors with `Result`](#Errors)
- [Advanced: Rejection](#Rejection)
- [Advanced: Bindings](#Bindings)
- [Discussion: Why JS promises are unsafe](#JSPromiseFlattening)
- [Discussion: How `reason-promise` makes promises type-safe](#TypeSafety)

<br/>

<a id="Installing"></a>
### Installing

```
npm install reason-promise
```

Then, add `reason-promise` to your `bsconfig.json`:

```json
{
  "bs-dependencies": [
    "reason-promise"
  ]
}
```

<br/>

<a id="GettingStarted"></a>
### Getting started

To quickly get a project for pasting the code examples, clone the
[example repo][example-repo]. The code is in `main.re`.

```
git clone https://github.com/aantron/promise-example-bsb
cd promise-example-bsb
npm install
npm run test    # To run each example.
```

There it also an example repo with
[a trivial binding to parts of node-fetch][example-binding].

While reading the tutorial, it can be useful to glance at the [type
signatures][rei] of the functions from time to time. They provide a neat summary
of what each function does and what it expects from its callback.

<br/>

<a id="Creating"></a>
### Creating new promises

The most basic function for creating a new promise is
[`Promise.pending`][pending]:

```reason
let (p, resolve) = Promise.pending();
Js.log(p);    /* Promise { <pending> } */
```

The second value returned, `resolve`, is a function for resolving the promise:

```reason
let (p, resolve) = Promise.pending();
resolve("Hello");
Js.log(p);    /* Promise { 'Hello' } */
```

[`Promise.resolved`][resolved] is a helper that returns an already-resolved
promise:

```reason
let p = Promise.resolved("Hello");
Js.log(p);    /* Promise { 'Hello' } */
```

...and [`Promise.exec`][exec] is for wrapping functions that take callbacks:

```reason
[@bs.val]
external setTimeout: (unit => unit, int) => unit = "setTimeout";

let p = Promise.exec(resolve => setTimeout(resolve, 1000));
Js.log(p);    /* Promise { <pending> } */

/* Program then waits for one second before exiting. */
```

<br/>

<a id="Values"></a>
### Getting values from promises

To do something once a promise is resolved, use [`Promise.get`][get]:

```reason
let (p, resolve) = Promise.pending();

p->Promise.get(s => Js.log(s));

resolve("Hello");   /* Prints "Hello". */
```

<br/>

<a id="Transforming"></a>
### Transforming promises

Use [`Promise.map`][map] to transform the value inside a promise:

```reason
let (p, resolve) = Promise.pending();

p
->Promise.map(s => s ++ " world")
->Promise.get(s => Js.log(s));

resolve("Hello");   /* Hello world */
```

To be precise, `Promise.map` creates a *new* promise with the transformed value.

If the function you are using to transform the value also returns a promise,
use [`Promise.flatMap`][flatMap] instead of `Promise.map`. `Promise.flatMap`
will flatten the nested promise.

<br/>

<a id="Tracing"></a>
### Tracing

If you have a chain of promise operations, and you'd like to inspect the value
in the middle of the chain, use [`Promise.tap`][tap]:

```reason
let (p, resolve) = Promise.pending();

p
->Promise.tap(s => Js.log("Value is now: " ++ s))
->Promise.map(s => s ++ " world")
->Promise.tap(s => Js.log("Value is now: " ++ s))
->Promise.get(s => Js.log(s));

resolve("Hello");

/*
Value is now: Hello
Value is now: Hello world
Hello world
*/
```

<br/>

<a id="Combining"></a>
### Concurrent combinations

[`Promise.race`][race] waits for *one* of the promises passed to it to resolve:

```reason
[@bs.val]
external setTimeout: (unit => unit, int) => unit = "setTimeout";

let one_second = Promise.exec(resolve => setTimeout(resolve, 1000));
let five_seconds = Promise.exec(resolve => setTimeout(resolve, 5000));

Promise.race([one_second, five_seconds])
->Promise.get(() => { Js.log("Hello"); exit(0); });

/* Prints "Hello" after one second. */
```

[`Promise.all`][all] instead waits for *all* of the promises passed to it,
concurrently:

```reason
[@bs.val]
external setTimeout: (unit => unit, int) => unit = "setTimeout";

let one_second = Promise.exec(resolve => setTimeout(resolve, 1000));
let five_seconds = Promise.exec(resolve => setTimeout(resolve, 5000));

Promise.all([one_second, five_seconds])
->Promise.get(_ => { Js.log("Hello"); exit(0); });

/* Prints "Hello" after five seconds. */
```

For convenience, there are several variants of `Promise.all`:

- [`Promise.all2`][all2]
- [`Promise.all3`][all3]
- [`Promise.all4`][all4]
- [`Promise.all5`][all5]
- [`Promise.all6`][all6]
- [`Promise.allArray`][allArray]

<br/>

<a id="Errors"></a>
### Handling errors with `Result`

Promises that can fail are represented using the standard library's
[`Result`][Result], and its constructors `Ok` and `Error`:

```reason
open Belt.Result;

Promise.resolved(Ok("Hello"))
->Promise.getOk(s => Js.log(s));      /* Hello */
```

[`Promise.getOk`][getOk] waits for `p` to have a value, and runs its function
only if that value is `Ok(_)`. If you instead resolve the promise with
`Error(_)`, there will be no output:

```reason
open Belt.Result;

Promise.resolved(Error("Failed"))
->Promise.getOk(s => Js.log(s));      /* Program just exits. */
```

You can wait for either kind of value by calling [`Promise.getOk`][getOk] and
[`Promise.getError`][getError]:

```reason
open Belt.Result;

let () = {
  let p = Promise.resolved(Error("Failed"));
  p->Promise.getOk(s => Js.log(s));
  p->Promise.getError(s => Js.log("Error: " ++ s));
};                                    /* Error: Failed */
```

...or respond to all outcomes using the ordinary [`Promise.get`][get]:

```reason
open Belt.Result;

Promise.resolved(Error("Failed"))
->Promise.get(result =>
  switch (result) {
  | Ok(s) => Js.log(s);
  | Error(s) => Js.log("Error: " ++ s);
  });                                 /* Error: Failed */
```

The full set of functions for handling results is:

- [`Promise.getOk`][getOk]
- [`Promise.tapOk`][tapOk]
- [`Promise.mapOk`][mapOk]
- [`Promise.flatMapOk`][flatMapOk]
- [`Promise.getError`][getError]
- [`Promise.tapError`][tapError]
- [`Promise.mapError`][mapError]
- [`Promise.flatMapError`][flatMapError]

There are also similar functions for working with [`Option`][Option]:

- [`Promise.getSome`][getSome]
- [`Promise.tapSome`][tapSome]
- [`Promise.mapSome`][mapSome]
- [`Promise.flatMapSome`][flatMapSome]

In addition, there is also a set of variants of `Promise.all` for results, which
propagate any `Error(_)` as soon as it is received:

- [`Promise.allOk`][allOk]
- [`Promise.allOk2`][allOk2]
- [`Promise.allOk3`][allOk3]
- [`Promise.allOk4`][allOk4]
- [`Promise.allOk5`][allOk5]
- [`Promise.allOk6`][allOk6]
- [`Promise.allOkArray`][allOkArray]

If you'd like instead to fully wait for all the promises to resolve with either
`Ok(_)` or `Error(_)`, you can use the ordinary `Promise.all` and its variants.

<br/>

<a id="Rejection"></a>
### Advanced: Rejection

As you can see from [Handling errors](#Errors), `Promise` doesn't use rejection
for errors &mdash; but JavaScript promises do. In order to support bindings to
JavaScript libraries, which often return promises that can be rejected,
`Promise` provides the [`Promise.Js`][Promise.Js] helper module.

`Promise.Js` works the same way as `Promise`. It similarly has:

- [`Promise.Js.get`][Js.get]
- [`Promise.Js.tap`][Js.tap]
- [`Promise.Js.map`][Js.map]
- [`Promise.Js.flatMap`][Js.flatMap]

However, because `Promise.Js` uses JS rejection for error handling rather than
`Result` or `Option`,

- There are no helpers for `Result` and `Option`.
- There is [`Promise.Js.catch`][Js.catch] for handling rejection.
- There is [`Promise.Js.rejected`][Js.rejected] for creating an
  already-rejected promise.

Underneath, `Promise` and `Promise.Js` have the same implementation:

```reason
type Promise.t('a) = Promise.Js.t('a, never);
```

That is, `Promise` is really `Promise.Js` that has no rejection type, and no
exposed helpers for rejection.

There are several helpers for converting between `Promise` and `Promise.Js`:

- [`Promise.Js.relax`][Js.relax]
- [`Promise.Js.toResult`][Js.toResult]
- [`Promise.Js.fromResult`][Js.fromResult]

[`Promise.Js.catch`][Js.catch] can also perform a conversion to `Promise`, if
you simply convert a rejection to a resolution. In the next example, note the
final line is no longer using `Promise.Js`, but `Promise`:

```reason
Promise.Js.rejected("Failed")
->Promise.Js.catch(s => Promise.resolved("Error: " ++ s))
->Promise.get(s => Js.log(s));        /* Error: Failed */
```

There are also two functions for converting between `Promise.Js` and the current
promise binding in the BuckleScript standard libarary, `Js.Promise`:

- [`Promise.Js.fromBsPromise`][Js.fromBsPromise]
- [`Promise.Js.toBsPromise`][Js.toBsPromise]

Because both libraries are bindings for the same exact kind of value, these are
both no-op identity functions that only change the type.

<br>

<a id="Bindings"></a>
### Advanced: Bindings

Refer to the [example node-fetch binding repo][example-binding].

When you want to bind a JS function that *returns* a promise, you can use
`Promise` directly in its return value:

```reason
/* A mock JS library. */
[%%bs.raw {|
function delay(value, milliseconds) {
  return new Promise(function(resolve) {
    setTimeout(function() { resolve(value); }, milliseconds)
  });
}|}]

/* Our binding. */
[@bs.val]
external delay: ('a, int) => Promise.t('a) = "delay";

/* Usage. */
delay("Hello", 1000)
->Promise.get(s => Js.log(s));

/* Prints "Hello" after one second. */
```

If the promise can be rejected, you should use `Promise.Js` instead, and
[convert to `Promise`](#Rejection) as quickly as possible, with intelligent
handling of rejection. Here is one way to do that:

```reason
/* Mock JS library. */
[%%bs.raw {|
function delayReject(value, milliseconds) {
  return new Promise(function(resolve, reject) {
    setTimeout(function() { reject(value); }, milliseconds)
  });
}|}]

/* Binding. */
[@bs.val]
external delayRejectRaw: ('a, int) => Promise.Js.t(_, 'a) = "delayReject";
let delayReject = (value, milliseconds) =>
  delayRejectRaw(value, milliseconds)
  ->Promise.Js.toResult;

/* Usage. */
delayReject("Hello", 1000)
->Promise.getError(s => Js.log(s));

/* Prints "Hello" after one second. */
```

Note that this binding has two steps: there is a raw binding, and then an extra
wrapper that converts rejections into `Result`s. If the potential rejections
are messy, this is a good place to insert additional logic for converting them
to nice Reason values :)

When *passing* a promise to JS, it is generally safe to use `Promise` rather
than `Promise.Js`:

```reason
/* Mock JS library. */
[%%bs.raw {|
function log(p) {
  p.then(function (v) { console.log(v); });
}|}]

/* Binding. */
[@bs.val]
external log: Promise.t('a) => unit = "log";

/* Usage. */
log(Promise.resolved("Hello"));       /* Hello */
```

<br/>

<a id="JSPromiseFlattening"></a>
### Discussion: Why JS promises are unsafe

The JS function [`Promise.resolve`][Promise.resolve] has a special case, which
is triggered when you try to resolve a promise with another, nested promise.
Unfortunately, this special case makes it impossible to assign
`Promise.resolve` a consistent type in Reason (and most type systems).

Here are the details. The code will use
[`Js.Promise.resolve`][Js.Promise.resolve], BuckleScript's direct binding to
JS's `Promise.resolve`.

`Js.Promise.resolve` takes a value, and creates a promise containing that value:

```reason
Js.log(Js.Promise.resolve(1));
/* Promise { 1 } */

Js.log(Js.Promise.resolve("foo"));
/* Promise { 'foo' } */
```

So, we should give it the type

```reason
Js.Promise.resolve: 'a => Js.Promise.t('a);
```

and, indeed, that's the type it [has][Js.Promise.resolve] in BuckleScript.

Following the pattern, we would *expect*:

```reason
let nestedPromise = Js.Promise.resolve(1);

Js.log(Js.Promise.resolve(nestedPromise));
/* Promise { Promise { 1 } } */
```

But that's not what happens! Instead, the output is just

```reason
/* Promise { 1 } */
```

The nested promise is missing! But the type system, following the pattern,
still thinks that this resulting value has type

```reason
Js.Promise.t(Js.Promise.t(int))
```

i.e., the type of the value we were (reasonably) expecting.

When you pass `nestedPromise` to `Js.Promise.resolve`, JS unwraps
`nestedPromise`, violating the type! There is no easy way to encode such special
casing in the type system &mdash; especially since JS does it not only to
nested promises, but to any would-be nested object that has a `.then` method.

The result is, if your program executes something like this, it will have
ordinary values in places where it expects another level of promises. For
example, if you do

```reason
let nestedPromise = Js.Promise.resolve(1);

Js.Promise.resolve(nestedPromise)
->Js.Promise.then_(p => /* ... */)
```

you would *expect* `p` in the callback to be a promise containing `1`, and the
type of `p` is indeed `Js.Promise.t(int)`. Instead, however, `p` is just the
bare value `1`. That means the callback will cause a runtime error as soon as
it tries to use promise functions on the `1`. Worse, you might store `p` in a
data structure, and the runtime error will occur at a very distant place in the
code. The type system is supposed to prevent such errors! That's part of the
point of using Reason.

The same special casing occurs throughout the JS `Promise` API &mdash; for
example, when you return a promise from the callback of `then_`. This means that
*most* of the JS `Promise` functions can't be assigned a correct type and
directly, safely be used from Reason.

<br/>

<a id="TypeSafety"></a>
### Discussion: How `reason-promise` makes promises type-safe

The [previous section](#JSPromiseFlattening) shows that JS promise functions are
broken. An important observation is that it is only the *functions* that are
broken &mdash; the promise *data structure* is not. That means that to make JS
promises type-safe, we can keep the existing JS data structure, and just provide
safe replacement functions to use with it in Reason. This is good news
for interop :)

To fix the functions, only the [special-case flattening](#JSPromiseFlattening)
has to be undone. So, when you call `reason-promise`'s
[`Promise.resolved(value)`][resolved], it checks whether `value` is a promise
or not, and...

- If `value` *is not* a promise, `reason-promise` just passes it to JS's
  [`Promise.resolve`][Promise.resolve], because JS will do the right thing.
- If `value` *is* a promise, it's not safe to simply pass it to JS, because it
  will trigger the special-casing. So, `reason-promise` boxes the nested
  promise:

  ```reason
  let nestedPromise = Promise.resolved(1);

  Js.log(Promise.resolved(nestedPromise));
  /* Promise { PromiseBox { Promise { 1 } } } */
  ```

  This box, of course, is not a promise, so inserting it in the middle is
  enough to suppress the special-casing.

  Whenever you try to take the value out of this resulting structure (for
  example, by calling [`Promise.get`][get] on it), `reason-promise`
  transparently unboxes the `PromiseBox` and passes the nested promise to your
  callback &mdash; as your callback would expect.

This conditional boxing and unboxing is done throughout `reason-promise`. It
only happens for nested promises, and anything else with a `.then` method. For
all other values, `reason-promise` behaves, internally, exactly like JS
`Promise` (though with a cleaner outer API). This is enough to make promises
type-safe.

This is a simple scheme, but `reason-promise` includes a very thorough
[test suite][tests] to be extra sure that it always manages the boxing
correctly.

This conditional boxing is similar to how unboxed optionals are implemented in
BuckleScript. Optionals are almost always unboxed, but when BuckleScript isn't
sure that the unboxing will be safe, it inserts a runtime check that boxes some
values, while still keeping most values unboxed.

[example-repo]: https://github.com/aantron/promise-example-bsb
[example-binding]: https://github.com/aantron/promise-example-binding
[rei]: https://github.com/aantron/promise/blob/c68b1feefdd5efc0397ba92f392d6cc47233f161/src/js/promise.rei#L15
[Result]: https://bucklescript.github.io/bucklescript/api/Belt.Result.html
[Option]: https://bucklescript.github.io/bucklescript/api/Belt.Option.html
[tests]: https://github.com/aantron/promise/tree/master/test

[pending]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L20-L22
[resolved]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L24-L26
[exec]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L28-L30
[get]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L35-L37
[map]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L43-L45
[flatMap]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L47-L49
[tap]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L39-L41
[race]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L123-L125
[all]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L127-L129
[all2]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L135-L137
[all3]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L139-L141
[all4]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L143-L145
[all5]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L147-L149
[all6]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L151-L158
[allArray]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L131-L133
[getOk]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L57-L59
[getError]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L73-L75
[tapOk]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L61-L63
[tapError]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L77-L79
[mapOk]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L65-L67
[mapError]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L81-L83
[flatMapOk]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L69-L71
[flatMapError]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L85-L87
[getSome]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L104-L106
[tapSome]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L108-L110
[mapSome]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L112-L114
[flatMapSome]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L116-L118
[allOk]: https://github.com/aantron/promise/blob/8142b0c4cb5e88e0241c3a6926fdf096b1b96935/src/js/promise.rei#L160-L162
[allOk2]: https://github.com/aantron/promise/blob/8142b0c4cb5e88e0241c3a6926fdf096b1b96935/src/js/promise.rei#L168-L170
[allOk3]: https://github.com/aantron/promise/blob/8142b0c4cb5e88e0241c3a6926fdf096b1b96935/src/js/promise.rei#L172-L176
[allOk4]: https://github.com/aantron/promise/blob/8142b0c4cb5e88e0241c3a6926fdf096b1b96935/src/js/promise.rei#L178-L183
[allOk5]: https://github.com/aantron/promise/blob/8142b0c4cb5e88e0241c3a6926fdf096b1b96935/src/js/promise.rei#L185-L191
[allOk6]: https://github.com/aantron/promise/blob/8142b0c4cb5e88e0241c3a6926fdf096b1b96935/src/js/promise.rei#L193-L200
[allOkArray]: https://github.com/aantron/promise/blob/8142b0c4cb5e88e0241c3a6926fdf096b1b96935/src/js/promise.rei#L164-L166
[Promise.Js]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L163
[Js.get]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L180-L182
[Js.tap]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L184-L186
[Js.map]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L188-L190
[Js.flatMap]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L192-L194
[Js.catch]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L197-L199
[Js.rejected]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L175-L177
[Js.relax]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L211-L213
[Js.toResult]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L215-L217
[Js.fromResult]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L219-L221
[Js.fromBsPromise]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L223-L225
[Js.toBsPromise]: https://github.com/aantron/promise/blob/51001f911ff31ecf51a633fba9f782769a2726c9/src/js/promise.rei#L227-L229
[Promise.resolve]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/resolve
[Js.Promise.Resolve]: https://bucklescript.github.io/bucklescript/api/Js.Promise.html#VALresolve

<!-- YOU HAVE FOUND THE SECRET EASTER EGG! -->
