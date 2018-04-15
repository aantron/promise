# Repromise &nbsp;&nbsp; [![Travis status][travis-img]][travis]

[**Repromise**][repromise] is a Reason promise library that works on all
platforms: the browser, Node.js, and native Linux, macOS, Windows. For example,
[the demo][demo],

```reason
let () = {
  ignore({
    let%await fd   = Io.open_("test/demo/demo.re");
    let%await data = Io.read (~fd, ~length = 1024);
    print_endline(data);
    Repromise.resolve();
  });

  Io.run();
};
```

...works on Node and native, and prints its own source code :)

<br/>

Interoperability is key. On JavaScript, `Repromise.t`s are the native
JavaScript promises we are all used to. For targeting machine code, Repromise
provides an [implementation][native] with equivalent semantics.

A small [Reason PPX (preprocessor)][ppx] provides the `let%await` syntax, which
is the Reason counterpart to `async`/`await`.

The [`Io` module][io] is a proof-of-concept at this point. It is implemented
using [libuv][libuv] when targeting machine code, and Node.js when targeting
JavaScript. Either way, the underlying implementation is the same, because
Node.js also uses libuv.



<br/>

## Participate!

We don't really know the best way to bind JS promises in Reason yet, so the
goal of Repromise is to iterate quickly, until we have something pretty good.

Take a look in the [issue tracker][issues] for discussions, and join us in
[ReasonML Discord][discord] for figuring this thing out. All feedback is very
welcome :)



<br/>

## Trying the code

Repromise doesn't have release packaging yet, but here is how you can run the
code in the repo...

#### With npm (JS):

```
opam switch 4.02.3+buckle-master
eval `opam config env`
opam pin add --dev-repo reason
git clone https://github.com/aantron/repromise.git
cd repromise
npm install
npm run build
(cd test && npm install && npm run build)
node ./test/lib/js/test.js
```

See [aantron/repromise-example-bsb][example-bsb] for how to depend on Repromise
in your project.

The `let%await` syntax is not yet easy to pull in with NPM, because it requires
building binaries, etc. We recommend waiting for it to be upstreamed into
BuckleScript, or for proper release packaging to be announced later.

<br/>

#### With esy (native):

```
git clone https://github.com/aantron/repromise.git
cd repromise
esy install
esy jbuilder exec test/test.exe
```

See [aantron/repromise-example-esy][example-esy] for an example starter repo.

<br/>

#### With opam (native):

```
git clone https://github.com/aantron/repromise.git
cd repromise
opam pin add --dev-repo reason
opam pin add repromise .
opam pin add ppx_await .
jbuilder exec test/test.exe
```




[repromise]: https://github.com/aantron/repromise
[demo]: https://github.com/aantron/repromise/blob/5debf48c00f1b101de389d5aae015b9f0fa9a63b/test/demo/demo.re
[native]: https://github.com/aantron/repromise/blob/5debf48c00f1b101de389d5aae015b9f0fa9a63b/src/native/repromise.re
[ppx]: https://github.com/aantron/repromise/blob/5debf48c00f1b101de389d5aae015b9f0fa9a63b/src/ppx/bucklescript/ppx_await.re
[io]: https://github.com/aantron/repromise/blob/5debf48c00f1b101de389d5aae015b9f0fa9a63b/src/io.rei
[opam]: http://opam.ocaml.org/
[node]: https://nodejs.org/en/
[npm]: https://www.npmjs.com/
[libuv]: http://libuv.org/
[bs]: https://github.com/BuckleScript/bucklescript
[bsb-native]: https://github.com/bsansouci/bsb-native
[issues]: https://github.com/aantron/repromise/issues?utf8=%E2%9C%93&q=label%3Adiscuss+
[discord]: https://discordapp.com/invite/reasonml
[travis]: https://travis-ci.org/aantron/repromise/branches
[travis-img]: https://img.shields.io/travis/aantron/repromise/master.svg?label=travis
[example-bsb]: https://github.com/aantron/repromise-example-bsb
[example-esy]: https://github.com/aantron/repromise-example-esy
