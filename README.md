# Repromise

**Repromise** is an asynchronous programming library that makes your Reason
code run everywhere: on the browser or in Node.js, and natively under Linux,
macOS, Windows:

```reason
let () = {
  ignore({
    let%await fd   = Io.open_("test/test.re");
    let%await data = Io.read (~fd, ~length = 1024);
    print_endline(data);
    Repromise.resolve();
  });

  Io.run();
};
```

...the above compiles to both JavaScript and machine code!

Interoperability is key: on JavaScript, `Repromise.t`s are exactly JavaScript's
familiar `Promise`s. For targeting native code, Repromise provides an
implementation with the same semantics.

The `Io` module is powered by libuv when targeting native code, and Node.js on
JavaScript. Either way, the underlying implementation is the same, because
Node.js is *itself* powered by libuv!

Repromise is actually capable of providing synchronous I/O functions as well, we
just haven't exposed them yet.

Finally, Repromise includes a small preprocessor so you can write the neat and
familiar `let%await` syntax as in the example above!



<br/>

## Trying it out

#### With esy:

```
git clone https://github.com/aantron/repromise.git
cd repromise
esy install
esy jbuilder build test/test.exe
_build/default/test/test.exe
```

#### With opam:

```
git clone https://github.com/aantron/repromise.git
cd repromise
opam pin add --dev-repo reason
opam pin add repromise .
opam pin add ppx_await .
jbuilder build test/test.exe
_build/default/test/test.exe
```

#### With npm:

```
git clone https://github.com/aantron/repromise.git
cd repromise
npm install
npm run build
(cd test && npm install && npm run build)
node ./test/lib/js/test.js
```



<br/>

## Why?

Repromise is basically an effort to unify the Node API on JS and libuv with the
native [Lwt][lwt] ecosystem. Among other things, we want to quickly study a new
JavaScript-friendly Lwt core and new libuv-based event loop.

[lwt]: https://github.com/ocsigen/lwt



<br/>

## Status

This repository is in a **proof-of-concept** stage at the moment. We intend to
expand it to a full-blown implementation. However, for the time being, the repo
is *very* rough: there is a functioning skeleton, but the packaging isn't there,
the C FFI is rife with obvious memory leaks, etc. All to be fixed shortly; the
initial code was written just yesterday :p


[opam]: http://opam.ocaml.org/
[node]: https://nodejs.org/en/
[npm]: https://www.npmjs.com/
[libuv]: http://libuv.org/
[ctypes]: https://github.com/ocamllabs/ocaml-ctypes
[bs]: https://github.com/BuckleScript/bucklescript
[bsb-native]: https://github.com/bsansouci/bsb-native
[discord]: https://discordapp.com/invite/reasonml
