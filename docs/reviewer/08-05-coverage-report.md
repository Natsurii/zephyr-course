# Lesson 08-05 — Coverage Report

**Module:** 08 — Unit Testing
**Type:** Assignment · **Tag:** `l8-task2`

## Before you review this

You are answering one question: **do the numbers match the expected figures, and is
the report committed under `twister-out/coverage/`?**

Coverage is a proxy. It shows which lines executed, not whether the assertions were
any good, so it corroborates the previous lesson rather than replacing it. Its real
value here is arithmetic: a leftover `ztest_test_skip()` executes nothing and drags
the percentages down, so matching the expected numbers proves no test was skipped.

The mechanism is instrumentation. With `--coverage`, gcov counters are compiled in,
each run writes `.gcda` files, and gcovr turns them into HTML. `--coverage-basedir
app/modules/ring_buf` narrows the report to the module so framework code does not
dilute it.

## The rules, and why each one exists

**Rule 1 — The run uses the coverage flags and a gcov tool.** *Failure it prevents:*
an uninstrumented binary producing an empty or bogus report.

**Rule 2 — The report is scoped to the module.** `--coverage-basedir
app/modules/ring_buf`. *Failure it prevents:* a percentage dominated by Zephyr,
hiding the module.

**Rule 3 — Lines are 81.4% (35/43), functions 100.0% (7/7), branches 64.3% (9/14).**
*Failure it prevents:* skipped tests, which reduce all three.

**Rule 4 — `twister-out/coverage/` is committed.** *Failure it prevents:* a report
that only exists on the author's disk.

**Rule 5 — The uncovered lines are explained.** *Failure it prevents:* treating 81.4% as a defect when it is the spec's intended ceiling.

## Reading the tag

Rule 1 holds: the run used `--coverage --coverage-tool gcovr` with
`--gcov-tool` pointing at the SDK's `x86_64-zephyr-elf-gcov`, and the tests passed
8/8 while instrumented.

Rule 2 holds in the command's `--coverage-basedir app/modules/ring_buf`, and the
committed `coverage_summary.json` records `"root": "../app/modules/ring_buf"` with
exactly one file, `src/ring_buf.c` — no Zephyr sources padded the denominator.

Rule 3 holds to the decimal, from `coverage_summary.json`:

```json
"line_total": 43,       "line_covered": 35,     "line_percent": 81.4,
"function_total": 7,    "function_covered": 7,  "function_percent": 100.0,
"branch_total": 14,     "branch_covered": 9,    "branch_percent": 64.3
```

Functions at 100% (7/7) is the strongest signal: every public function was called,
so no test body is still skipping.

Rule 4 holds: eight files are committed under `twister-out/coverage/`
(`index.html`, `index.functions.html`, `index.css`, `index.js`, the per-file page,
plus `coverage.json`, `coverage_summary.json` and `coverage.log`).

Rule 5 holds on inspection of the red lines. All eight are failure guards the spec
never exercises:

| Lines | Guard | Test that would close it |
| --- | --- | --- |
| 23–25 | `rb_init` invalid capacity | `rb_init(0)`, `rb_init(17)` |
| 56–58 | `rb_pop` on empty (`-ENODATA`) | pop from empty buffer |
| 70–72 | `rb_peek` on empty | peek empty buffer |
| 74–76 | `rb_peek(NULL)` | `rb_peek(NULL)` |

Note line 52 is covered, because `test_pop_null_returns_einval` does call
`rb_pop(NULL)`; its sibling `peek(NULL)` is the unprotected one.

## Running the check yourself

```bash
west twister -T tests/ring_buf -p qemu_x86 \
    --coverage --coverage-tool gcovr \
    --coverage-basedir app/modules/ring_buf \
    --gcov-tool "$ZEPHYR_SDK_INSTALL_DIR/x86_64-zephyr-elf/bin/x86_64-zephyr-elf-gcov"
```

Then open `twister-out/coverage/index.html` and compare the three figures with Rule
3. A lower line or branch number usually means a skipped test; a lower function
number almost certainly does.

## Verdict

All five rules hold. The numbers match exactly, the scope is narrow, the report is
committed, and the gaps are the spec's own omissions rather than missing tests.

## If you want to go further

Branch coverage below line coverage is normal in guard-heavy code and is not by
itself a fault. The honest reading of 64.3% is that the spec chose depth over
breadth: it pins success paths and two specific errors, and leaves the routine
`-ENODATA`/invalid-capacity guards to future tests. `native_sim` again builds
nothing on macOS (POSIX arch is Linux-only), hence `qemu_x86`.