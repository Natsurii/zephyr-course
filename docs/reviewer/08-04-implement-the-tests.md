# Lesson 08-04 — Implement the Tests

**Module:** 08 — Unit Testing
**Type:** Assignment · **Tag:** `l8-task1`

## Before you review this

You are answering one question: **are all seven specified tests implemented, and do
they assert enough to fail when the behaviour is wrong?**

A test that passes on a broken implementation is worse than no test, because it
manufactures confidence. The principle is therefore assertion *depth*: assert the
whole outcome, including the state afterwards, so the test pins behaviour rather
than merely touching it.

Two mechanics matter first. A suite declares six things — name, predicate, setup,
before, after, teardown — and only `before` runs between tests, which is where
shared state is reset. The supplied `before` does `rb_init(4)`, so each test starts
from a clean buffer at capacity four.

## The rules, and why each one exists

**Rule 1 — All seven stubs are implemented; no `ztest_test_skip()` remains.**
*Failure it prevents:* a green run that tests nothing.

**Rule 2 — Each test asserts its return codes and values precisely.**
`zassert_equal` against `0`, `-ENOSPC`, `-EINVAL`, or the exact value.
*Failure it prevents:* accepting any non-zero code, or any value.

**Rule 3 — Failure paths also assert that state is unchanged.**
*Failure it prevents:* a rejected push that consumes a slot passing unnoticed.

**Rule 4 — Preconditions are asserted, not assumed.** The reinit test proves the
buffer is non-empty before reinitialising.
*Failure it prevents:* a no-op `rb_init` satisfying "clears state".

**Rule 5 — Post-conditions are asserted.** Emptiness after the final pop.
*Failure it prevents:* a pop that reads correctly but fails to consume.

**Rule 6 — The run reports eight passing and zero failed.** *Failure it prevents:*
arguing from source inspection instead of a result.

## Reading the tag

Rule 1 holds: seven bodies are filled and no `ztest_test_skip()` call remains.
`test_fresh_state` was supplied, making eight tests across three suites.

Rule 2 holds in each, and the codes are exact rather than "some error":

```c
zassert_equal(rb_push(99), -ENOSPC, "push on a full buffer must return -ENOSPC");
zassert_equal(rb_pop(NULL), -EINVAL, "pop(NULL) must return -EINVAL");
```

Rule 3 holds where it can — the rejected push also pins the count:

```c
zassert_equal(rb_count(), 4, "rejected push must not consume a slot");
```

Rule 4 is the subtle one, and it is present. Before reinitialising, the test
establishes that there is something to clear:

```c
zassert_equal(rb_push(99), 0, "push(99) must succeed");
zassert_equal(rb_count(), 1, "count must be 1 after the push");
zassert_equal(rb_init(4), 0, "re-init with capacity 4 must succeed");
zassert_true(rb_is_empty(), "Re-initialised buffer must be empty");
zassert_equal(rb_count(), 0, "Re-initialised buffer count must be 0");
```

Without the first two lines, an `rb_init` that returned `0` and did nothing would
pass.

Rule 5 holds in the FIFO test, which pops all three then asserts emptiness, and in
the single push/pop test.

## Running the check yourself

```bash
west twister -T tests/ring_buf -p qemu_x86
```

A pass ends `8 of 8 executed test cases passed (100.00%)`. To probe depth, break
the module deliberately — make `rb_push` return `0` without storing — and confirm
the FIFO test fails rather than the suite staying green.

`native_sim` is the documented platform but builds no configurations on macOS,
because it needs the Linux-only POSIX arch; `qemu_x86` is the fallback.

## Verdict

All six rules hold. Seven tests implemented, codes and values exact, pre- and
post-conditions asserted, and the run is 8/8.

## If you want to go further

`zassert_*` aborts its test, whereas `zexpect_*` continues and `zassume_*` skips —
useful when several independent facts should be reported together. Note
`CONFIG_ZTEST_SHUFFLE=y` in `prj.conf` randomises suite and test order, so a test
relying on the previous one's leftovers passes only by luck; the per-test `before`
hook is what makes shuffling safe.