#include <linux/module.h>
#include <linux/init.h>

/* This is the function executed during the module loading */
static int dummy_module_init(void)
{
	printk("dummy_module loaded!\n");
	return 0;
}

/* This is the function executed during the module unloading */
static void dummy_module_exit(void)
{
	printk("dummy_module unloaded!\n");
	return;
}

module_init(dummy_module_init);
module_exit(dummy_module_exit);

MODULE_AUTHOR("Rodolfo Giometti <giometti@hce-engineering.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
