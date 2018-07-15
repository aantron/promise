---
id: Performance
title: Performance
---

The Repromise repo includes some [benchmarks](https://github.com/aantron/repromise/blob/master/test/js/benchmark.re) comparing Repromise to direct usage of JS promises. They can be run with

```
cd test
npm install
npm run benchmark
```

Running the benchmarks on Node.js on one machine gives results like this:

```
TEST                              TIME/CALL

Js.Promise.resolve(1)             19.9ns
Repromise.resolve(1)              20.7ns

Js.Promise.resolve(promise)        5.0ns
Repromise.resolve(promise)        43.9ns

Js.Promise.then                   23.1ns
Repromise.then                    56.6ns
```

These timings have the costs of allocation and garbage collection factored in.

So,

- The performance of [`Repromise.resolve`](API#resolve) on values that are not nested promises, such as `1` is almost the same as the performance of direct `Js.Promise.resolve`. This is because in this case, a Repromise is represented directly as a JS promise, with no extra allocations. The minor difference in timing is probably due to the cost of the dynamic check for a nested promise, which comes out negative.

- When given a nested promise, [`Repromise.resolve`](API#resolve) is slower than `Js.Promise.resolve`. However, this is because `Js.Promise.resolve` is [not type-safe](DesignFAQ#why-are-js-promises-not-type-safe), and simply returns its argument as its result. `Repromise.resolve`, instead, allocates two new objects: a new JS promise, and a [wrapper object](Interop#representation) to prevent the outer and inner promises from collapsing into each other. These two allocations are probably the reason why `Repromise.resolve` is up to about twice as slow on nested promises, as on non-promise values.

- [`Repromise.then_`](API#then) is about half as fast as `Js.Promise.then`. This is due to dynamic checks for nested promises, and due to [setting up asynchronous exception handling](API#onunhandledexception). However, the difference is not significant. `then_` is almost always used around I/O, which takes much longer than tens of nanoseconds to complete.
