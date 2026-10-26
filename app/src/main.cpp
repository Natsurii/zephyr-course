#include <our_drivers/our_driver.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define SLEEP_TIME_MS 1000

/* Device handle stays generic: the driver is picked from the devicetree. */
static const struct device *const led_sensor = DEVICE_DT_GET_ANY(our_driver);

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
	struct sensor_value level;
	struct sensor_value count;
	bool led_state = true;

	if (!device_is_ready(led_sensor)) return 0;

	while (1) {
		/* Task 2: flip the LED through the custom extension API. */
		if (our_driver_toggle(led_sensor) < 0) return 0;

		/* Task 1: sample the pin, then read the channel. */
		if (sensor_sample_fetch(led_sensor) < 0) return 0;
		if (sensor_channel_get(led_sensor, SENSOR_CHAN_PROX, &level) < 0) return 0;

		if (sensor_channel_get(led_sensor, (enum sensor_channel)OUR_DRIVER_CHAN_TOGGLE_COUNT,
				       &count) < 0)
			return 0;

		led_state = !led_state;
		LOG_INF("LED state: %s (level=%d, toggles=%d)", level.val1 ? "ON" : "OFF",
			level.val1, count.val1);
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}