# Repromise &nbsp;&nbsp; [![Version 0.5.0][npm-img]][npm] [![Travis status][travis-img]][travis]

[npm-img]: https://img.shields.io/npm/v/@aantron/repromise.svg
[npm]: https://www.npmjs.com/package/@aantron/repromise
[travis]: https://travis-ci.org/aantron/repromise/branches
[travis-img]: https://img.shields.io/travis/aantron/repromise/master.svg?label=travis

Repromise is a clean JS Promise API for ReasonML, with:

- **Soundness** &mdash; Repromises nest, and don't "collapse" like JS promises.
- **Explicit errors** &mdash; no baked-in `reject` or exception handling, so you can choose how to handle errors.
- **Interop** &mdash; each Repromise is still a JS promise, so all JS APIs that return promises already return Repromises, and JS tooling should work!
- **Type safety** &mdash; even though Repromises and JS promises are the same, they cannot be freely converted. So, when you are in Reason and using Repromise, everything is guaranteed to work predictably and safely.

Basically, JS promise objects are good, but the functions (`Promise.resolve`, `Promise.then`, etc.) are [broken][broken]. Repromise uses the objects, but provides better functions.

Repromise is lightweight: the compiled code compresses to about 1K.

Repromise also has a native implementation, written in pure ReasonML. We haven't published this yet, though it is [in the repo][native].

[native]: https://github.com/aantron/repromise/blob/master/src/native/repromise.re
[broken]: https://aantron.github.io/repromise/docs/DesignFAQ#why-are-js-promises-not-type-safe

<br/>

## Installing

```
npm install bs-platform
npm install @aantron/repromise
```

Add `reason-repromise` to your `bsconfig.json`:

```json
{
  "bs-dependencies": [
    "@aantron/repromise"
  ]
}
```

See the [sample project][example-bsb] for a minimal working setup. There is also a minimal example of a [binding to an existing JS library][example-binding].

<br/>

## Using

The Repromise API is pretty similar to JS promises, but...

- There is no `catch` or `reject`.
- Instead of only `then`, there are [`Repromise.then_`][Repromise.then], [`Repromise.map`][Repromise.map], and [`Repromise.wait`][Repromise.wait], which all schedule a callback, but have slightly different types.

Create new pending promises with [`Repromise.new_`][Repromise.new]:

```ocaml
let (p, resolve_p) = Repromise.new_();
```

[`Repromise.wait`][Repromise.wait] adds a callback, which will run when the promise resolves:

```ocaml
p |> Repromise.wait(text => print_endline(text));
```

Resolving `p` now will print `"Hello"`:

```ocaml
resolve_p("Hello");
```

Promises can be transformed using [`Repromise.map`][Repromise.map]:

```ocaml
let (p, resolve_p) = Repromise.new_();
p
|> Repromise.map(text => text ++ " world!")
|> Repromise.wait(text => print_endline(text));
resolve_p("Hello");
```

...and [`Repromise.then_`][Repromise.then] is for when your transformation returns another promise.

See the [API docs][api] for the rest of the functions. Most of them have examples, which you can try out by pasting them into the [sample repo][example-bsb].

[Repromise.new]: https://aantron.github.io/repromise/docs/API#new
[Repromise.then]: https://aantron.github.io/repromise/docs/API#then
[Repromise.map]: https://aantron.github.io/repromise/docs/API#map
[Repromise.wait]: https://aantron.github.io/repromise/docs/API#wait
[api]: https://aantron.github.io/repromise/docs/API

<br/>

## Interop

Since Repromises are JS promises, you can declare that existing JS APIs return Repromise objects directly. For example, here is part of a binding to `node-fetch`:

```ocaml
[@bs.send]
external text: response => Repromise.t(string) = "";
```

The full example is in the [binding sample repo][example-binding], and see the [interop docs][interop] for all the details.

[interop]: https://aantron.github.io/repromise/docs/Interop

<br/>

## Roadmap

- [ ] Release native implementation
- [ ] Release `let%await` syntax (the PPX)
- [ ] Helpers for converting `Repromise.Rejectable.t` to `Belt.Result.t`

[example-bsb]: https://github.com/aantron/repromise-example-bsb
[example-binding]: https://github.com/aantron/repromise-example-binding
