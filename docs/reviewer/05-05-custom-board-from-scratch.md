# Lesson 05-05 — Custom Board from Template

**Module:** 05 — Custom Boards
**Type:** Assignment · **Tag:** `l5-task2` · **Status: repaired before submission**

## Before you review this

You are answering one question: **does this board print `Board Initialized` before
`main()` runs?** The word *before* is the whole assignment. Printing the line from
`main()` satisfies the eye but not the requirement — by then the application is
already running, so nothing has been shown about board bring-up.

The principle is the kernel's initialisation ladder. Zephyr does not jump to
`main()`; it walks a series of init levels in fixed order, and `main()` is merely
a thread started at the last of them:

```
PRE_KERNEL_1 → PRE_KERNEL_2 → POST_KERNEL → APPLICATION → main()
```

A function registered with `SYS_INIT` is compiled into the `.initlevel` section in
level order. To print before the application, you register at an early level, not
in the application.

## The rules, and why each one exists

**Rule 1 — A board exists, built from scratch.** *Failure it prevents:* leaning on
the donor directory so the "from scratch" part is untested.

**Rule 2 — The banner is emitted by board code, not by the app.** *Failure it
prevents:* the message following app edits or vanishing when the app changes.

**Rule 3 — Registration is at an early `SYS_INIT` level.** *Failure it prevents:*
a banner that prints after `main()` has begun, defeating Rule 2's purpose.

**Rule 4 — The board compiles the source.** *Failure it prevents:* an unused `.c`
file that is never linked, so the banner silently never appears.

**Rule 5 — `Kconfig.defconfig` is named correctly and keyed to this board.**
*Failure it prevents:* Zephyr ignoring board defaults entirely.

**Rule 6 — The board builds and the string is in the image.** *Failure it
prevents:* passing on source inspection when the binary lacks the message.

## Reading the tag

The board is `app/boards/my_board/`, its own directory (Rule 1).

The board registers the hook in `board_init.c`:

```c
static int my_board_init(void)
{
        printf("Board Initialized\n");
        return 0;
}

SYS_INIT(my_board_init, PRE_KERNEL_1, 0);
```

`PRE_KERNEL_1` is the earliest level available, so the line lands before
`main()` — Rules 2 and 3 hold by construction. The board compiles it through its
own `CMakeLists.txt`:

```cmake
zephyr_library()
zephyr_library_sources(board_init.c)
```

which satisfies Rule 4.

Rule 5 is worth dwelling on, because it was wrong before repair. The file had been
named `Kconfig.def_config` — with an underscore. Zephyr looks for
`Kconfig.defconfig`, so the file was never read; the board's defaults were dead.
It was renamed, and its guard changed from the inherited
`if BOARD_BLACKPILL_F401CE` to `if BOARD_MY_BOARD`, since a guard on the donor
symbol would not match this board.

## Running the check yourself

```bash
git worktree add /tmp/wt l5-task2 && cd /tmp/wt
west build -p always -b my_board app -d /tmp/b -DBOARD_ROOT=/tmp/wt/app
strings /tmp/b/zephyr/zephyr.elf | grep -x "Board Initialized"
```

A pass prints the string — Rule 6 is a property of the binary, not the sources.
To confirm the ordering rather than just presence, inspect the init table, where
the first entry runs first:

```bash
arm-none-eabi-objdump -s -j initlevel /tmp/b/zephyr/zephyr.elf | head -3
arm-none-eabi-nm /tmp/b/zephyr/zephyr.elf | grep my_board_init
```

The address in the first slot of `.initlevel` matches `my_board_init`.

## Verdict

All six rules hold. The banner is board-owned, registered at `PRE_KERNEL_1`,
linked, and present in the image ahead of `main()`.

## If you want to go further

`SYS_INIT`'s third argument orders functions *within* a level, so keep it `0`
unless order among peers genuinely matters. Printing at `PRE_KERNEL_1` also means
console output may not be buffered the way you expect during later boot; for
bring-up diagnostics that is fine, for anything chatty prefer `POST_KERNEL`.