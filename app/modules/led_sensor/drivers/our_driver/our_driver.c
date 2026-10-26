/*
 * LED sensor driver
 *
 * Treats an on-board LED as a sensor:
 *   - sensor_sample_fetch() samples the physical pin level and stores it
 *   - sensor_channel_get()  returns the sampled level (SENSOR_CHAN_PROX) and
 *                           drives the LED on/off accordingly
 *
 * The custom extension API our_driver_toggle() flips the LED and bumps a
 * counter kept in the mutable data struct.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <our_drivers/our_driver.h>

#define DT_DRV_COMPAT our_driver

LOG_MODULE_REGISTER(our_driver, CONFIG_SENSOR_LOG_LEVEL);

/* Immutable settings, built from the devicetree node at compile time. */
struct our_driver_config {
	struct gpio_dt_spec led;
	uint32_t callibration;
};

/* Runtime mutable state, one instance per devicetree node. */
struct our_driver_data {
	/* Last level sampled by sample_fetch(), 1 = LED on. */
	uint8_t sampled;
	/* Number of toggles performed through our_driver_toggle(). */
	uint32_t toggle_count;
};

/* Private helpers */

static inline int our_driver_internal_write(const struct device *dev, bool on)
{
	const struct our_driver_config *cfg = dev->config;
	uint32_t i;
	int ret;

	/*
	 * "callibration" is the number of writes performed per update. It
	 * models a chip that needs a register burst rather than a single
	 * store, and shows how a DTS property reaches the driver.
	 */
	for (i = 0; i < MAX(cfg->callibration, 1U); i++) {
		ret = gpio_pin_set_dt(&cfg->led, on ? 1 : 0);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}

static inline int our_driver_internal_read(const struct device *dev, uint8_t *level)
{
	const struct our_driver_config *cfg = dev->config;
	int ret;

	ret = gpio_pin_get_dt(&cfg->led);
	if (ret < 0) {
		return ret;
	}

	*level = (uint8_t)ret;
	return 0;
}

/* Standard sensor API */

static int our_driver_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	struct our_driver_data *data = dev->data;

	/* The LED is a single channel device: every fetch samples the pin. */
	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_PROX) {
		return -ENOTSUP;
	}

	return our_driver_internal_read(dev, &data->sampled);
}

static int our_driver_channel_get(const struct device *dev, enum sensor_channel chan,
				  struct sensor_value *val)
{
	const struct our_driver_data *data = dev->data;
	int ret;

	if (val == NULL) {
		return -EINVAL;
	}

	switch (chan) {
	case SENSOR_CHAN_PROX:
		/* Drive the LED to the level that was just sampled. */
		ret = our_driver_internal_write(dev, data->sampled != 0U);
		if (ret < 0) {
			return ret;
		}

		val->val1 = data->sampled ? 1 : 0;
		val->val2 = 0;
		return 0;
	case OUR_DRIVER_CHAN_TOGGLE_COUNT:
		val->val1 = (int32_t)data->toggle_count;
		val->val2 = 0;
		return 0;
	default:
		return -ENOTSUP;
	}
}

/*
 * Custom extension API, routed through the standard attr_get slot so the
 * driver stays a plain sensor device. Application code calls the public
 * our_driver_toggle() wrapper below.
 */
static int our_driver_attr_get(const struct device *dev, enum sensor_channel chan,
			       enum sensor_attribute attr, struct sensor_value *val)
{
	struct our_driver_data *data = dev->data;
	int ret;

	ARG_UNUSED(chan);

	if (val == NULL) {
		return -EINVAL;
	}

	switch (attr) {
	case SENSOR_ATTR_GAIN:
		/* Toggle the LED and report the new toggle counter. */
		ret = our_driver_internal_write(dev, data->sampled == 0U);
		if (ret < 0) {
			return ret;
		}

		data->sampled = (data->sampled == 0U) ? 1U : 0U;
		data->toggle_count++;

		val->val1 = (int32_t)data->toggle_count;
		val->val2 = 0;
		return 0;
	default:
		return -ENOTSUP;
	}
}

int our_driver_toggle(const struct device *dev)
{
	struct sensor_value val;

	return sensor_attr_get(dev, SENSOR_CHAN_PROX, SENSOR_ATTR_GAIN, &val);
}

/* Registered API table: standard sensor entry points plus the custom one. */
static DEVICE_API(sensor, our_driver_api) = {
	.sample_fetch = our_driver_sample_fetch,
	.channel_get = our_driver_channel_get,
	.attr_get = our_driver_attr_get,
};

static int our_driver_init(const struct device *dev)
{
	const struct our_driver_config *cfg = dev->config;
	struct our_driver_data *data = dev->data;

	if (!gpio_is_ready_dt(&cfg->led)) {
		LOG_ERR("LED GPIO %s not ready", cfg->led.port->name);
		return -ENODEV;
	}

	/* Start from a known state: sampled level mirrors the pin, LED off. */
	if (our_driver_internal_read(dev, &data->sampled) < 0) {
		return -EIO;
	}

	data->toggle_count = 0;

	LOG_DBG("\"%s\" ready (callibration=%u)", dev->name, cfg->callibration);
	return 0;
}

#define OUR_DRIVER_DEFINE(inst)                                                                    \
	static struct our_driver_data data_##inst;                                                 \
	static const struct our_driver_config cfg_##inst = {                                       \
		.led = GPIO_DT_SPEC_INST_GET(inst, gpios),                                         \
		.callibration = DT_INST_PROP(inst, callibration),                                  \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, our_driver_init, NULL, &data_##inst, &cfg_##inst, POST_KERNEL,  \
			      CONFIG_SENSOR_INIT_PRIORITY, &our_driver_api)

DT_INST_FOREACH_STATUS_OKAY(OUR_DRIVER_DEFINE)