#include <linux/device.h>
#include <linux/atomic.h>

#define PULSE_MAX_SOURCES	32	/* should be enough... */
#define PULSE_NAME_LEN		32

/*
 * Pulse port basic structs
 */

/* Main struct */
struct pulse_device {
	char name[PULSE_NAME_LEN];

	atomic_t counter;
	unsigned int old_status;

	unsigned int id;
	struct module *owner;
	struct device *dev;
};

/*
 * Exported functions
 */

#define to_class_dev(obj) container_of((obj), struct class_device, kobj)
#define to_pulse_device(obj) container_of((obj), struct pulse_device, class)

extern void pulse_event(struct pulse_device *pulse);
extern struct pulse_device *pulse_device_register(const char *name,
						  struct device *parent);
extern void pulse_device_unregister(struct pulse_device *pulse);
