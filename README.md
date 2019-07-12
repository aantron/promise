# Repromise &nbsp;&nbsp; [![Version 0.6.1][npm-img]][npm] [![Travis status][travis-img]][travis]

[npm-img]: https://img.shields.io/npm/v/@aantron/repromise.svg
[npm]: https://www.npmjs.com/package/@aantron/repromise
[travis]: https://travis-ci.org/aantron/repromise/branches
[travis-img]: https://img.shields.io/travis/aantron/repromise/master.svg?label=travis

Repromise is a binding to JS promises, with:

- **Interop** &mdash; each Repromise [is a JS promise][representation], so JS libraries that return promises already return Repromises, and existing JS tooling should work.
- **Type safety** &mdash; Repromises nest, and don't ["collapse"][broken] like JS promises.
- **Explicit errors** &mdash; no baked-in `reject` or exception handling, so you can choose how to handle errors.
- **Minimal code** &mdash; the compiled `repromise.js` compresses to about 1K.
- **Native support** &mdash; Repromise also has a [pure-Reason implementation][native], which compiles to native code.

<br/>

```ocaml
let (p, resolve_p) = Repromise.make();

p
|> Repromise.map(s => s ++ ", world!")
|> Repromise.wait(print_endline);

resolve_p("Hello");

/* Prints "Hello, world!" */
```

<br/>

## Installing

```
npm install bs-platform
npm install @aantron/repromise
```

Add `@aantron/repromise` to your `bsconfig.json`:

```json
{
  "bs-dependencies": [
    "@aantron/repromise"
  ]
}
```

<br/>

## Sample projects

We have a couple repos that show how to depend on Repromise and get started with it:

- [Minimal project that prints `"Hello, world!"`.][example-bsb]
- [Minimal binding to a JS library, `node-fetch`.][example-binding]

<br/>

## Docs

...are [online][docs]!

To begin, we recommend looking at the [`"Hello, world!"` project][example-bsb], and then reading the [API docs][api]. Starting from [`wait`][Repromise.wait], each function has examples. You can paste them into the `"Hello, world!"` project to play around with Repromise.

The [Design FAQ][design] gives some background about Repromise.

To learn how to write bindings, we suggest starting with the [`node-fetch` project][example-binding], then reading the [Interop docs][interop], and perhaps the [rejectable API docs][rejectable].

<br/>

## Roadmap

- [ ] Release [native implementation][native].
- [ ] Helpers for converting `Repromise.Rejectable.t` to `Belt.Result.t`.
- [ ] Smarter [async exception handling][onUnhandledException].

<br/>

## Contact

Please don't hesitate to [open an issue][issue], or come [bug us on Discord][discord]. All questions, feedback, and [bikeshedding][bikeshedding] are welcome :)

Repromise is at a pretty early stage of development, and we can change just about everything.

[example-bsb]: https://github.com/aantron/repromise-example-bsb#readme
[example-binding]: https://github.com/aantron/repromise-example-binding#readme
[native]: https://github.com/aantron/repromise/blob/master/src/native/repromise.re
[broken]: https://aantron.github.io/repromise/docs/DesignFAQ#why-are-js-promises-not-type-safe
[rejectable]: https://aantron.github.io/repromise/docs/RejectableAPI
[design]: https://aantron.github.io/repromise/docs/DesignFAQ
[Repromise.wait]: https://aantron.github.io/repromise/docs/API#wait
[api]: https://aantron.github.io/repromise/docs/API
[docs]: https://aantron.github.io/repromise
[interop]: https://aantron.github.io/repromise/docs/Interop
[issue]: https://github.com/aantron/repromise/issues/new
[discord]: https://discordapp.com/invite/reasonml
[representation]: https://aantron.github.io/repromise/docs/Interop#representation
[onUnhandledException]: https://aantron.github.io/repromise/docs/API#onunhandledexception
[bikeshedding]: https://github.com/aantron/repromise/issues/22
