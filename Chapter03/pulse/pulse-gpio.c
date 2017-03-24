#define PULSE_GPIO_NAME "pulse-gpio"
#define pr_fmt(fmt) PULSE_GPIO_NAME ": " fmt

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>

#include "pulse.h"

struct pulse_gpio_priv {
	int num_pulses;
	struct {
		struct pulse_device *dev;
		int irq;
	} pulse[];
};

/*
 * IRQ handler
 */

static irqreturn_t irq_handler(int i, void *ptr, struct pt_regs *regs)
{
	struct pulse_device *pulse = (struct pulse_device *) ptr;

	BUG_ON(!ptr);

	pulse_event(pulse);

	return IRQ_HANDLED;
}

/*
 * Platform driver stuff
 */

static inline int sizeof_pulse_gpio_priv(int num_pulses)
{
        return sizeof(struct pulse_gpio_priv) +
	       (sizeof(struct pulse_device) * num_pulses);
}

static int pulse_gpio_probe(struct platform_device *pdev)
{
        struct device *dev = &pdev->dev;
        struct fwnode_handle *child;
	struct pulse_gpio_priv *priv;
        int count, ret;
        struct device_node *np;

	/* Get the number of defined pulse sources */
        count = device_get_child_node_count(dev);
        if (!count)
                return -ENODEV;

	/* Allocate private data */
        priv = devm_kzalloc(dev, sizeof_pulse_gpio_priv(count), GFP_KERNEL);
        if (!priv)
                return -ENOMEM;

        device_for_each_child_node(dev, child) {
		int irq, flags;
		struct gpio_desc *gpiod;
                const char *label, *trigger;
		struct pulse_device *new_pulse;

		/* Get the GPIO descriptor */
                gpiod = devm_get_gpiod_from_child(dev, NULL, child);
                if (IS_ERR(gpiod)) {
                        fwnode_handle_put(child);
                        ret = PTR_ERR(gpiod);
                        goto error;
                }
		gpiod_direction_input(gpiod);

                np = to_of_node(child);

		/* Get the GPIO's properties */
                if (fwnode_property_present(child, "label")) {
                        fwnode_property_read_string(child, "label", &label);
                } else {
                        if (IS_ENABLED(CONFIG_OF) && !label && np)
                                label = np->name;
                        if (!label) {
                                ret = -EINVAL;
                                goto error;
                        }
                }

		flags = 0;
                ret = fwnode_property_read_string(child, "trigger", &trigger);
		if (ret == 0) {
			if (strcmp(trigger, "rising") == 0)
				flags |= IRQF_TRIGGER_RISING;
			else if (strcmp(trigger, "fallng") == 0)
				flags |= IRQF_TRIGGER_FALLING;
			else if (strcmp(trigger, "both") == 0)
				flags |= IRQF_TRIGGER_RISING | \
					 IRQF_TRIGGER_FALLING;
			else {
				ret = -EINVAL;
				goto error;
			}
		}

		/* Register the new pulse device */
                new_pulse = pulse_device_register(label, dev);
                if (!new_pulse) {
                        fwnode_handle_put(child);
			ret = PTR_ERR(new_pulse);
                        goto error;
                }

		/* Is GPIO in pin IRQ capable? */
		irq = gpiod_to_irq(gpiod);
		if (irq < 0) {
			ret = irq;
			goto error;
		}

		/* Ok, now we can request the IRQ */
		ret = request_irq(irq, (irq_handler_t) irq_handler, flags,
				PULSE_GPIO_NAME, new_pulse);
		if (ret < 0)
			goto error;

		priv->pulse[priv->num_pulses].dev = new_pulse;
		priv->pulse[priv->num_pulses].irq = irq;
		priv->num_pulses++;
        }

	platform_set_drvdata(pdev, priv);

        return 0;

error:
	/* Unregister everything in case of errors */
        for (count = priv->num_pulses - 1; count >= 0; count--) {
		if (priv->pulse[count].dev)
			pulse_device_unregister(priv->pulse[count].dev);
		if (priv->pulse[count].irq && priv->pulse[count].dev)
			free_irq(priv->pulse[count].irq,
				 priv->pulse[count].dev);
	}

        return ret;
}

static int pulse_gpio_remove(struct platform_device *pdev)
{
	struct pulse_gpio_priv *priv = platform_get_drvdata(pdev);
        int i;

        for (i = 0; i < priv->num_pulses; i++) {
		if (priv->pulse[i].dev)
			pulse_device_unregister(priv->pulse[i].dev);
		if (priv->pulse[i].irq && priv->pulse[i].dev)
			free_irq(priv->pulse[i].irq,
				 priv->pulse[i].dev);
	}

        return 0;
}

static const struct of_device_id of_gpio_pulses_match[] = {
        { .compatible = "gpio-pulses", },
        { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, of_gpio_pulses_match);

static struct platform_driver pulse_gpio_driver = {
        .probe          = pulse_gpio_probe,
        .remove         = pulse_gpio_remove,
        .driver         = {
                .name   = PULSE_GPIO_NAME,
                .of_match_table = of_gpio_pulses_match,
        },
};

module_platform_driver(pulse_gpio_driver);
MODULE_AUTHOR("Rodolfo Giometti <giometti@hce-engineering.com>");
MODULE_DESCRIPTION("Pulse support for counting-board");
MODULE_LICENSE("GPL");
