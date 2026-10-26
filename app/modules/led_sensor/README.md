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

`callibration` is a required devicetree property fetched with
`DT_INST_PROP()`; it controls how many GPIO writes are performed per update.

## Wiring

- `app/CMakeLists.txt` appends the module to `ZEPHYR_EXTRA_MODULES`.
- `app/app.overlay` creates the `led_sensor` node with `compatible = "our,driver"`.
- `app/prj.conf` sets `CONFIG_OUR_DRIVER=y` and `CONFIG_SENSOR=y`.

## Build

```bash
west build -p always -b our_board app -- -DBOARD_ROOT=$(pwd)/app
```

Note: `BOARD_ROOT` must be the application directory — Zephyr appends
`/boards` itself. Each `our,driver` node creates its own device instance,
config and data struct from the single instantiation macro.