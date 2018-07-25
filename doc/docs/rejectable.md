---
id: RejectableAPI
title: Rejectable API
---

<br/>

In addition to the main API, [`Repromise`](API), Repromise also provides `Repromise.Rejectable`, which exposes the rejection support that is built into JS promises. This is mainly for writing bindings. Rejectable Repromises should be converted to normal Repromises as soon as possible using [`catch`](#catch).

Most of the functions in `Repromise.Rejectable` are similar to the ones in [`Repromise`](API), but there are two new important functions, [`rejected`](#rejected) and the already-mentioned [`catch`](#catch).

<br/>

## Type `Repromise.Rejectable.t`

```reason
type Repromise.Rejectable.t('a, 'e);
```

The type of promises that can be resolved with values of type `'a`, or rejected with errors of type `'e`. For example, a `Repromise.Rejectable.t(int, exn)` is a Repromise that can be pending, resolved with an `int`, or rejected with `exn`.

In fact, the normal `Repromise.t('a)` is defined as a rejectable promise that can never be rejected:

```reason
type Repromise.t('a) = Repromise.Rejectable.t('a, never);
```

where `never` is a type that has no objects (cannot be constructed). This is important in converting rejectable Repromises to normal Repromises in [`catch`](#catch).

<br/>

## `make`

```reason
make: unit => (Repromise.Rejectable.t('a, 'e), ('a => unit), ('e => unit))
```

Returns a tuple of a new pending promise, a function for resolving it, and a function for rejecting it:

```reason
let (p, resolve_p, reject_p) = Repromise.Rejectable.make();
reject_p(Failure("failed"));
```

<br/>

## `resolved`

```reason
resolved: 'a => Repromise.Rejectable.t('a, _)
```

Creates a promise that is already resolved with the given value:

```reason
let p = Repromise.Rejectable.resolved("Hello");
```

This is equivalent to

```reason
let (p, resolve_p, _) = Repromise.Rejectable.make();
resolve_p("Hello");
```

<br/>

## `rejected`

```reason
rejected: 'e => Repromise.Rejectable.t(_, 'e)
```

Creates a promise that is already rejected with the given value:

```reason
let p = Repromise.Rejectable.rejected(Failure("failed"));
```

This is equivalent to

```reason
let (p, _, reject_p) = Repromise.Rejectable.make();
reject_p(Failure("failed"));
```

<br/>

## `relax`

```reason
relax: Repromise.t('a) => Repromise.Rejectable('a, _)
```

Allows treating a normal Repromise, which is rejectable only with `never`, as rejectable with any type. In other words, an equivalent type signature for this function is:

```reason
relax: Repromise.Rejectable.t('a, never) => Repromise.Rejectable('a, _)
```

This is useful, for instance, if you would like to pass a list of normal Repromises and rejectable ones to [`race`](#race). Doing so without `relax` will not typecheck, because the normal Repromise is rejectable with `never`, and the rejectable one is rejectable with `exn`:

```reason
let ((p1: Repromise.t(int)), _) = Repromise.make();
let ((p2: Repromise.Rejectable.t(int, exn)), _, _) =
  Repromise.Rejectable.make();

Repromise.Rejectable.race([p1, p2]);   /* Type error */
```

It works with `relax`, however:

```reason
Repromise.Rejectable.race([Repromise.relax(p1), p2]);   /* Ok */
```

<br/>

## `catch`

```reason
catch:
  (('e1 => Repromise.Rejectable('a, 'e2)), Repromise.Rejectable('a, 'e1)) =>
    Repromise.Rejectable('a, 'e2)
```

Attaches a callback to the given promise. The callback is called after the promise is rejected:

```reason
let (p, _, reject_p) = Repromise.Rejectable.make();
p
|> Repromise.Rejectable.catch(error => {
  prerr_endline(error);
  Repromise.resolved();
})
|> ignore;

/* Causes "failed" to be printed to STDERR/the error log: */
reject_p("failed");
```

In the above example, the final promise returned by `catch`, which we just `ignore`, has type `Repromise.t(unit)`. So, this is how `catch` can convert a rejectable Repromise (`p`) to a normal one! Just finish the callback of `catch` with `Repromise.resolved(...)`, or otherwise return a normal Repromise from it.

A typical pattern is to convert success values and errors into `result`s:

```reason
let (p1, _, reject_p1) = Repromise.Rejectable.make();
let p2 =
  p1
  |> Repromise.Rejectable.map(value => Ok(value))
  |> Repromise.Rejectable.catch(error => Error(error));

/* Causes p2 to be *resolved* with Error("failed") */
reject_p1("failed");
```

<br/>

## `wait`

```reason
wait: (('a => unit), Repromise.Rejectable.t('a, _)) => unit
```

Like the normal [`Repromise.wait`](API#wait), but works on rejectable Repromises. If the Repromise does get rejected (instead of resolved), the callback is never called.

<br/>

## `map`

```reason
map:
  (('a => 'b), Repromise.Rejectable.t('a, 'e)) => Repromise.Rejectable.t('b, 'e)
```

Like the normal [`Repromise.map`](API#map), but works on rejectable Repromises. If the Repromise gets rejected (instead of resolved), the callback is not called, and the error is simply propagated to the final promise returned by `map`.

<br/>

## `andThen`

```reason
andThen:
  (('a => Repromise.Rejectable.t('b, 'e)), Repromise.Rejectable.t('a, 'e)) =>
    Repromise.Rejectable.t('b, 'e)
```

Like the normal [`Repromise.andThen`](API#then), but works on rejectable Repromises. If the Repromise gets rejected (instead of resolved), the callback is not called, and the error is propagated to the final promise returned by `andThen`.

<br/>

## `all`

```reason
all:
  list(Repromise.Rejectable.t('a, 'e)) => Repromise.Rejectable.t(list('a), 'e)
```

Like the normal [`Repromise.all`](API#all), but works on rejectable Repromises. If one of the argument promises is rejected, the final promise returned by `all` is rejected immediately, without waiting for the rest of the argument promises to be rejected or become resolved.

<br/>

## `race`

```reason
race: list(Repromise.Rejectable.t('a, 'e)) => Repromise.Rejectable('a, 'e)
```

Like the normal [`Repromise.race`](API#race), but for rejectable Repromises. If one of the argument promises is rejected, the final promise returned by `race` is rejected immediately.

<br/>

## `fromJsPromise`

```reason
fromJsPromise: Js.Promise.t('a) => Repromise.Rejectable.t('a, Js.Promise.error)
```

Converts a BuckleScript [`Js.Promise`](https://bucklescript.github.io/bucklescript/api/Js.Promise.html) to a Repromise. Since `Js.Promise` is a binding to JS promises, and [Repromises have the same representation](https://aantron.github.io/repromise/docs/Interop#representation), this function just returns the argument you pass to it &mdash; but with a different type.

<br/>

## `toJsPromise`

```reason
toJsPromise: Repromise.Rejectable.t('a, _) => Js.Promise.t('a)
```

Converts a Repromise to a BuckleScript [`Js.Promise`](https://bucklescript.github.io/bucklescript/api/Js.Promise.html). Since `Js.Promise` is a binding to JS promises, the [same warnings](https://aantron.github.io/repromise/docs/Interop#passing-promises-to-js) apply when using `toJsPromise`, as when passing Repromises directly to JS. In short, it is important that the nested values (`'a`) are not promises.

Otherwise, when `'a` is not a promise, Repromises and `Js.Promise`s have the same representation, so the function just returns the argument you pass to it.

<br/>
