# Lesson 06-04 — Base Sensor Driver

**Module:** 06 — Zephyr Driver Development
**Type:** Assignment · **Tag:** `l6-task1`

## Before you review this

You are answering one question: **is this a driver that Zephyr can discover, build
and call through the standard Sensor API, purely from devicetree?** Nothing in the
review is about how clever the hardware handling is; it is about whether the
plumbing that makes a driver a *Zephyr* driver exists.

The principle is that a driver's identity comes from two declarations that must
agree: the binding says which devicetree nodes are ours, and the code names that
binding with `DT_DRV_COMPAT`. From those, a macro generates one device instance
per matching enabled node. Get the pair right and instances, config and data
appear without hand-written registration.

## The rules, and why each one exists

**Rule 1 — Drivers are C, not C++.** The lecture states this outright.
*Failure it prevents:* the module failing to compile once it is pulled in.

**Rule 2 — The module is a module.** A `zephyr/module.yml` that registers the
module and sets `dts_root`, so bindings are discovered, plus a module `Kconfig`
and `CMakeLists.txt`.
*Failure it prevents:* a driver on disk that the build never sees.

**Rule 3 — A binding defines the compatible and its required properties.**
`dts/bindings/driver/our,driver.yaml` with `compatible: "our,driver"` and a
required `callibration`.
*Failure it prevents:* nodes selected by guesswork, and properties read from the
tree that the binding never promised.

**Rule 4 — The code names the compatible.** `#define DT_DRV_COMPAT our_driver`,
with the comma of the compatible becoming an underscore. *Failure it prevents:*
`DT_INST_*` macros expanding against the wrong binding.

**Rule 5 — Instantiation is generated, not typed.** One
`DT_INST_FOREACH_STATUS_OKAY` macro emits config, data and a
`DEVICE_DT_INST_DEFINE` per enabled node.
*Failure it prevents:* per-board hand-editing, and instances that exist for
disabled nodes.

**Rule 6 — Both standard entry points are implemented.**
`sensor_sample_fetch` reads the hardware into the mutable data;
`sensor_channel_get` converts that state into a `sensor_value` and drives the LED.
*Failure it prevents:* a device that registers but cannot be used; the Sensor API
has no partial contract.

**Rule 7 — Kconfig is gated on the hardware being present.**
`depends on DT_HAS_OUR_DRIVER_ENABLED`, selecting `GPIO` and `SENSOR`.
*Failure it prevents:* the driver building on boards that have no such node.

## Reading the tag

Rule 1 holds: `our_driver.c` is C.

Rule 2 holds through the module layout — `zephyr/module.yml` registers the module
and declares `dts_root`, so the binding directory is picked up, and the module
carries its own `Kconfig` and `CMakeLists.txt`.

Rule 3 holds at the binding: the compatible is `"our,driver"` and `callibration`
is a required property, so a node missing it fails at build time rather than
silently reading zero.

Rule 4 holds at the top of the source:

```c
#define DT_DRV_COMPAT our_driver
```

`our,driver` → `our_driver` is the required transformation; the comma cannot
appear in a C identifier.

Rule 5 holds at the bottom, where a single macro does the instantiating:

```c
#define OUR_DRIVER_DEFINE(inst)                                     \
        static struct our_driver_data data_##inst;                  \
        static const struct our_driver_config cfg_##inst = { ... }; \
        DEVICE_DT_INST_DEFINE(inst, our_driver_init, NULL,          \
                              &data_##inst, &cfg_##inst,            \
                              POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &our_driver_api)

DT_INST_FOREACH_STATUS_OKAY(OUR_DRIVER_DEFINE)
```

Config is `const`, data is mutable and separate from it — the lecture's split.

Rule 6 holds in the API body: `our_driver_sample_fetch()` samples the pin into
`data->sampled`, and `our_driver_channel_get()` reports it through
`SENSOR_CHAN_PROX`, writing the sampled level back out to the pin.

Rule 7 holds in the module `Kconfig`: `depends on DT_HAS_OUR_DRIVER_ENABLED`, with
`select GPIO` and `select SENSOR`.

## Running the check yourself

Discovery is the interesting failure mode, and it needs no board:

```bash
git worktree add /tmp/wt l6-task1 && cd /tmp/wt
west build -p always -b blackpill_f401ce app -DBOARD_ROOT=/tmp/wt/app
```

Then confirm the device instance exists rather than assuming it:

```bash
arm-none-eabi-nm /tmp/b/zephyr/zephyr.elf | grep -i our_driver
```

You should see the generated `data_0`/`cfg_0` and the device in `.device_states`.
Remove `app.overlay`'s node and rebuild to watch the instance disappear — that is
Rule 5 doing its job.

## Verdict

All seven rules hold. The driver is discoverable, instantiated from devicetree,
implements both standard entry points, and is correctly gated.

## If you want to go further

The `callibration` property is a teaching device, not a hardware need: it sets how
many GPIO writes a single update performs, so the property visibly reaches the
driver without a bus. Real drivers replace it with actual register or bus
parameters. Also note `CONFIG_SENSOR_INIT_PRIORITY` at `POST_KERNEL`: a driver
must not init at an early level, since GPIO dependencies may not be ready.