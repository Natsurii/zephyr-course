# Lesson 03-04 — Custom Menus with Kconfig

**Module:** 03 — Application Configuration with Kconfig
**Type:** Assignment · **Tag:** `l3-task1`

## Before you review this

You are answering one question: **does the Kconfig reproduce the specified menu
tree — same nesting, same gating, same bounds?**

Kconfig looks like a flat list of switches but is really a declaration language for
an option space. `menu`, `choice` and `if` exist so a large space stays navigable and
so an option can be hidden until it is meaningful. `range` belongs to that same
purpose: it bounds input where the user types it.

That is why the structure, not the symbol count, is correct or incorrect. A tree
with the right names in the wrong nesting places an option in front of a user who
should never see it. Concretely: options that only apply under *Advanced LED
settings* must be unreachable while that menu is off.

## The rules, and why each one exists

**Rule 1 — `LED Subsystem` is a top-level menu.** *Failure it prevents:* orphan
options at the root level.

**Rule 2 — Blink sleep time is a `choice`, defaulting to medium.** *Failure it
prevents:* a free-form sleep value where exactly one of three is wanted.

**Rule 3 — `Advanced LED settings` is a submenu.** *Failure it prevents:* advanced
tuning mixed into the top level.

**Rule 4 — Brightness is an int, default 100, bounded 0–100.** *Failure it prevents:*
a percentage that accepts 5000.

**Rule 5 — Fade duration is an int, default 500.** *Failure it prevents:* a missing
default the user must invent.

**Rule 6 — `Expert settings` is a nested submenu holding the two booleans.**
*Failure it prevents:* expert toggles sitting at the same level as ordinary ones.

**Rule 7 — Both booleans are gated behind the advanced menu.** *Failure it prevents:*
expert switches visible and settable while the feature they tune is off.

**Rule 8 — The tree is tagged `l3-task1`.** *Failure it prevents:* review of an
unfixed state.

## Reading the tag

Each rule maps to one construct in `app/Kconfig`, in order: `menuconfig MY_LED` (1),
`choice MY_LED_BLINK_SLEEP` with `default MY_LED_BLINK_SLEEP_MEDIUM` (2),
`menuconfig MY_LED_ADVANCED` (3), `config MY_LED_BRIGHTNESS` with `range 0 100`
(4), `config MY_LED_FADE_DURATION_MS` with `default 500` (5), and
`menu "Expert settings"` holding `MY_LED_DEBUG` and `MY_LED_CUSTOM_PATTERN` (6).

Rule 7 is the structural one, and it is the wrapping block:

```kconfig
if MY_LED_ADVANCED
...
menu "Expert settings"
...
endmenu # Expert settings
endif # MY_LED_ADVANCED
```

The `if` encloses the expert menu rather than merely preceding it, so `menuconfig`
omits those symbols while the parent is off. Every symbol is `MY_LED_`-prefixed so
nothing collides with Zephyr's own `LED`/`LED_*` Kconfig.

Rule 8 holds — the tag exists.

## Running the check yourself

```bash
git worktree add /tmp/wt l3-task1 && cd /tmp/wt
west build -t menuconfig -b blackpill_f401ce app
```

Walk it: the top-level entry reads `LED Subsystem`, entering it shows the sleep
choice with `1s (medium)` selected, and the expert switches appear only after you
switch *Advanced LED settings* on. Toggle the parent off and confirm they vanish —
that is Rule 7. Try `5000` for brightness and confirm it refuses to exceed 100.

## Verdict

All eight rules hold. The nesting, the gating and the bounds match the brief, and
the tree is tagged.

## If you want to go further

Nothing in C consumes these symbols yet; the lesson is purely declarative and the
next one wires the period into code. If you review Module 03 in isolation that is
expected — do not fail it for unused symbols. Also note the prompt text `1s
(medium)` comes from the choice item's string, so the wording is part of the
specification, not decoration.