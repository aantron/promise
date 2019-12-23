# Promise &nbsp;&nbsp;&nbsp; [![Version 0.6.1][version-img]][version] [![Travis status][travis-img]][travis] [![Coverage][coveralls-img]][coveralls]

[version-img]: https://img.shields.io/badge/version-0.6.1-blue.svg
[version]: https://github.com/aantron/promise/releases
[travis]: https://travis-ci.org/aantron/promise/branches
[travis-img]: https://img.shields.io/travis/aantron/promise/master.svg?label=travis
[coveralls]: https://coveralls.io/github/aantron/promise?branch=master
[coveralls-img]: https://img.shields.io/coveralls/aantron/promise/master.svg

A super light and type-safe binding to JS promises.

```reason
let p = Promise.resolved("Hello");
Js.log(p);

/* Promise { 'Hello' } */

p
->Promise.map(s => s ++ " world!")
->Promise.get(s => Js.log(s));

/* Hello world! */
```

As you can see from `Js.log(p)`, each `Promise` is a familiar JS promise. So,
you can use `Promise` directly to write JS bindings.

There is only one exception to this: when there is a promise nested inside
another promise. JS doesn't allow this. `Promise` automatically handles that
for you. The way `Promise` does that makes JS promises type-safe in Reason.

`Promise` also offers a cleaner API, as we would expect in ReasonML :)

`Promise` is just a tiny binding with a few small tricks. It weighs in at about
1K bundled.

`Promise` also comes with a native implementation, which passes all the same
tests. The plan is to make `Promise` interop with Lwt, libuv, and js_of_ocaml.

<br/>

## Installing

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
