/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * Copyright (c) 2024 Veniamin Milevski
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#define SLEEP_TIME_MS	1

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE	DT_ALIAS(sw0)
#define SW1_NODE	DT_ALIAS(sw1)
#define SW2_NODE	DT_ALIAS(sw2)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
#if !DT_NODE_HAS_STATUS(SW1_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#if !DT_NODE_HAS_STATUS(SW2_NODE, okay)
#error "Unsupported board: sw2 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
							      {0});
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET_OR(SW1_NODE, gpios,
							      {0});
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET_OR(SW2_NODE, gpios,
							      {0});
static struct gpio_callback button_cb_data;
static struct gpio_callback button1_cb_data;
static struct gpio_callback button2_cb_data;

/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
						     {0});
static struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios,
						     {0});
static struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios,
						     {0});

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}
void button1_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}
void button2_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&button)) {
		printk("Error: button device %s is not ready\n",
		       button.port->name);
		return 0;
	}
	if (!gpio_is_ready_dt(&button1)) {
		printk("Error: button device %s is not ready\n",
		       button1.port->name);
		return 0;
	}
	if (!gpio_is_ready_dt(&button2)) {
		printk("Error: button device %s is not ready\n",
		       button2.port->name);
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return 0;
	}
	ret = gpio_pin_configure_dt(&button1, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button1.port->name, button1.pin);
		return 0;
	}
	ret = gpio_pin_configure_dt(&button2, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button2.port->name, button2.pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button.port->name, button.pin);
		return 0;
	}
	ret = gpio_pin_interrupt_configure_dt(&button1,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button1.port->name, button1.pin);
		return 0;
	}
	ret = gpio_pin_interrupt_configure_dt(&button2,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button2.port->name, button2.pin);
		return 0;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	printk("Set up button at %s pin %d\n", button.port->name, button.pin);

	gpio_init_callback(&button1_cb_data, button1_pressed, BIT(button1.pin));
	gpio_add_callback(button1.port, &button1_cb_data);
	printk("Set up button at %s pin %d\n", button1.port->name, button1.pin);

	gpio_init_callback(&button2_cb_data, button2_pressed, BIT(button2.pin));
	gpio_add_callback(button2.port, &button2_cb_data);
	printk("Set up button at %s pin %d\n", button2.port->name, button2.pin);

	if (led.port && !gpio_is_ready_dt(&led)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led.port->name);
		led.port = NULL;
	}
	if (led1.port && !gpio_is_ready_dt(&led1)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led1.port->name);
		led1.port = NULL;
	}
	if (led2.port && !gpio_is_ready_dt(&led2)) {
		printk("Error %d: LED device %s is not ready; ignoring it\n",
		       ret, led2.port->name);
		led2.port = NULL;
	}
	if (led.port) {
		ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led.port->name, led.pin);
			led.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
		}
	}
	if (led1.port) {
		ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led1.port->name, led1.pin);
			led1.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led1.port->name, led1.pin);
		}
	}
	if (led2.port) {
		ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led2.port->name, led2.pin);
			led2.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led2.port->name, led2.pin);
		}
	}

	printk("Press the button\n");

	while (1) {
		if (led.port) {
			/* If we have an LED, match its state to the button's. */
			int val = gpio_pin_get_dt(&button);
			if (val >= 0) {
				gpio_pin_set_dt(&led, val);
			}
		}

		if (led1.port) {
			int val = gpio_pin_get_dt(&button1);
			if (val >= 0) {
				gpio_pin_set_dt(&led1, val);
			}
		}

		if (led2.port) {
			int val = gpio_pin_get_dt(&button2);
			if (val >= 0) {
				gpio_pin_set_dt(&led2, val);
			}
		}

		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
