/*
 * LED sensor driver - Public API
 *
 * A "sensor" that exposes the board LED: sensor_sample_fetch() samples the
 * physical pin level into the driver data, sensor_channel_get() returns that
 * level through SENSOR_CHAN_PROX and turns the LED on or off.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef OUR_DRIVERS_OUR_DRIVER_H
#define OUR_DRIVERS_OUR_DRIVER_H

#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Extra channels exposed by this driver, used with
 *        sensor_channel_get().
 *
 * Sensor-specific channels start at SENSOR_CHAN_PRIV_START so they cannot
 * clash with the generic sensor channel enum. C++ call sites must cast:
 *
 *   sensor_channel_get(dev, (enum sensor_channel)OUR_DRIVER_CHAN_TOGGLE_COUNT,
 *                      &val);
 */
enum our_driver_channel {
	/** Number of times the LED was toggled by the driver. Raw count. */
	OUR_DRIVER_CHAN_TOGGLE_COUNT = SENSOR_CHAN_PRIV_START,
};

/**
 * @brief Toggle the LED and bump the toggle counter in the runtime data.
 *
 * Task 2 extension API. Usually you only read the counter afterwards with
 * sensor_channel_get(dev, OUR_DRIVER_CHAN_TOGGLE_COUNT, &val), which keeps the
 * application on the standard Sensor API.
 *
 * @param dev Pointer to the device.
 * @return 0 on success, negative errno on failure. The state is left unchanged
 *         when the GPIO write fails.
 */
int our_driver_toggle(const struct device *dev);

/** @cond INTERNAL_HIDDEN */

/**
 * @brief Driver-specific API, stored in the device's api slot.
 *
 * The standard sensor API is embedded as the first member so a
 * struct our_driver_api * is also a valid struct sensor_driver_api *.
 */
struct our_driver_api {
	struct sensor_driver_api sensor;

	/** Toggle the LED; backs our_driver_toggle(). */
	int (*toggle)(const struct device *dev);
};

/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* OUR_DRIVERS_OUR_DRIVER_H */