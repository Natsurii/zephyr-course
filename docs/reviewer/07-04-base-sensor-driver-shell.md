# Lesson 07-04 — Base Sensor Driver Shell

**Module:** 07 — Shell Subsystem
**Type:** Assignment · **Tag:** `l7-task1`

## Before you review this

You are answering one question: **can the driver be driven from a serial console
through a `sensorroot` command with exactly the three named subcommands?**

The principle is that the shell is a *tree*, and a tree needs a root. Commands hang
off a root node at level 0; each child is registered as an array terminated by a
sentinel. Registration is static, so the whole tree exists at link time. Because
the root is an array too, forgetting the sentinel is not a warning but a dangling
read.

Handlers all share one signature and return an `int`. Output goes through the
`shell_*` print family — `shell_print`, `shell_warn`, `shell_error` — never
`printf`, because the shell owns the console.

## The rules, and why each one exists

**Rule 1 — A single root command named `sensorroot`.** *Failure it prevents:*
subcommands scattered at top level, unpredictable to type.

**Rule 2 — Exactly three subcommands: `fetch`, `read`, `info`.** *Failure it
prevents:* scope drift, and a reviewer unable to match the brief.

**Rule 3 — Each does what its name says through the standard API.**
`fetch` → `sensor_sample_fetch()`; `read` → `sensor_channel_get()` printing the
result; `info` → device name and ready state.
*Failure it prevents:* a command that reports something other than the API value.

**Rule 4 — The subcommand array ends with `SHELL_SUBCMD_SET_END`.** *Failure it
prevents:* the shell reading past the array.

**Rule 5 — Handlers use `shell_*` print functions and return an `int`.** *Failure
it prevents:* interleaved console output and an unusable return code.

**Rule 6 — The shell code is optional in the build.** *Failure it prevents:* the
shell pulled into production images that do not want it.

## Reading the tag

The root and its three children are declared together (Rules 1 and 2):

```c
SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensorroot,
        SHELL_CMD_ARG(fetch, NULL, "Sample the sensor (sensor_sample_fetch)", cmd_sensorroot_fetch, 1, 0),
        SHELL_CMD_ARG(read,  NULL, "Read SENSOR_CHAN_PROX (sensor_channel_get)", cmd_sensorroot_read, 1, 0),
        SHELL_CMD_ARG(info,  NULL, "Print device name and ready state", cmd_sensorroot_info, 1, 0),
        SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(sensorroot, &sub_sensorroot, "LED sensor driver control", NULL);
```

The sentinel is present (Rule 4), and the root is registered against the set.

Rule 3 holds in the bodies: `fetch` calls `sensor_sample_fetch`, `read` calls
`sensor_channel_get(..., SENSOR_CHAN_PROX, &val)` and prints both `val1` and
`val2`, and `info` prints `dev->name` and the result of `device_is_ready()`:

```c
shell_print(sh, "device    : %s", shell_led_sensor->name);
shell_print(sh, "ready     : %s", device_is_ready(shell_led_sensor) ? "true" : "false");
```

Both read paths first reject an unusable device through a shared helper, so a
missing or unready device reports rather than faults.

Rule 5 holds: every handler takes `(const struct shell *sh, size_t argc, char
**argv)`, returns `int`, and prints only via `shell_print`/`shell_error`. `read`
additionally hints `run "sensorroot fetch" first`, because it reports the value
stored by the last sample rather than sampling again.

Rule 6 holds in the build: the source is compiled only under
`CONFIG_OUR_DRIVER_SHELL`, an option that `depends on OUR_DRIVER` and `select
SHELL`.

## Running the check yourself

Build and look for the registered command symbols:

```bash
git worktree add /tmp/wt l7-task1 && cd /tmp/wt
west build -p always -b blackpill_f401ce app -DBOARD_ROOT=/tmp/wt/app
arm-none-eabi-nm /tmp/b/zephyr/zephyr.elf | grep sensorroot
```

A pass shows `shell_cmd_sensorroot`, `shell_sub_sensorroot` and the three
`cmd_sensorroot_*` handlers. Then on hardware, `sensorroot info` must print a name
and `ready : true`; `sensorroot fetch` followed by `sensorroot read` must report a
`SENSOR_CHAN_PROX` value.

## Verdict

All six rules hold. The root exists with exactly the three required children, each
calls the API its name promises, the sentinel and print conventions are correct,
and the code is build-optional.

## If you want to go further

`SHELL_CMD_ARG(..., 1, 0)` means one mandatory word (the subcommand itself) and
zero optional ones — the right arity for a command taking no operand. The next
lesson needs an operand, and you will see those counters change and the value
checked. Also note the shell task is single-threaded: keep handlers short, as these
are.