# Promise &nbsp;&nbsp;&nbsp; [![Version 1.0.2][version-img]][version] [![Travis status][travis-img]][travis] [![Coverage][coveralls-img]][coveralls]

[version-img]: https://img.shields.io/badge/version-1.0.2-blue.svg
[version]: https://github.com/aantron/promise/releases
[travis]: https://travis-ci.org/aantron/promise/branches
[travis-img]: https://img.shields.io/travis/aantron/promise/master.svg?label=travis
[coveralls]: https://coveralls.io/github/aantron/promise?branch=master
[coveralls-img]: https://img.shields.io/coveralls/aantron/promise/master.svg

A super light and type-safe binding to JS promises.

```reason
Promise.resolved("Hello")->Js.log;  /* Promise { 'Hello' } */

Promise.resolved("Hello")
->Promise.map(s => s ++ " world!")
->Promise.get(s => Js.log(s));      /* Hello world! */
```

As you can see on the first line, `Promise.t` maps directly to familiar JS
promises from your JS runtime. That means...

- You can use `reason-promise` directly to [write JS bindings](#Bindings).
- All JS tooling for promises immediately applies to `reason-promise`.
- Even if you do something exotic, like switch out the promise implementation at
  the JS level, for, say, better stack traces, `reason-promise` still binds to
  it!

There is only one exception to the rule that `Promise.t` maps directly to JS
promises: when there is a promise nested inside another promise. JS doesn't
allow this at all. [`reason-promise` emulates it in a way that makes its API
type-safe](#TypeSafety). This is in contrast to BuckleScript's built-in
`Js.Promise`, which exposes the JS behavior that silently flattens nested
promises, with the result that [the API in BuckleScript has incorrect
types](#JSPromiseFlattening).

In addition:

- `reason-promise` offers a clean functional API, with [helpers for `Result` and
  `Option`](#Errors).
- `reason-promise` is tiny. It weighs in at about [1K bundled][bundle-size].
- `reason-promise` also comes with a [pure-Reason implementation][native], which
  passes all the same tests. It can be used for native code or in JS.

[bundle-size]: https://travis-ci.org/aantron/promise/jobs/638748670#L241
[native]: https://github.com/aantron/promise/tree/master/src/native

<br/>

## Installing

Run

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

## Tutorial

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

<br>

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

<a id="Creating"></a>
### Creating new promises

The most basic function for creating a new promise is
[`Promise.pending`][pending], which gives you a promise and a function for
resolving it:

```reason
let (p, resolve) = Promise.pending();
Js.log(p);    /* Promise { <pending> } */

resolve("Hello");
Js.log(p);    /* Promise { 'Hello' } */
```

[`Promise.resolved`][resolved] is a helper that returns an already-resolved
promise:

```reason
let p = Promise.resolved("Hello");
Js.log(p);    /* Promise { 'Hello' } */
```

...and [`Promise.exec`][exec] is for running functions that take callbacks:

```reason
[@bs.val]
external setTimeout: (unit => unit, int) => unit = "setTimeout";

let p =
  Promise.exec(resolve => setTimeout(resolve, 1000))
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

resolve("Hello");   /* "Hello" is logged. */
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

You can wait for either kind of value by calling [`Promise.getOk`][getOk] or
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

<br/>

<a id="Rejection"></a>
### Advanced: Rejection

As you can see from [Handling errors](#Errors), `Promise` doesn't use rejection
for errors &mdash; but JavaScript promises do. In order to support bindings to
JavaScript libraries, which often return promises that can be rejected,
`Promise` provides the `Promise.Js` helper module.

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

Refer to the [example repo][example-binding].

When you want to bind a JS function that *returns* a promise, you can use
`Promise` directly in its return value:

```reason
[%%bs.raw {|
function delay(value, milliseconds) {
  return new Promise(function(resolve) {
    setTimeout(function() { resolve(value); }, milliseconds)
  });
}|}]

[@bs.val]
external delay: ('a, int) => Promise.t('a) = "delay";

delay("Hello", 1000)
->Promise.get(s => Js.log(s));

/* Prints "Hello" after one second. */
```

If the promise can be rejected, you should use `Promise.Js` instead, and
[convert to `Promise`](#Rejection) as quickly as possible. Here is one way to
do that:

```reason
[%%bs.raw {|
function delayReject(value, milliseconds) {
  return new Promise(function(resolve, reject) {
    setTimeout(function() { reject(value); }, milliseconds)
  });
}|}]

[@bs.val]
external delayRejectRaw: ('a, int) => Promise.Js.t(_, 'a) = "delayReject";
let delayReject = (value, milliseconds) =>
  delayRejectRaw(value, milliseconds)
  ->Promise.Js.toResult;

delayReject("Hello", 1000)
->Promise.getError(s => Js.log(s));

/* Prints "Hello" after one second. */
```

When *passing* a promise to JS, it is generally safe to use `Promise` rather
than `Promise.Js`:

```reason
[%%bs.raw {|
function log(p) {
  p.then(function (v) { console.log(v); });
}|}]

[@bs.val]
external log: Promise.t('a) => unit = "log";

log(Promise.resolved("Hello"));       /* Hello */
```

As always, it is important to be careful about the set of values that a promise
can be resolved or rejected with, since JS can return anything :) Additional JS
code may be necessary to handle this, as with any JS binding.

<br/>

<a id="JSPromiseFlattening"></a>
### Discussion: Why JS promises are unsafe

The JS function [`Promise.resolve`][Promise.resolve] has a special check for
whether the value being put into a promise is another promise or not.
Unfortunately, this check makes it impossible to assign JS's `Promise.resolve`
a correct type in Reason (and most type systems).

Here are the details. The code will use
[`Js.Promise.resolve`][Js.Promise.resolve], BuckleScript's binding to JS's
`Promise.resolve`.

`Js.Promise.resolve` takes a value, and creates a promise containing that value:

```reason
Js.Promise.resolve(1)->Js.log;
/* Promise { 1 } */

Js.Promise.resolve("foo")->Js.log;
/* Promise { "foo" } */
```

So, we should give it the type

```reason
Js.Promise.resolve: 'a => Js.Promise.t('a);
```

and, indeed, that's the type it has in BuckleScript.

Following the pattern, we would expect this:

```reason
let anotherPromise = Js.Promise.resolve(1);
Js.Promise.resolve(anotherPromise)->Js.log;
/* Promise { Promise { 1 } } */
```

We would expect the result to have type `Js.Promise.t(Js.Promise.t(int))`...
but that's not what happens! Instead, the output is just

```reason
/* Promise { 1 } */
```

The nested promise is missing! When you pass `anotherPromise` to
`Js.Promise.resolve`, JS sneakily unwraps `anotherPromise`, violating the type!
This is special-case behavior that JS runs only when the value is a promise
(technically, a "thenable"), and there is no way to easily encode such special
casing in the type system.

The result is, if your program executes something like this, it will have
ordinary values in places where it expects promises. For example, if you call
`then_` on the promise above, you would expect the program to see a promise
containing `1` in the callback. Instead, the callback will get just `1`, causing
a runtime error as soon as the program tries to use promise functions on the
`1`.

The same special casing occurs throughout the JS `Promise` API &mdash; for
example, when you return a promise from the callback of `.then`. This means that
most of the JS `Promise` functions can't be assigned a correct type and used
safely from Reason.

<br/>

<a id="TypeSafety"></a>
### Discussion: How `reason-promise` makes promises type-safe

The previous section shows that [JS promise functions are
broken](#JSPromiseFlattening). An important observation is that it is only the
*functions* that are broken &mdash; the promise *data representation* is not.
That means that to make JS promises type-safe, we can keep the same data
representation, and just provide safe replacement functions to use with it in
Reason. This is good news for interop :)

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
  let value = Promise.resolved(1);
  Promise.resolved(value)->Js.log;
  /* Promise { PromiseBox { Promise { 1 } } } */
  ```

  This box, of course, is not a promise, so it's enough to bypass the
  special-casing.

  Whenever you try to take the value out of this resulting promise (for example,
  by calling [`Promise.get`][get] on it), `reason-promise` transparently
  unboxes the nested promise before passing it to your callback.

This conditional boxing and unboxing is done throughout `reason-promise`. It
only happens for nested promises. For all other values, `reason-promise`
behaves, internally, exactly like JS `Promise` (though with a cleaner outer
API). This is enough to make promises type-safe.

This is a simple scheme, but `reason-promise` includes a very thorough
[test suite][tests] to be extra sure that it always manages the boxing
correctly.

This conditional boxing is similar to how unboxed optionals are implemented in
BuckleScript. Optionals are almost always unboxed, but when BuckleScript isn't
sure that the unboxing will be safe, it inserts a runtime check that boxes some
values, while still keeping most values unboxed.

[example-repo]: https://github.com/aantron/promise-example-bsb
[example-binding]: https://github.com/aantron/promise-example-binding
[rei]: https://github.com/aantron/promise/blob/c68b1feefdd5efc0397ba92f392d6cc47233f161/src/js/promise.rei#L15-L154
[Result]: https://bucklescript.github.io/bucklescript/api/Belt.Result.html
[Option]: https://bucklescript.github.io/bucklescript/api/Belt.Option.html
[tests]: https://github.com/aantron/promise/tree/master/test

[pending]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L16-L18
[resolved]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L20-L22
[exec]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L24-L26
[get]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L31-L33
[map]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L39-L41
[flatMap]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L43-L45
[tap]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L35-L37
[race]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L117-L119
[all]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L121-L123
[all2]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L129-L131
[all3]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L133-L135
[all4]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L137-L139
[all5]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L141-L143
[all6]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L145-L152
[allArray]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L125-L127
[getOk]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L53-L55
[getError]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L69-L71
[tapOk]:https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L57-L59
[tapError]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L73-L75
[mapOk]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L61-L63
[mapError]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L77-L79
[flatMapOk]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L65-L67
[flatMapError]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L81-L83
[getSome]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L98-L100
[tapSome]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L102-L104
[mapSome]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L106-L108
[flatMapSome]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L110-L112
[Js.get]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L174-L176
[Js.tap]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L178-L180
[Js.map]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L182-L184
[Js.flatMap]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L186-L188
[Js.catch]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L191-L193
[Js.rejected]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L169-L171
[Js.relax]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L205-L207
[Js.toResult]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L209-L211
[Js.fromResult]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L213-L215
[Js.fromBsPromise]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L217-L219
[Js.toBsPromise]: https://github.com/aantron/promise/blob/dcbd596fa8207c62b7c26416ff681f25e6eedb67/src/js/promise.rei#L221-L223
[Promise.resolve]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/resolve
[Js.Promise.Resolve]: https://bucklescript.github.io/bucklescript/api/Js.Promise.html#VALresolve

<!-- YOU HAVE FOUND THE SECRET EASTER EGG! -->
