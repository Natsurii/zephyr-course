# Lesson 07-05 — Extended Sensor Driver Shell

**Module:** 07 — Shell Subsystem
**Type:** Assignment · **Tag:** `l7-task2`

## Before you review this

You are answering one question: **can a user type a bad argument and get a
`shell_error` back, instead of a wrong action or a crash?**

A shell argument is untrusted input. The lesson's principle is that arity and value
are *different* checks, and each belongs to the mechanism built for it. Arity is
declarative, so `SHELL_CMD_ARG` handles it before your code runs — its two trailing
counters are mandatory and optional operands, and the shell refuses a mismatch.
Value is semantic, so the handler parses and range-checks, reporting through
`shell_error`.

That split matters: a missing operand never reaches the handler at all, so arity
enforcement comes free and cannot be forgotten. Only the value check is yours to
write.

## The rules, and why each one exists

**Rule 1 — The extension API is exposed as a subcommand taking one operand.**
*Failure it prevents:* an unreachable driver verb.

**Rule 2 — The arity counters enforce exactly one operand.**
`SHELL_CMD_ARG(..., 2, 0)`: one mandatory word (the subcommand) plus one mandatory
operand, zero optional.
*Failure it prevents:* a missing operand being treated as a value.

**Rule 3 — A missing operand is rejected by the shell, not the handler.**
*Failure it prevents:* an uninitialised or empty `argv[1]` reaching a parser.

**Rule 4 — The value is parsed with a checked parser, not `atoi`.**
`shell_strtoul(argv[1], 10, &err)`. *Failure it prevents:* `abc` parsing as `0`,
which is a legal value and so passes silently.

**Rule 5 — Non-numeric and out-of-range inputs each get a `shell_error`.**
*Failure it prevents:* a silent no-op the user cannot diagnose.

**Rule 6 — The parsed width is narrowed only after the range check.**
*Failure it prevents:* truncation before validation, so a huge number wraps into a
"valid" one.

## Reading the tag

The registration carries the counters of Rule 2 — `2, 0`:

```c
SHELL_CMD_ARG(set, NULL,
              "Set the LED level through the extension API: set <0|1>",
              cmd_sensorroot_set, 2, 0),
```

Because the count is two, `sensorroot set` with no operand is rejected by
`SHELL_CMD_ARG` itself (Rules 2 and 3), and `sensorroot set 1 2` likewise.

Parsing is checked, into a wide type (Rules 4 and 6):

```c
unsigned long value;
int err = 0;

value = shell_strtoul(argv[1], 10, &err);
if (err == -ERANGE) {
        shell_error(sh, "\"%s\" is too large, expected a number", argv[1]);
        return -ERANGE;
} else if (err < 0) {
        shell_error(sh, "\"%s\" is not a number", argv[1]);
        return err;
}
```

`err` distinguishes overflow from garbage, so the two messages differ. Note the
declared type is `unsigned long`, and the cast to `uint8_t` happens only here,
after validation:

```c
if (value > 1UL) {
        shell_error(sh, "%lu out of range, expected 0 (off) or 1 (on)", value);
        return -EINVAL;
}
...
ret = our_driver_set(shell_led_sensor, (uint8_t)value);
```

The range is checked on the wide value, so `300` is rejected rather than wrapping
to `44`. Every failure path goes through `shell_error` (Rule 5).

The driver repeats the range check and returns `-EINVAL` for direct API callers,
and like `toggle` it drives the pin before mutating state.

## Running the check yourself

```bash
git worktree add /tmp/wt l7-task2 && cd /tmp/wt
west build -p always -b blackpill_f401ce app -DBOARD_ROOT=/tmp/wt/app
```

On the console, walk the table:

| Input | Expected |
| --- | --- |
| `sensorroot set` | shell arity error (handler not entered) |
| `sensorroot set 1 2` | shell arity error |
| `sensorroot set abc` | `"abc" is not a number` |
| `sensorroot set 2` | `2 out of range, expected 0 (off) or 1 (on)` |
| `sensorroot set 1` | `LED set to 1 (ON)`, LED lights |

The first row is the one to watch closely: the message comes from the shell, which
is how you know Rule 2 and not just Rule 5 is in place.

## Verdict

All six rules hold. Arity is enforced declaratively, values are parsed and checked
before narrowing, and both bad-input classes report through `shell_error`.

## If you want to go further

`shell_strtoul` rejects a leading `-`, so `set -1` reports "not a number" rather
than underflowing to a huge unsigned value. If negative levels ever become
meaningful, switch to `shell_strtol`. Also resist putting the range check in the
driver alone — the two layers serve different callers, the shell for humans and the
driver for API users, and dropping either leaves one of them unguarded.