/*
 * LED sensor driver - shell front end
 *
 * Exposes the driver from the Sensor API through a `sensorroot` root shell
 * command so the driver can be poked from a serial console without an app:
 *
 *   sensorroot fetch  -> sensor_sample_fetch()
 *   sensorroot read   -> sensor_channel_get() on SENSOR_CHAN_PROX
 *   sensorroot info   -> device name and ready state
 *   sensorroot set    -> our_driver_set() extension API (task 2)
 *
 * Argument counts are enforced by SHELL_CMD_ARG; values are range checked here.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_string_conv.h>
#include <zephyr/sys/util.h>

#include <our_drivers/our_driver.h>

/* Same handle as the app: resolved from the devicetree, kept generic. */
static const struct device *const shell_led_sensor = DEVICE_DT_GET_ANY(our_driver);

/* Shared check: report why the device cannot be used and return its state. */
static bool shell_device_usable(const struct shell *sh)
{
	if (shell_led_sensor == NULL) {
		shell_error(sh, "no \"our,driver\" node in the devicetree");
		return false;
	}

	if (!device_is_ready(shell_led_sensor)) {
		shell_error(sh, "\"%s\" is not ready", shell_led_sensor->name);
		return false;
	}

	return true;
}

/* fetch: sample the pin into the driver data. */
static int cmd_sensorroot_fetch(const struct shell *sh, size_t argc, char **argv)
{
	int ret;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (!shell_device_usable(sh)) {
		return -ENODEV;
	}

	ret = sensor_sample_fetch(shell_led_sensor);
	if (ret < 0) {
		shell_error(sh, "sensor_sample_fetch failed (%d)", ret);
		return ret;
	}

	shell_print(sh, "sampled \"%s\"", shell_led_sensor->name);
	return 0;
}

/* read: call channel_get() and print the value it reports. */
static int cmd_sensorroot_read(const struct shell *sh, size_t argc, char **argv)
{
	struct sensor_value val = {0};
	int ret;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (!shell_device_usable(sh)) {
		return -ENODEV;
	}

	ret = sensor_channel_get(shell_led_sensor, SENSOR_CHAN_PROX, &val);
	if (ret < 0) {
		shell_error(sh, "sensor_channel_get(PROX) failed (%d)", ret);
		shell_error(sh, "run \"sensorroot fetch\" first");
		return ret;
	}

	shell_print(sh, "SENSOR_CHAN_PROX = %d (val2=%d), LED %s", val.val1, val.val2,
		    val.val1 ? "ON" : "OFF");
	return 0;
}

/* info: device name and ready state. */
static int cmd_sensorroot_info(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (shell_led_sensor == NULL) {
		shell_print(sh, "device    : <none>");
		shell_print(sh, "ready     : false");
		return -ENODEV;
	}

	shell_print(sh, "device    : %s", shell_led_sensor->name);
	shell_print(sh, "ready     : %s", device_is_ready(shell_led_sensor) ? "true" : "false");

	return 0;
}

/* set: drive an explicit level through the driver extension API. */
static int cmd_sensorroot_set(const struct shell *sh, size_t argc, char **argv)
{
	unsigned long value;
	int err = 0;
	int ret;

	/*
	 * SHELL_CMD_ARG already rejected a missing or surplus operand, so argv[1]
	 * is present. It can still be non-numeric or out of range.
	 */
	value = shell_strtoul(argv[1], 10, &err);
	if (err == -ERANGE) {
		shell_error(sh, "\"%s\" is too large, expected a number", argv[1]);
		return -ERANGE;
	} else if (err < 0) {
		shell_error(sh, "\"%s\" is not a number", argv[1]);
		return err;
	}

	if (value > 1UL) {
		shell_error(sh, "%lu out of range, expected 0 (off) or 1 (on)", value);
		return -EINVAL;
	}

	if (!shell_device_usable(sh)) {
		return -ENODEV;
	}

	ret = our_driver_set(shell_led_sensor, (uint8_t)value);
	if (ret < 0) {
		shell_error(sh, "our_driver_set(%lu) failed (%d)", value, ret);
		return ret;
	}

	shell_print(sh, "LED set to %lu (%s)", value, value ? "ON" : "OFF");
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensorroot,
			       SHELL_CMD_ARG(fetch, NULL, "Sample the sensor (sensor_sample_fetch)",
					     cmd_sensorroot_fetch, 1, 0),
			       SHELL_CMD_ARG(read, NULL,
					     "Read SENSOR_CHAN_PROX (sensor_channel_get)",
					     cmd_sensorroot_read, 1, 0),
			       SHELL_CMD_ARG(info, NULL, "Print device name and ready state",
					     cmd_sensorroot_info, 1, 0),
			       /* 1 mandatory ("set") + 1 operand, 0 optional. */
			       SHELL_CMD_ARG(set, NULL,
					     "Set the LED level through the extension API: set <0|1>",
					     cmd_sensorroot_set, 2, 0),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(sensorroot, &sub_sensorroot, "LED sensor driver control", NULL);