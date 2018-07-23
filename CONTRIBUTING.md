# Contributing to Repromise

Please ask any questions, [open issues](https://github.com/aantron/repromise/issues/new), etc. All of these are valued contributions :) This file is meant to help specifically with working on the code of Repromise.

The repo has [three main pieces of code](#ProjectStructure):

1. The main, compile-to-JS Repromise, for BuckleScript.
2. A native implementation of Repromise.
3. An `async/await` syntax.

Only (1) is released, so all the instructions here are for working on (1). As the other stuff gets polished up for initial release, this file will be expanded with information about those parts of Repromise as well :)

<br/>

#### Table of contents

- [Quick start: `repromise.re`](#QuickStart)
- [Running tests](#RunningTests)
- [Project structure](#ProjectStructure)

<br/>

<a id="QuickStart"></a>
## Quick start: `repromise.re`

On BuckleScript, Repromise consists of only two files (boilerplate aside): [`repromise.re`][repromise.re], and its interface [`repromise.rei`][repromise.rei]. So, the minimal workflow is:

1. Clone Repromise and set up for development:

    ```
    git clone https://github.com/aantron/repromise.git
    cd repromise
    npm install bs-platform
    ```

2. Open [`src/js/repromise.re`][repromise.re] and edit away!

3. Send a PR to the Repromise repo.

<br/>

<a id="RunningTests"></a>
## Running tests

The tests are in a separate local NPM package in [`test/`](https://github.com/aantron/repromise/tree/master/test). To run them, first do this once:

```
cd test
npm install
```

Then, each time you want to run the tests,

```
npm run tests
```

<br/>

<a id="ProjectStructure"></a>
## Project structure

First, the boilerplate. Repromise is packaged for three package manager/build system combinations:

1. NPM+bsb, for **JS**: [`package.json`][package.json] and [`bsconfig.json`](https://github.com/aantron/repromise/blob/master/bsconfig.json).
2. opam+Dune, for **native**: [`repromise.opam`](https://github.com/aantron/repromise/blob/master/repromise.opam) and the various `jbuild` files.
3. NPM+esy+Dune, also for **native**: `jbuild` files, [`package.json`][package.json], and [`esy.json`](https://github.com/aantron/repromise/blob/master/esy.json).

Only (1) is released.

Typically, `.rei` files (interfaces) are included in both the JS and native packages, but `.re` files (implementations) are different for JS and native.

The important files and directories are:

- [`src/js`](https://github.com/aantron/repromise/tree/master/src/js): Repromise for BuckleScript/JS.

- [`src/native`](https://github.com/aantron/repromise/tree/master/src/native): Repromise for Dune/native.

- [`src/ppx/bucklescript`](https://github.com/aantron/repromise/tree/master/src/ppx/bucklescript): BuckleScript/JS-friendly implementation of the `async`/`await` syntax.

- [`src/ppx/omp`](https://github.com/aantron/repromise/tree/master/src/ppx/omp): the same `async`/`await` syntax, but for native.

- [`test/framework`](https://github.com/aantron/repromise/tree/master/test/framework): custom test framework, ported from Lwt.

- [`test`](https://github.com/aantron/repromise/tree/master/test): tests that are the same for JS and native. We try to put most tests in this category, to ensure that the implementations remain compatible.

- [`test/js`](https://github.com/aantron/repromise/tree/master/test/js): JS-specific tests. These are especially concerned with passing Repromises to JS, and interactions between JS promises and Repromises.

- [`test/native`](https://github.com/aantron/repromise/tree/master/test/native): native-only tests. These are especially concerned with memory leaks, as naive implementations of promises leak memory with common usage patterns.

[repromise.re]: https://github.com/aantron/repromise/blob/master/src/js/repromise.re
[repromise.rei]: https://github.com/aantron/repromise/blob/master/src/js/repromise.rei
[package.json]: https://github.com/aantron/repromise/blob/master/package.json
