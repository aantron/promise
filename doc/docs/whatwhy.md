---
id: WhatWhy
title: What & Why
---

<br/>

Repromise is a binding to JavaScript promises that is:

- **minimal**: Repromise objects are JavaScript promises, and
- **type-safe**: the Repromise library provides safe replacements for JavaScript's unsafe promise methods.

<br/>

## Reusing `Promise` objects

JS `Promise` objects are basically ok, so Repromise reuses them. This has two advantages:

- **interop**: any JS library that uses JS promises also uses Repromises, because they are the same thing, and
- **tooling** for `Promise` developed in the JS ecosystem can be used with Repromises.

See [Interop](Interop) for details.

<br/>

## Replacing `Promise` methods

JS `Promise` methods, such as `Promise.resolve`, have [behavior](DesignFAQ#why-are-js-promises-not-type-safe) that is incompatible with ReasonML's type system (and almost any other type system). So, Repromise provides its own safe functions, such as [`Repromise.resolve`](API#resolve).

<br/>

## Other differences

Since Repromise is changing the methods, we also have the opportunity to make some different design decisions, such as dropping rejection in favor of explicit error handling.

<br/>
