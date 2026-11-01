#include <errno.h>

#include <our_drivers/our_driver.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* Device handle stays generic: the driver is picked from the devicetree. */
static const struct device *const led_sensor = DEVICE_DT_GET_ANY(our_driver);

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Report the errors that do not stop the heartbeat. */
static void log_err(const char *what, int err)
{
	LOG_ERR("%s failed (%d)", what, err);
}

int main(void)
{
	struct sensor_value level = {0};
	struct sensor_value count = {0};
	int ret;

	if (!device_is_ready(led_sensor)) {
		LOG_ERR("LED sensor device not ready");
		return 0;
	}

	while (1) {
		/*
		 * Task 2: flip the LED through the custom extension API. The
		 * driver owns the state and the toggle counter, so the
		 * application keeps no copy of its own.
		 */
		ret = our_driver_toggle(led_sensor);
		if (ret < 0) {
			log_err("toggle", ret);
		}

		/* Task 1: sample the pin, then read the channel. */
		ret = sensor_sample_fetch(led_sensor);
		if (ret < 0) {
			log_err("sample_fetch", ret);
		}

		if (sensor_channel_get(led_sensor, SENSOR_CHAN_PROX, &level) < 0) {
			log_err("channel_get(PROX)", ret);
		}

		ret = sensor_channel_get(led_sensor, (enum sensor_channel)OUR_DRIVER_CHAN_TOGGLE_COUNT,
					 &count);
		if (ret < 0) {
			log_err("channel_get(TOGGLE_COUNT)", ret);
		}

		LOG_INF("LED state: %s (level=%d, toggles=%d)", level.val1 ? "ON" : "OFF",
			level.val1, count.val1);

		k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);
	}

	return 0;
}