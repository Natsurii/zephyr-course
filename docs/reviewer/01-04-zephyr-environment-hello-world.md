# Lesson 01-04 — Zephyr Environment & Hello World

**Module:** 01 — What is Zephyr, Preparing Environment
**Type:** Assignment · **Tag:** none

## Before you review this

You are answering one question: **can this workspace build anything at all?**

There is no tag to inspect, because this lesson is about the environment itself. Its
output is not code but a working toolchain — which makes it the precondition for
every later chapter. If the environment is broken, a reviewer cannot distinguish a
missing feature from a build that never ran, and every later verdict becomes noise.

Reviewers do not audit the candidate's laptop. What you do instead is confirm the
environment is *externally evidenced*: the workspace is a fork, a manifest pins the
Zephyr revision, and a sample builds. Those three facts are checkable from here.

## The rules, and why each one exists

**Rule 1 — The project is a fork of the course repository.** *Failure it prevents:*
submissions in a tree the reviewer cannot correlate with the course.

**Rule 2 — The manifest pins a known Zephyr revision.** *Failure it prevents:*
"works on my machine" drifting as `main` moves under the candidate.

**Rule 3 — A sample builds and boots a banner.** *Failure it prevents:* a
half-installed SDK that only fails later, inside an assignment.

**Rule 4 — The lesson is counted as complete on the platform.** *Failure it
prevents:* skipping prerequisites and mis-attributing later failures.

## Reading the workspace

Rule 1 holds: `origin` is the candidate's own fork of the course repository, and
the course repo is the upstream.

Rule 2 holds — the manifest is pinned rather than floating:

```bash
west manifest --path     # deps/zephyr
cd deps/zephyr && git describe --tags
```

The revision is the version the course targets, so builds are reproducible.

Rule 3 is evidenced indirectly but convincingly: every tagged lesson that follows
builds, which cannot happen on a broken toolchain. The optional half of the lesson —
`native_sim` and the boot banner — is where the environment is thinner on this host;
see the note below.

## Running the check yourself

```bash
west --version
west manifest --path
west build -p always -b blackpill_f401ce app
```

A pass ends at a linked `zephyr.elf` with a memory report. `command not found` for
`west`, or a manifest that will not resolve, means Rule 2 or 3 is missing and later
verdicts should be treated cautiously.

## Verdict

All four rules hold. The environment is a pinned fork that builds, and no tag was
requested for this lesson.

## If you want to go further

This host cannot run `native_sim`: it needs the Linux-only POSIX arch, so it builds
zero configurations on macOS. `qemu_x86` is the substitute target used throughout
this book. Separately, the SDK here began as a partial install — `arm-zephyr-eabi`
present but `x86_64-zephyr-elf` absent — and the x86 toolchain was added with
`setup.sh -t x86_64-zephyr-elf`. If a `qemu_x86` build fails here with `C compiler
not found`, that is the missing piece, not the candidate's work.