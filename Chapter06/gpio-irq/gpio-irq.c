#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/input.h>

/*
 * Defines
 */

#define NAME			KBUILD_BASENAME

struct keys_s {
	char *name;
	int gpio;
	int irq;
	int btn;
};

/*
 * Module's parameters
 */

static int debug;
module_param(debug, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(int, "Set to 1 to enable debugging messages");

static int ngpios;
static int gpios[2] = { -1 , -1 };
module_param_array(gpios, int, &ngpios, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(gpios, "Defines the GPIOs number to be used as a list of"
                 " numbers separated by commas.");

/* Logging stuff */
#define __message(level, fmt, args...)                                  \
                printk(level "%s: " fmt "\n" , NAME , ## args)

#define DBG(code)                                                       \
        do {                                                            \
                if (unlikely(debug)) do {                               \
                        code                                            \
                } while (0);                                            \
        } while (0)

#define info(fmt, args...)                                              \
        __message(KERN_INFO, fmt , ## args)

#define err(fmt, args...)                                               \
        __message(KERN_ERR, fmt , ## args)

#define dbg(fmt, args...)                                               \
        do {                                                            \
                if (unlikely(debug))                                    \
                        __message(KERN_DEBUG, fmt , ## args);           \
        } while (0)

/*
 * Global variables
 */

static struct input_dev *b_dev;

static struct keys_s keys[2] = {
	[0] = {
		.name = "0",
		.btn = KEY_0,
		.gpio = -1,
		.irq = -1,
	},

	[1] = {
		.name = "1",
		.btn = KEY_1,
		.gpio = -1,
		.irq = -1,
	},
};

/*
 * IRQ handler
 */

static irqreturn_t irq_handler(int i, void *ptr, struct pt_regs *regs)
{
	struct keys_s *key = (struct keys_s *) ptr;
	int status;

	/* Get the gpio status */
	status = !!gpio_get_value(key->gpio);
	dbg("IRQ on GPIO%d status=%d", key->gpio, status);

	/* Report the button event */
	input_report_key(b_dev, key->btn, status);
	input_sync(b_dev);

        return IRQ_HANDLED;
}

/*
 * Usage function
 */

static void usage(void)
{
	err("usage: insmod %s [ debug=1 ] gpios=gpio1#,gpio2#", NAME);
}

/*
 * Module stuff
 */

static int __init gpioirq_init(void)
{
	int i;
	int ret;

	/* Check the supplied GPIOs numbers */
	if (ngpios != 2) {
		usage();
		ret = -EINVAL;
		goto exit;
	}

	/* Request the GPIOs and then setting them up as needed */
	for (i = 0; i < 2; i++) {
		dbg("got GPIO%d", gpios[i]);

		/* Is the GPIO line free? */
		ret = gpio_request(gpios[i], NAME);
		if (ret) {
			err("unable to request GPIO%d\n", gpios[i]);
			goto free_gpios;
        	}
		keys[i].gpio = gpios[i];

		/* If so then setting it as input */
        	gpio_direction_input(gpios[i]);

                /* Is GPIO in pin IRQ capable? */
                ret = gpio_to_irq(gpios[i]);
		if (ret < 0) {
			err("GPIO%d is not IRQ capable\n", gpios[i]);
			ret = -EINVAL;
                        goto free_gpios;
                }
		keys[i].irq = ret;

		/* Then request the IRQ */
		ret = request_irq(keys[i].irq, (irq_handler_t) irq_handler,
				IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				NAME, &keys[i]);
                if (ret < 0) {
			err("unable to request IRQ%d for GPIO%d\n",
				keys[i].irq, keys[i].gpio);
			ret = -EINVAL;
                        goto free_gpios;
                }
		dbg("GPIO%d (key=\"%s\") mapped on IRQ %d",
			keys[i].gpio, keys[i].name, keys[i].irq);
	}

	/* Allocate the input device */
	b_dev = input_allocate_device();
	if (!b_dev) {
		err("cannot allocate memory");
		ret = -ENOMEM;
		goto free_gpios;
	}
	b_dev->evbit[0] = BIT_MASK(EV_KEY);
	b_dev->name = NAME;
	b_dev->dev.parent = NULL;
	b_dev->id.bustype = BUS_HOST;
	b_dev->id.vendor = 0x0001;
	b_dev->id.product = 0x0001;
	b_dev->id.version = 0x0001;

	/* Define the keys mapping */
	for (i = 0; i < 2; i++)
		set_bit(keys[i].btn, b_dev->keybit);
 
	/* Register the input device */
	ret = input_register_device(b_dev);
	if (ret) {
		err("cannot register input device");
		goto free_dev;
	}

        info("input GPIO IRQ module loaded");

        return 0;

free_dev:
	input_free_device(b_dev);

free_gpios:
	for ( ; i >= 0; i--) {
		if (keys[i].irq >= 0)
			free_irq(keys[i].irq, &keys[i]);
		if (keys[i].gpio >= 0)
			gpio_free(keys[i].gpio);
	}

exit:
	return ret;
}

static void __exit gpioirq_exit(void)
{
	int i;

	input_unregister_device(b_dev);

	for (i = 0; i < 2; i++) {
		dbg("freeing IRQ %d for GPIO%d...", keys[i].irq, keys[i].gpio);
		free_irq(keys[i].irq, &keys[i]);
		gpio_free(keys[i].gpio);
	}

        info("input GPIO IRQ module released");
}

module_init(gpioirq_init);
module_exit(gpioirq_exit);

MODULE_AUTHOR("Rodolfo Giometti <giometti@hce-engineering.com>");
MODULE_DESCRIPTION("input GPIO IRQ module");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");
