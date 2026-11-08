# Lesson 09-03 — Secure Boot Demo

**Module:** 09 — Security
**Type:** Demo · **Optional** · **Tag:** none

## What this is

A text lesson accompanying the live security lecture. It supplies a QEMU walkthrough
of secure boot: a valid image, a tampered image, and a wrong-key image, run to show
the chain of trust accepting and rejecting each in turn.

The surrounding lecture frames why this matters — device security obligations, what
Zephyr provides (isolation, crypto and keys, the vulnerability process,
`hardenconfig`), and how secure boot is anchored in hardware.

## What a reviewer should look at

Nothing graded. Module 09 has no assignment, so the demo produces no tag.

It is still worth knowing as context: `hardenconfig` and the isolation features are
things a reviewer may notice in later submission trees, and the demo shows what the
failures look like.

## Why it has no tag

A demonstration is not an assignment. The module is three lessons — lecture video,
slides, and this demo — with no task attached to any of them.
