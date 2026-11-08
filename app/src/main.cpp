/*
 * Configurable heartbeat LED.
 *
 * Works on any board: the LED comes from the app-led alias in app.overlay,
 * which points at the board's led0, and the blink period comes from
 * CONFIG_APP_HEARTBEAT_PERIOD_MS (change it in menuconfig and rebuild).
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* The devicetree node identifier for the "app-led" alias from app.overlay. */
#define LED_NODE DT_ALIAS(app_led)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
	bool led_state = true;

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED GPIO not ready");
		return 0;
	}

	if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
		LOG_ERR("LED GPIO configure failed");
		return 0;
	}

	LOG_INF("Heartbeat on %s every %d ms", led.port->name, CONFIG_APP_HEARTBEAT_PERIOD_MS);

	while (1) {
		if (gpio_pin_toggle_dt(&led) < 0) {
			LOG_ERR("LED toggle failed");
			return 0;
		}

		led_state = !led_state;
		LOG_INF("LED state: %s", led_state ? "ON" : "OFF");

		/* Period comes from Kconfig, so menuconfig changes the blink rate. */
		k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);
	}

	return 0;
}