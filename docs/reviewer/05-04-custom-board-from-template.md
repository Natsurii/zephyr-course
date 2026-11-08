# Lesson 05-04 — Custom Board form Template

**Module:** 05 — Custom Boards
**Type:** Assignment · **Tag:** `l5-task1`

## Before you review this

You are answering one question: **is there a real, complete board definition where
Zephyr looks for one?** "Complete" is doing the work in that sentence. A board is
not a DTS file that happens to compile; it is a directory of interlocking pieces —
identity, devicetree, default configuration, and a flashing recipe — and a board
missing any of them will surprise the next person who selects it.

The principle is that a board is *metadata*, not code. The lecture's method is
copy/rename: take a known-good board of the same SoC and family, rename it, and
adjust the deltas. That keeps the structure correct by inheritance, so review is
about presence and coherence rather than about authoring style.

## The rules, and why each one exists

**Rule 1 — The board lives under the app's `BOARD_ROOT`, in `boards/`.** Zephyr
resolves `BOARD_ROOT` and appends `/boards`.
*Failure it prevents:* a board that builds only from one machine, because it sits
somewhere only that machine knows about.

**Rule 2 — The directory identifies the board.** `board.yml` names the board and
its SoC; `our_board.yaml` carries the metadata Zephyr indexes.
*Failure it prevents:* a board Zephyr cannot enumerate, so `-b` reports it as
invalid.

**Rule 3 — A devicetree describes the hardware.** `our_board.dts` with the right
SoC and pin map.
*Failure it prevents:* a board that builds but drives the wrong pins.

**Rule 4 — Default configuration exists and is keyed to this board.** A
`*_defconfig` plus `Kconfig.defconfig` guarded by this board's symbol.
*Failure it prevents:* inherited defaults that silently apply because they were
copied from the donor board and never renamed.

**Rule 5 — A flashing recipe ships with it.** `board.cmake` plus, here, an
openocd config.
*Failure it prevents:* a board that builds but cannot be flashed.

**Rule 6 — The board actually builds.** *Failure it prevents:* everything above
existing but wired together wrongly.

## Reading the tag

Start at the location — Rule 1. The board is at `app/boards/our_board/`, and the
app's build is invoked with `-DBOARD_ROOT=<app>`, so Zephyr will find it.

Then check Rule 2 and Rule 3 by listing the directory. All eleven expected files
are present:

```
Kconfig.defconfig   Kconfig.our_board   board.cmake   board.yml
our_board.dts       our_board.yaml      our_board_defconfig
support/openocd.cfg doc/index.rst       doc/img/Blackpill_Pinout.jpg
doc/img/blackpill-v3.jpg
```

`board.yml` declares the board and its `stm32f401xe` SoC (Rule 2), `our_board.dts`
carries the pin map (Rule 3), `board.cmake` and `support/openocd.cfg` form the
flash recipe (Rule 5), and the defconfig and `Kconfig.our_board` supply defaults
(rule 4). Note what the filenames show about method: every one is the donor
board's name already replaced, which is precisely the copy/rename outcome the
lecture teaches.

## Running the check yourself

Rule 6 subsumes the rest — if the wiring is wrong, the build fails:

```bash
git worktree add /tmp/wt l5-task1 && cd /tmp/wt
west build -p always -b our_board app -DBOARD_ROOT=/tmp/wt/app
```

A pass ends in `Linking CXX executable zephyr/zephyr.elf` with a memory report.
An invalid board fails earlier at configure with `Invalid BOARD`, which is what
you would see if `board.yml` or the directory name were wrong.

## Verdict

All six rules hold. The board is complete, correctly placed, and builds.

## If you want to go further

The donor is a Blackpill F401CE, so the doc images and pinout are inherited rather
than newly drawn — normal for the copy/rename method and not a defect. The app is
built rather than `samples/hello_world`; both produce the same board validation,
and the app is the tree that owns this board.