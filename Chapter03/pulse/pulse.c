#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/idr.h>
#include <linux/kdev_t.h>
#include <linux/mutex.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/gpio.h>

#include "pulse.h"

#define DRIVER_NAME             "pulse"
#define DRIVER_VERSION          "0.80.0"

/*
 * Local variables
 */

static dev_t pulse_devt;
static struct class *pulse_class;

static DEFINE_MUTEX(pulse_idr_lock);
static DEFINE_IDR(pulse_idr);

/*
 * sysfs methods
 */

static ssize_t counter_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct pulse_device *pulse = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", atomic_read(&pulse->counter));
}
static DEVICE_ATTR_RO(counter);

static ssize_t counter_and_reset_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct pulse_device *pulse = dev_get_drvdata(dev);
	int counter = atomic_read(&pulse->counter);

	atomic_set(&pulse->counter, 0);
	return sprintf(buf, "%d\n", counter);
}
static DEVICE_ATTR_RO(counter_and_reset);

static ssize_t set_to_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct pulse_device *pulse = dev_get_drvdata(dev);
	int status, ret;

	ret = sscanf(buf, "%d", &status);
	if (ret != 1)
		return -EINVAL;

	atomic_set(&pulse->counter, status);

	return count;
}
static DEVICE_ATTR_WO(set_to);

/*
 * Class attributes
 */

static struct attribute *pulse_attrs[] = {
        &dev_attr_counter.attr,
        &dev_attr_counter_and_reset.attr,
        &dev_attr_set_to.attr,
        NULL,
};

static const struct attribute_group pulse_group = {
        .attrs = pulse_attrs,
};

static const struct attribute_group *pulse_groups[] = {
        &pulse_group,
        NULL,
};


static void pulse_device_destruct(struct device *dev)
{
        struct pulse_device *pulse = dev_get_drvdata(dev);

        /* Now we can release the ID for re-use */
        pr_debug("deallocating pulse%d\n", pulse->id);
        mutex_lock(&pulse_idr_lock);
        idr_remove(&pulse_idr, pulse->id);
        mutex_unlock(&pulse_idr_lock);

        kfree(dev);
        kfree(pulse);
}

/*
 * Exported functions
 */

void pulse_event(struct pulse_device *pulse)
{
	atomic_inc(&pulse->counter);
}
EXPORT_SYMBOL(pulse_event);

struct pulse_device *pulse_device_register(const char *name,
					   struct device *parent)
{
	struct pulse_device *pulse;
	dev_t devt;
	int ret;

	/* First allocate a new pulse device */
	pulse = kmalloc(sizeof(struct pulse_device), GFP_KERNEL);
	if (unlikely(!pulse))
		return ERR_PTR(-ENOMEM);

        mutex_lock(&pulse_idr_lock);
        /*
         * Get new ID for the new pulse source.  After idr_alloc() calling
         * the new source will be freely available into the kernel.
         */
        ret = idr_alloc(&pulse_idr, pulse, 0, PULSE_MAX_SOURCES, GFP_KERNEL);
        if (ret < 0) {
                if (ret == -ENOSPC) {
                        pr_err("%s: too many PPS sources in the system\n",
                               name);
                        ret = -EBUSY;
                }
                goto error_device_create;
        }
        pulse->id = ret;
        mutex_unlock(&pulse_idr_lock);

        devt = MKDEV(MAJOR(pulse_devt), pulse->id);

	/* Create the device and init the device's data */
	pulse->dev = device_create(pulse_class, parent, devt, pulse,
				   "%s", name);
	if (unlikely(IS_ERR(pulse->dev))) {
		dev_err(pulse->dev, "unable to create device %s\n", name);
		ret = PTR_ERR(pulse->dev);
		goto error_idr_remove;
	}
	dev_set_drvdata(pulse->dev, pulse);
	pulse->dev->release = pulse_device_destruct;

	/* Init the pulse data */
	strncpy(pulse->name, name, PULSE_NAME_LEN);
	atomic_set(&pulse->counter, 0);
	pulse->old_status = -1;

	dev_info(pulse->dev, "pulse %s added\n", pulse->name);

	return pulse;

error_idr_remove:
	mutex_lock(&pulse_idr_lock);
        idr_remove(&pulse_idr, pulse->id);

error_device_create:
	mutex_unlock(&pulse_idr_lock);
	kfree(pulse);

	return ERR_PTR(ret);
}
EXPORT_SYMBOL(pulse_device_register);

void pulse_device_unregister(struct pulse_device *pulse)
{
	/* Drop all allocated resources */
	device_destroy(pulse_class, pulse->dev->devt);

	dev_info(pulse->dev, "pulse %s removed\n", pulse->name);
}
EXPORT_SYMBOL(pulse_device_unregister);

/*
 * Module stuff
 */

static int __init pulse_init(void)
{
	printk(KERN_INFO "Pulse driver support v. " DRIVER_VERSION
		" - (C) 2014-2016 Rodolfo Giometti\n");

	/* Create the new class */
	pulse_class = class_create(THIS_MODULE, "pulse");
	if (!pulse_class) {
		printk(KERN_ERR "pulse: failed to allocate class\n");
		return -ENOMEM;
	}
	pulse_class->dev_groups = pulse_groups;

	return 0;
}

static void __exit pulse_exit(void)
{
	class_destroy(pulse_class);
}

module_init(pulse_init);
module_exit(pulse_exit);

MODULE_AUTHOR("Rodolfo Giometti <giometti@hce-engineering.com>");
MODULE_DESCRIPTION("Pulse driver support v. " DRIVER_VERSION);
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
