---
id: API
title: API
---

<br/>

Most of the functions on this page have examples. The easiest way to try them is to clone the [Repromise example repo](https://github.com/aantron/repromise-example-bsb), and paste the code into `main.re`.

<br/>

## Type `Repromise.t`

```reason
type Repromise.t('a);
```

The type of promises that can be resolved with values of type `'a`. For example, an object of type `Repromise.t(int)` is a Repromise that is either pending (doesn't yet have a value), or resolved with an `int`.

<br/>

## `make`

```reason
make: unit => (Repromise.t('a), ('a => unit))
```

Returns a tuple of a new pending promise, and a function for resolving it:

```reason
let (p, resolve_p) = Repromise.make();
```

`p` starts out pending. After calling `resolve_p`,

```reason
resolve_p("Hello");
```

`p` is resolved with `"Hello"`. Any callbacks that have been attached to `p` by [`wait`](#wait), [`map`](#map), [`andThen`](#andThen), [`all`](#all), or [`race`](#race), get called, and receive `"Hello"` as their argument. See examples in the rest of the documentation below.

Each promise can only be resolved once, so calling `resolve_p` again has no effect.

<br/>

## `resolved`

```reason
resolved: 'a => Repromise.t('a)
```

Returns a promise that starts out resolved, with the given value:

```reason
let p = Repromise.resolved("Hello");
```

This is basically the same as

```reason
let (p, resolve_p) = Repromise.make();
resolve_p("Hello");
```

<br/>

## `wait`

```reason
wait: (('a => unit), Repromise.t('a)) => unit
```

Attaches a callback to the promise, which will be called after that promise is resolved:

- If the promise is already resolved, the callback is called on the next tick (almost immediately):

    ```reason
    let p = Repromise.resolved("Hello");
    p |> Repromise.wait(text => print_endline(text));
    /* Prints "Hello". */
    ```

- If the promise is pending and gets resolved later, the callback is called at that time:

    ```reason
    let (p, resolve_p) = Repromise.make();
    p |> Repromise.wait(text => print_endline(text));
    /* Doesn't print anything yet! */

    Js.Global.setTimeout(() => resolve_p("Hello"), 1000) |> ignore;
    /* Prints "Hello" after one second. */
    ```

- If the promise is pending, but never gets resolved, the callback is never called.

Callbacks are always asynchronous. Even if a callback is queued on a promise that is already resolved, it is run later, on the next tick. This can lead to surprising execution orders:

```reason
Repromise.resolved()
|> Repromise.wait(() => print_endline("printed second"));

print_endline("printed first");
```

The `wait` call only schedules its callback to run later, on the next tick. The `print_endline` below `wait` is executed immediately, however.

<br/>

## `map`

```reason
map: (('a => 'b), Repromise.t('a)) => Repromise.t('b)
```

Transforms the value in a promise, once the promise is resolved:

```reason
let (p, resolve_p) = Repromise.make();

p
|> Repromise.map(text => text ++ ", world!")
|> Repromise.wait(print_endline);
/* None of the callbacks have been called yet. */

resolve_p("Hello");
/* Prints "Hello, world!" */
```

<br/>

## `andThen`

```reason
andThen: (('a => Repromise.t('b)), Repromise.t('a)) => Repromise.t('b)
```

Like [`map`](#map), but the return value of the callback is another promise. This allows the callback to start an async operation:

```reason
let (p1, resolve_p1) = Repromise.make();
Js.Global.setTimeout(() => resolve_p1(), 1000) |> ignore;

p1
|> Repromise.andThen(() => {
  print_endline("Hello");

  let (p2, resolve_p2) = Repromise.make();
  Js.Global.setTimeout(() => resolve_p2(), 1000) |> ignore;
  p2
})
|> Repromise.wait(() => {
    print_endline("world!");
});
/* Prints "Hello" after one second, then "world!" after another second. */
```

<br/>

## `all`

```reason
all: list(Repromise.t('a)) => Repromise.t(list('a))
```

Creates a promise which resolves when **all** the given promises resolve:

```reason
let (p1, resolve_p1) = Repromise.make();
let (p2, resolve_p2) = Repromise.make();

Repromise.all([p1, p2])
|> Repromise.wait(texts =>
  texts |> String.concat(", ") |> print_endline);

resolve_p1("Hello");
resolve_p2("world!");
/* Prints "Hello, world!" */
```

<br/>

## `all2` &mdash; `all6`

```reason
all2: (Repromise.t('a), Repromise.t('b)) => Repromise.t(('a, 'b))
/* etc. */
```

These work the same way as [`all`](#all), but the argument promises can be
resolved with different types.

<br/>

## `race`

```reason
race: list(Repromise.t('a)) => promise('a)
```

Creates a promise which resolves when **one** of the given promises resolves:

```reason
let (p1, resolve_p1) = Repromise.make();
let (p2, resolve_p2) = Repromise.make();

Repromise.race([p1, p2])
|> Repromise.wait(print_endline);

resolve_p1("Hello");
resolve_p2("world!");
/* Prints only "Hello". */
```

<br/>

## `onUnhandledException`

```reason
onUnhandledException: ref(exn => never)
```

When a callback called by Repromise (such as what you pass to [`wait`](#wait)) raises an exception, the exception is passed to `Repromise.onUnhandledException^`.

When your program starts, `Repromise.onUnhandledException` contains a function which prints the exception to the error log (on JavaScript) or STDERR (on native). Replace the reference to change this behavior.

In the future, we may want to design a wiser approach to asynchronous exception handling. See https://github.com/aantron/repromise/issues/16 for a discussion.

<br/>
