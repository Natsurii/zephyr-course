# Lesson 02-04 — LED Blinky

**Module:** 02 — West, West topologies & Hello World
**Type:** Assignment · **Tag:** `l2-task1`

## Before you review this

You are answering one question: **does this blink an LED once per second through the
standard GPIO API, without hardcoding a board?**

West owns build, flash and debug; the lesson's code uses it to reach hardware for
the first time. The pattern it establishes is the one every later driver repeats:
resolve a node by alias, validate it, then drive it through a typed spec rather than
a raw port and pin number.

The alias matters more than it looks. `DT_ALIAS(led0)` names an *indirection*, so
the application asks for "the board's LED" and the board decides which pin that is.
A node path would glue this file to one board.

## The rules, and why each one exists

**Rule 1 — The LED is driven with the typed GPIO API.** `gpio_pin_toggle_dt` on a
`gpio_dt_spec`. *Failure it prevents:* raw port/pin arithmetic that skips
devicetree.

**Rule 2 — The node is reached by alias.** `DT_ALIAS(led0)`.
*Failure it prevents:* board-specific paths in application code.

**Rule 3 — The device is validated before use.** A readiness check and a configure
return-check, both guarded. *Failure it prevents:* toggling an unconfigured pin and
faulting instead of returning.

**Rule 4 — The period is one second.** `SLEEP_TIME_MS` of 1000.
*Failure it prevents:* a blink rate that does not match the brief.

**Rule 5 — GPIO is enabled in configuration.** `CONFIG_GPIO=y`.
*Failure it prevents:* the API compiling out.

**Rule 6 — The work is tagged `l2-task1`.** *Failure it prevents:* a submission with
no reviewable state.

## Reading the tag

Rule 1 holds on the call itself, which takes the spec rather than three integers:

```c
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
...
gpio_pin_toggle_dt(&led);
```

Rule 2 holds at the top, against the board alias:

```c
#define LED_NODE DT_ALIAS(led0)
```

Rule 3 holds as two guarded returns before the loop, so a bad GPIO reports instead
of faulting:

```c
if (!gpio_is_ready_dt(&led)) return 0;
if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) return 0;
```

Rule 4 holds with `#define SLEEP_TIME_MS 1000` under `k_msleep(SLEEP_TIME_MS)`.
Rule 5 holds in `prj.conf`, which also enables `CONFIG_LOG` for the state messages.
Rule 6 holds — the tag exists.

## Running the check yourself

```bash
git worktree add /tmp/wt l2-task1 && cd /tmp/wt
west build -p always -b blackpill_f401ce app -DBOARD_ROOT=/tmp/wt/app
```

A pass links `zephyr.elf`. On hardware, the LED toggles at roughly one hertz and the
log alternates `LED state: ON` / `OFF`. To check Rule 4 by inspection rather than by
eye, confirm `SLEEP_TIME_MS` is 1000 and is the only argument to the sleep.

## Verdict

All six rules hold. The blink is API-driven, alias-resolved, guarded, one second,
and GPIO is enabled.

## If you want to go further

The optional `native_sim` half of the lesson was not run here, since the POSIX arch
does not build on macOS; the expected banner is `*** Booting Zephyr OS build
v4.2.0 ***`. Note that `board.cmake` for a stock board provides flashing, but no
`west flash` was performed — this book reviews build correctness, not silicon.

One naming wrinkle: this tag was first pushed to the remote misspelled as
`2-task1`. It is now pushed correctly as `l2-task1`, so link the latter.