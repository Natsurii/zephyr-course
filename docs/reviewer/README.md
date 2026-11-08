# Reviewer's handbook

A chapter per lesson for *Zephyr Course: Fundamental (Cohort 2/2026)*, written to be
read in order by someone who has not reviewed embedded code before.

Each chapter opens with the single question the reviewer must answer, teaches the
principle behind it, then walks the submitted tag and shows how to verify the claim
independently. Chapters are ordered as the course presents them.

## How to read a chapter

Every chapter has the same shape:

- **Before you review this** — the one question, and the principle underneath it
- **The rules, and why each one exists** — each rule paired with the failure it catches
- **Reading the tag** — the evidence, walked in rule order
- **Running the check yourself** — commands, and what passing and failing look like
- **Verdict** — rule by rule
- **If you want to go further** — nuance that prevents a false failure

Two conventions throughout. Verdicts are earned by building the tag, never by
reading a commit message. And a rule is always taught before it is applied, so a
first-time reviewer learns the reasoning rather than a checklist.

## Tagged assignments

| Chapter | Tag | Status |
| --- | --- | --- |
| [01-04 Environment & Hello World](01-04-zephyr-environment-hello-world.md) | none | precondition only |
| [02-04 LED Blinky](02-04-led-blinky.md) | `l2-task1` | holds |
| [03-04 Custom Menus with Kconfig](03-04-custom-menus-with-kconfig.md) | `l3-task1` | holds |
| [04-04 Application Level Overlay](04-04-application-level-overlay.md) | `l4-task1` | **repaired before submission** |
| [05-04 Custom Board form Template](05-04-custom-board-from-template.md) | `l5-task1` | holds |
| [05-05 Custom Board from Template](05-05-custom-board-from-scratch.md) | `l5-task2` | **repaired before submission** |
| [06-04 Base Sensor Driver](06-04-base-sensor-driver.md) | `l6-task1` | holds |
| [06-05 Extended Sensor Driver](06-05-extended-sensor-driver.md) | `l6-task2` | holds |
| [07-04 Base Sensor Driver Shell](07-04-base-sensor-driver-shell.md) | `l7-task1` | holds |
| [07-05 Extended Sensor Driver Shell](07-05-extended-sensor-driver-shell.md) | `l7-task2` | holds |
| [08-04 Implement the Tests](08-04-implement-the-tests.md) | `l8-task1` | holds |
| [08-05 Coverage Report](08-05-coverage-report.md) | `l8-task2` | holds |

## Optional sessions

Live sessions are recorded and carry no tag. They are worth knowing about, and the
chapters say what (if anything) is worth opening.

| Chapter | Contents |
| --- | --- |
| [Live Meeting 1](live-meeting-1.md) | 1 lesson, meeting information |
| [Live Meeting 2](live-meeting-2.md) | 3 lessons, incl. demo code |
| [Live Meeting 3](live-meeting-3.md) | 3 lessons, incl. demo code |

## Course-level pages

| Chapter | Note |
| --- | --- |
| [09-03 Secure Boot Demo](09-03-secure-boot-demo.md) | demonstration, no tag |
| [Add Your GitHub Account](add-your-github-account.md) | prerequisite; **still incomplete** |
| [Final Assessment](final-assessment.md) | course-level, no tag |

## Before you start reviewing

Two things to settle first, because both invalidate tags otherwise:

1. **Identity.** The profile's Git Account field must hold the GitHub username, and
   the repository must be public. See [Add Your GitHub Account](add-your-github-account.md);
   it is currently unfilled.
2. **Reproduce, do not trust.** Build every tag in a throwaway worktree:

   ```bash
   git worktree add /tmp/wt <tag>
   cd /tmp/wt && west build -p always -b blackpill_f401ce app -DBOARD_ROOT=/tmp/wt/app
   ```

## Platform caveat

`native_sim` is the platform named in Modules 08's instructions, but it requires the
POSIX architecture and so builds only on Linux. On macOS it selects zero
configurations. Use `qemu_x86` instead, which needs the SDK's x86_64 toolchain:

```bash
ZEPHYR_SDK_INSTALL_DIR/…/setup.sh -t x86_64-zephyr-elf
```

Being unable to build `native_sim` on a Mac is not a candidate failure.