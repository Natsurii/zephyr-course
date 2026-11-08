# Lesson 04-04 — Application Level Overlay

**Module:** 04 — Application Configuration with Devicetree
**Type:** Assignment · **Tag:** `l4-task1` · **Status: repaired before submission**

## Before you review this

You are checking one thing: **does the blink rate come from Kconfig or from a
number typed into C?** Everything else in this file serves that question. If the
period is a literal, the assignment is not done, no matter how good the overlay
looks.

The reason is a rule that holds for all of Zephyr: configuration that a user is
meant to change must cross a boundary the compiler respects. Kconfig writes
`CONFIG_*` macros during configure; C either reads those macros or it does not.
There is no third case.

## The rules, and why each one exists

Each rule below is followed by the failure it prevents. Read the failure, then
check whether the tree in front of you exhibits it.

**Rule 1 — Hardware location goes in Devicetree.** The lecture tabulates Kconfig
as "Do I want this feature?" and Devicetree as "Where is this hardware?"
*Failure it prevents:* a pin number written into C, which breaks the moment the
board changes.

**Rule 2 — Reach a node by alias.** The lecture lists alias, node label, path in
that order and annotates the alias "preferred — stable API".
*Failure it prevents:* binding the app to `led0`, which is a board-specific
alias, instead of the app's own `app-led` indirection.

**Rule 3 — Overlays patch the board tree.** `app.overlay` is the application
fallback for exactly this. *Failure it prevents:* having no place to declare
`app-led` at all.

**Rule 4 — A policy value is a Kconfig int, defaulted and bounded.** Here:
`APP_HEARTBEAT_PERIOD_MS`, int, default 500, range 100–2000.
*Failure it prevents:* an unbounded or hidden period that a user cannot find or
cannot trust.

**Rule 5 — C sleeps for the macro.** `k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS)`.
*Failure it prevents:* Rule 4's symbol becoming dead configuration.

**Rule 6 — The check is falsifiable.** Change the value, rebuild, and the blink
must change. *Failure it prevents:* passing review on file presence alone.

## Reading the tag

Start with what is present. `app.overlay` defines `app-led`, and `app/Kconfig`
declares the int with the correct default and range — Rules 1 through 4 are
satisfied on inspection.

Now read `main.cpp`, and watch for Rule 5's failure:

```c
#define SLEEP_TIME_MS 1000            /* a literal — Rule 5 broken */
#define LED_NODE DT_ALIAS(led0)       /* board alias — Rule 2 broken */
...
k_msleep(SLEEP_TIME_MS);
```

`SLEEP_TIME_MS` expands to 1000. The macro `CONFIG_APP_HEARTBEAT_PERIOD_MS`
appears nowhere, so the symbol has no consumer. Because a literal cannot react
to Kconfig, Rule 6 also fails — and note the ordering: Rule 5's failure *entails*
Rule 6's. That is why one literal is enough to fail the whole assignment even
though two files are correct.

## The repair

```c
#define LED_NODE DT_ALIAS(app_led)                       /* Rule 2, 5 */
...
k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);                /* Rule 5 */
```

Squashed into the `l4-task1` commit, so the tag stands alone.

## Running the check yourself

Never take a reviewer's word for Rule 6 — it is mechanical. Build twice, the
second time with an override, and diff the emitted instruction:

```bash
git worktree add /tmp/wt l4-task1 && cd /tmp/wt
west build -p always -b blackpill_f401ce app -d /tmp/b1
west build -p always -b blackpill_f401ce app -d /tmp/b2 \
    -DCONFIG_APP_HEARTBEAT_PERIOD_MS=1800
arm-none-eabi-objdump -d /tmp/b1/zephyr/zephyr.elf | grep 'mov.w\s*r3, #'
```

You should see the immediate change with the override:

```
b1: mov.w r3, #500    @ 0x1f4
b2: mov.w r3, #1800   @ 0x708
```

The operand follows the config, so the sleep duration is genuinely derived and
Rule 6 holds. Also try the bound: overriding `5000` warns and lands on 500,
because `range 100 2000` clamps it — Rule 4 is a bound, not decoration.

## Verdict

All six rules hold at the tag, Rule 6 proven by disassembly.

## If you want to go further

`app-led` points at the board's `user_led` label rather than another alias,
because an alias cannot target an alias — this is the lecture's "by node label"
form, not a defect. `l4-task1` predates the custom board, so verify against
`blackpill_f401ce`; `our_board` does not exist until `l5-task1`.