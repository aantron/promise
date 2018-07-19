---
id: QuickStart
title: Quick Start
---

## Trying

Clone the [sample project][example-bsb] for a minimal working setup:

```sh
git clone https://github.com/aantron/repromise-example-bsb.git
cd repromise-example-bsb
npm install
npm run test
```

[`main.re`][main.re] contains the interesting code:

```reason
let (p, resolve_p) = Repromise.make();

p
|> Repromise.map(s => s ++ ", world!")
|> Repromise.wait(print_endline);

resolve_p("Hello");
```

Then, head to the [API docs](API). Each function starting from [`wait`](API#wait) has examples. You can paste the code blocks into `main.re`, and rerun `npm run test` to see the output.

There is also a [sample project for bindings][example-binding], which shows how to use Repromise with [`node-fetch`][node-fetch] to do some real-world I/O. To learn more about creating bindings, see [Interop](Interop).

<br/>

## Installing in an existing project

Make sure `bs-platform` is installed. After that,

```sh
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

[example-bsb]: https://github.com/aantron/repromise-example-bsb#readme
[main.re]: https://github.com/aantron/repromise-example-bsb/blob/master/main.re
[example-binding]: https://github.com/aantron/repromise-example-binding#readme
[node-fetch]: https://www.npmjs.com/package/node-fetch
