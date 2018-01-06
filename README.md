# Repromise

**Repromise** is an asynchronous programming library that makes your Reason
code run everywhere: on the browser or in Node.js, and natively under Linux,
macOS, Windows:

```reason
let () = {
  ignore ({
    let%await fd   = Io.open_("test/test.re");
    let%await data = Io.read (~fd, ~length = 1024);
    print_endline(data);
    Promise.resolve(());
  });

  Io.run();
};
```

...the above compiles to both JavaScript and machine code!

Interoperability is key: on JavaScript, Repromise `Promise.t`s are exactly
JavaScript's familiar `Promise`s. For targeting native code, Repromise provides
an implementation with the same semantics.

The `Io` module is powered by libuv when targeting native code, and Node.js on
JavaScript. Either way, the underlying implementation is the same, because
Node.js is *itself* powered by libuv!

Repromise is actually capable of providing synchronous I/O functions as well, we
just haven't exposed them yet.

Finally, Repromise includes a small preprocessor so you can write the neat and
familiar `let%await` syntax as in the example above!



<br/>

## Why?

Repromise is basically a prototype of the future [Lwt][lwt]. We want to
quickly study a new JavaScript-friendly Lwt core and new libuv-based event loop.

[lwt]: https://github.com/ocsigen/lwt




<br/>

## Trying it out

At the moment, the repo contains a minimalist isomorphic promise library, and a
small demo/test that runs both directly on your system, and under Node. You're
welcome to play with the demo and all the other code and see what you can come
up with; join us discussing the repo on [Discord][discord], open an issue, or
send a PR!

The easiest way to get started is to clone the repo and use the `Makefile`:

```
git clone https://github.com/aantron/repromise.git
cd repromise
make deps
make
```

You'll see the test compile and run.

Before that, the commands check that you have [opam][opam], [Node.js][node], and
[NPM][npm] installed. They are best installed from your system's package
manager if missing.

Then, they use opam to install the main native dependencies, [libuv][libuv] and
[Ctypes][ctypes], and NPM to install [BuckleScript][bs] and
[bsb-native][bsb-native]. They also install a `refmt` from Reason `master`,
because that is needed for the `let%await ...` syntax.



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
