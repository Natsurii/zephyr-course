# LED sensor driver module

Lecture 06 homework — a Zephyr *driver module* that exposes an on-board LED
through the standard Sensor API.

## Layout

```text
modules/led_sensor/
├── zephyr/module.yml                  # West module registration
├── CMakeLists.txt                     # Module root build directives
├── Kconfig                            # Module root config menu
├── include/our_drivers/our_driver.h   # Public C API (incl. extension API)
├── drivers/our_driver/
│   ├── our_driver.c                   # Driver implementation
│   ├── CMakeLists.txt                 # Build logic (CONFIG_OUR_DRIVER)
│   └── Kconfig                        # Config menu (DT_HAS_OUR_DRIVER_ENABLED)
└── dts/bindings/driver/our,driver.yaml # Binding for compatible "our,driver"
```

## Behaviour

| Call | Effect |
| --- | --- |
| `sensor_sample_fetch()` | samples the physical pin level into `dev->data` |
| `sensor_channel_get(SENSOR_CHAN_PROX)` | writes the sampled level through to the pin (on/off) and returns `0`/`1` |
| `sensor_channel_get(OUR_DRIVER_CHAN_TOGGLE_COUNT)` | returns the runtime toggle counter |
| `our_driver_toggle()` | Task 2 extension API: flips the LED and increments the counter kept in the data struct |
| `our_driver_set(dev, 0\|1)` | extension API: drives an explicit level and stores it as the new state |

`callibration` is a required devicetree property fetched with
`DT_INST_PROP()`; it controls how many GPIO writes are performed per update.

## Extension API (home task 2)

The Sensor API has no write entry point, so the module also publishes its own
API struct. `struct our_driver_api` embeds `struct sensor_driver_api` as its
first member and adds `toggle` and `set` hooks, which keeps the standard sensor
slots untouched while still letting the extension resolve through `dev->api`:

```c
#include <our_drivers/our_driver.h>

our_driver_toggle(dev);                       /* flips the LED, bumps counter */
our_driver_set(dev, 1);                       /* drives an explicit level */
sensor_channel_get(dev, (enum sensor_channel)OUR_DRIVER_CHAN_TOGGLE_COUNT, &val);
```

Both hooks write the pin first and only then update `data->sampled` and
`data->toggle_count`, so a failed GPIO write leaves the driver state consistent.
`sensorroot set` surfaces this on the serial console.
The app drives a heartbeat at `CONFIG_APP_HEARTBEAT_PERIOD_MS` (app `Kconfig`).

## Shell command (home task 7/1)

With `CONFIG_OUR_DRIVER_SHELL=y` the module registers the root command
`sensorroot`, so the driver can be exercised from a serial console:

```text
uart:~$ sensorroot info      device name and ready state
uart:~$ sensorroot fetch     calls sensor_sample_fetch()
uart:~$ sensorroot read      calls sensor_channel_get(SENSOR_CHAN_PROX) and prints it
uart:~$ sensorroot set 1     drives the LED via the extension API (0 = off, 1 = on)
```

`sensorroot set` wraps `our_driver_set()`, the extension-API counterpart of
`toggle()`. `SHELL_CMD_ARG` enforces the operand count (2 mandatory, 0
optional), and the handler rejects non-numeric or out-of-range values with
`shell_error()` before touching the device.

`fetch` must run before `read`, because `read` reports the value stored by the
last fetch. Both first check `device_is_ready()` and print an error otherwise.
The shell front end lives in `drivers/our_driver/our_driver_shell.c` and is
only compiled when `CONFIG_OUR_DRIVER_SHELL` is set.

## Wiring

- `app/CMakeLists.txt` appends the module to `ZEPHYR_EXTRA_MODULES`.
- `app/app.overlay` creates the `led_sensor` node with `compatible = "our,driver"`.
- `app/prj.conf` sets `CONFIG_OUR_DRIVER=y`, `CONFIG_SENSOR=y` and the shell
  options.

## Build

```bash
west build -p always -b our_board app -- -DBOARD_ROOT=$(pwd)/app
```

Note: `BOARD_ROOT` must be the application directory — Zephyr appends
`/boards` itself. Each `our,driver` node creates its own device instance,
config and data struct from the single instantiation macro.