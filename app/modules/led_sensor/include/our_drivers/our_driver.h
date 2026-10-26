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
 * clash with the generic sensor channel enum.
 */
enum our_driver_channel {
	/** Number of times the LED was toggled by the driver. Raw count. */
	OUR_DRIVER_CHAN_TOGGLE_COUNT = SENSOR_CHAN_PRIV_START,
};

/**
 * @brief Toggle the LED and bump the toggle counter in the runtime data.
 *
 * Used by @ref our_driver_channel_get for OUR_DRIVER_CHAN_TOGGLE_COUNT.
 *
 * @param dev Pointer to the device.
 * @return 0 on success, negative errno on failure.
 */
int our_driver_toggle(const struct device *dev);

#ifdef __cplusplus
}
#endif

#endif /* OUR_DRIVERS_OUR_DRIVER_H */