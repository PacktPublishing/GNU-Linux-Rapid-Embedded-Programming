#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <libusb.h>

/*
 * Defines
 */

#define NAME            program_invocation_short_name

/* The g_zero default USB vendor:device product numbers */
#define VENDOR_ID	0x1a0a
#define PRODUCT_ID	0xbadd

/* Logging & other stuff */

#define __message(stream, fmt, args...)                                 \
                fprintf(stream, "%s: " fmt "\n" , NAME , ## args)

#define DBG(code)                                                       \
        do {                                                            \
                if (unlikely(enable_debug)) do {                        \
                        code                                            \
                } while (0);                                            \
        } while (0)

#define info(fmt, args...)                                              \
        __message(stdout, fmt , ## args)

#define err(fmt, args...)                                               \
        __message(stderr, fmt , ## args)

#define dbg(fmt, args...)                                               \
        do {                                                            \
                if (unlikely(enable_debug))                             \
                        __message(stderr, fmt , ## args);               \
        } while (0)

#define BUILD_BUG_ON_ZERO(e)                                            \
                (sizeof(char[1 - 2 * !!(e)]) - 1)
#define __must_be_array(a)                                              \
                BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(typeof(a), \
                                                        typeof(&a[0])))
#define ARRAY_SIZE(arr)                                                 \
                (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

#define unlikely(x)       __builtin_expect(!!(x), 0)
#define BUG()                                                           \
        do {                                                            \
                err("fatal error in %s():%d", __func__, __LINE__);      \
                exit(EXIT_FAILURE);                                     \
        } while (0)
#define EXIT_ON(condition)                                              \
        do {                                                            \
                if (unlikely(condition))                                \
                        BUG();                                          \
        } while(0)
#define BUG_ON(condition)       EXIT_ON(condition)

#define WARN()                                                          \
        do {                                                            \
                err("warning notice in %s():%d", __func__, __LINE__);   \
        } while (0)
#define WARN_ON(condition)                                              \
        do {                                                            \
                if (unlikely(condition))                                \
                        WARN();                                         \
        } while(0)

/*
 * Global variables
 */

uint8_t buffer[4096] = "TEST STRING\n";

/*
 * Main
 */

int main(int argc, char *argv[])
{
	libusb_device_handle *handle;
	int flag = 0;		/* set to 1 if kernel driver detached */
	int n;
	int ret;

	/* Init th libusb */
	ret = libusb_init(NULL);
	if (ret < 0) {
        	err("unable to init libusb");
        	exit(-1);
	}

	/* Check for g_zero device is connected to the system */
	handle = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
	if (!handle) {
		err("unable to open g_zero device");
		exit(-1);
	}
	info("g_zero device found");

	/*
	 * Check whether a kernel driver is attached to interface #0.
	 * If so, we'll need to detach it.
	 */
	if (libusb_kernel_driver_active(handle, 0)) {
		ret = libusb_detach_kernel_driver(handle, 0);
		if (ret == 0) {
			flag = 1;
		} else {
			err("error detaching kernel driver");
			exit(-1);
		}
	}

	/* Claim interface #0 */
	ret = libusb_claim_interface(handle, 0);
	if (ret) {
		err("error claiming interface");
		exit(-1);
	}

	/* Send an all-zeros message to endpoint 0x01 */
	ret = libusb_bulk_transfer(handle, 0x01, buffer,
					sizeof(buffer), &n, 100);
	if (ret) {
    		err("error sending message to device ret=%d", ret);
		exit(-1);
	}
	info("%d bytes transmitted successfully", n);

	sleep(1);

	/* Receive an all-zeros message from endpoint 0x81 */
	ret = libusb_bulk_transfer(handle, 0x81, buffer,
					sizeof(buffer), &n, 100);
	if (ret) {
		err("error receiving message from device ret=%d", ret);
		exit(-1);
	}
	if (n != sizeof(buffer)) {
		err("error receiving %d bytes while expecting %ld",
					n, sizeof(buffer));
		exit(-1);
	}
	info("%d bytes received successfully", n);

	printf("string=%s\n", buffer);

	/* Release interface #0 */
	ret = libusb_release_interface(handle, 0);
	if (ret) {
		err("error releasing interface");
		exit(-1);
	}

	/*
	 * If we detached a kernel driver from interface #0 earlier, we'll now 
	 * need to attach it again.
	 */
	if (flag)
		libusb_attach_kernel_driver(handle, 0);

	libusb_exit(NULL);

	return 0;
}
