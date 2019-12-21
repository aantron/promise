# Promise &nbsp;&nbsp;&nbsp; [![Version 0.6.1][npm-img]][npm] [![Travis status][travis-img]][travis] [![Coverage][coveralls-img]][coveralls]

[npm-img]: https://img.shields.io/npm/v/@aantron/repromise.svg
[npm]: https://www.npmjs.com/package/@aantron/repromise
[travis]: https://travis-ci.org/aantron/repromise/branches
[travis-img]: https://img.shields.io/travis/aantron/repromise/master.svg?label=travis
[coveralls]: https://coveralls.io/github/aantron/repromise?branch=master
[coveralls-img]: https://img.shields.io/coveralls/aantron/repromise/master.svg

<br/>

```reason
let (p, resolve) = Promise.pending();
p
->Promise.map((s) => s ++ " world!")
->Promise.get(Js.log);
resolve("Hello");

/* Hello world! */
```

<br/>

**Repromise** is a simple Reason promise library. It has:

- **Clean API** &mdash; correct types, unlike JS promises.
- **Interop** &mdash; nonetheless, each Repromise is a JS promise underneath.
You can `console.log` them 😄

How does this work?

It turns out that a cheap runtime check is all that's needed to make JS
promises behave safely.

So, Repromise is a very thin layer over the familiar JS `Promise`, which
converts `Promise` into the kind of neat API you'd want to use in ReasonML.
This gives...

- **Simplicity** &mdash; no new runtime data types, no bookkeeping.
- **Bindings** &mdash; anything that takes or returns `Promise`, already takes
and returns Repromise.
- **Tiny size**
- **Memory safety** &mdash; Repromise avoids all known pitfalls of naive
implementations.
- **Semantics** &mdash; no timing issues, no starvation, etc. JS `Promise`
actually gets this right 😆

And finally, since the API is a clean start on promises...

- **Better errors** &mdash; Repromises can't be rejected. Instead, they use
`Result`.
- **Ecosystem** &mdash; direct interop with Reason native and OCaml, with libuv
and Lwt.

Even though Repromise is simple, it has a ton of tests to make sure that JS
`Promise` is behaving safely :smile: The tests also check that Repromise
behaves the same way on BuckleScript and native, even checking timing and
typical memory leaks!

<br/>

## Installing

```
npm install bs-platform
npm install @aantron/repromise
```

Then, add `@aantron/repromise` to your `bsconfig.json`:

```json
{
  "bs-dependencies": [
    "@aantron/repromise"
  ]
}
```

<br/>

## Tutorial

To get started quickly, you can clone the starter repo:

```
git clone https://github.com/aantron/repromise-example-bsb tutorial
cd tutorial
npm install
npm run test    /* To run the code. */
```

Open `main.re` and create a new promise with `Repromise.make`:

```reason
let (p, resolveP) = Repromise.make();
Js.log(p);    /* Promise { <pending> } */
```

To resolve the promise, use `resolveP`:

```reason
let (p, resolveP) = Repromise.make();
resolveP("Hi!");
Js.log(p);    /* Promise { 'Hi!' } */
```

#### Waiting on promises

To do something once a promise is resolved, use `wait`:

```reason
let (p, resolveP) = Repromise.make();
p |> Repromise.wait(Js.log);
resolveP("Hi!")    /* Hi! */
```

#### Transforming promises

Use `map` for transforming the value inside a promise:

```reason
let (p, resolveP) = Repromise.make();
p
|> Repromise.map(s => s ++ ", there!")
|> Repromise.wait(Js.log);
resolveP("Hi");    /* Hi, there! */
```

To be a little more precise, `map` creates a new promise with the transformed
value.

If the function you are using to transform the promise also returns a promise,
use `andThen` instead of `map`.

#### Handling errors

Potential errors are represented using Repromises that contain `Result`. Use
`mapOk` to transform these promises:

```reason
let (p, resolveP) = Repromise.make();
p
|> Repromise.mapOk(s => s ++ ", there!")
|> Repromise.waitOk(Js.log);
resolveP(Ok("Hi"));    /* Hi, there! */
```

If you instead resolve the promise with `Error(_)`, there will be no output:

```reason
let (p, resolveP) = Repromise.make();
p
|> Repromise.mapOk(s => s ++ ", there!")
|> Repromise.waitOk(Js.log);
resolveP(Error("Oh no!"));
```

Apart from `mapOk`, there is also `mapError` for transforming the `Error`
instead of the `Ok` value. There are also `waitOk`, `waitError`, `andThenOk`,
and `andThenError` for all the other kinds of transformations.

<br/>

## Documentation

The full API can be neatly seen at a glance in the [`.rei` file][rei]. Repromise
also has proper docs posted [online][docs], including how to write
[bindings][bindings].

[rei]: https://github.com/aantron/repromise/blob/readme/src/js/repromise.rei
[docs]: https://aantron.github.io/repromise/
[bindings]: https://aantron.github.io/repromise/docs/Interop
