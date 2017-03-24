#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>

/*
 * Defines
 */

#define NAME            program_invocation_short_name
#define SYSFS_GPIO_DIR	"/sys/class/gpio"
#define POLL_TIMEOUT	(1 * 1000)			/* in ms */

/* Some useful GPIO defines */
#define GPIO_IN		0
#define GPIO_OUT	1
#define GPIO_NONE	"none"
#define GPIO_RISING	"rising"
#define GPIO_FALLING	"falling"
#define GPIO_BOTH	"both"

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
 * GPIO number to GPIO name conversion function
 *
 * This is needed in order to keep in count the Atmel's non-standard names.
 * The code can be easily extended in order to suite all needs just adding
 * a new ordered entry in the lut[] array.
 */

#ifdef _ATMEL_GPIOS

char *lut[] = {
	[57] = "pioB25",
	[58] = "pioB26",
	[59] = "pioB27",
};

#else  /* ! _ATMEL_GPIOS */

char *lut[] = {
	[24] = "gpio24",
	[91] = "gpio91",
	[191] = "gpio191",
	[200] = "gpio200",
};

#endif /* _ATMEL_GPIOS */

char *gpio2name(int gpio)
{
	BUG_ON(gpio < 0);

	/* Check for gpio index out of range or if the corresponding entry
	 * into the lut[] array is not defined
	 */ 
	if (gpio >= ARRAY_SIZE(lut) || lut[gpio] == NULL) {
		err("unable to get GPIO%d name! "
		     "Consider to fix up the lut[] array", gpio);
		BUG();
	}

	return lut[gpio];
}

/*
 * Global variables
 */

int enable_debug;
int gpio[2];

/*
 * GPIOs management functions
 */

int gpio_export(unsigned int gpio)
{
	int fd, len;
	char *buf;
 
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0)
		return fd;
 
	len = asprintf(&buf, "%d", gpio);
	BUG_ON(len < 0);

	write(fd, buf, len);
	close(fd);
 
	free(buf);
	return 0;
}

int gpio_unexport(unsigned int gpio)
{
	int fd, len;
	char *buf;
 
	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0)
		return fd;
 
	len = asprintf(&buf, "%d", gpio);
	BUG_ON(len < 0);

	write(fd, buf, len);
	close(fd);

	free(buf);
	return 0;
}

int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	int fd, len;
	char *buf;
 
	len = asprintf(&buf, SYSFS_GPIO_DIR  "/%s/direction", gpio2name(gpio));
	BUG_ON(len < 0);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		free(buf);
		return fd;
	}
 
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);
 
	free(buf);
	close(fd);

	return 0;
}

int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd, len;
	char *buf;
 
	len = asprintf(&buf, SYSFS_GPIO_DIR "/%s/value", gpio2name(gpio));
	BUG_ON(len < 0);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		free(buf);
		return fd;
	}
 
	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);
 
	free(buf);
	close(fd);

	return 0;
}

int gpio_get_value(unsigned int gpio, unsigned int *value)
{
	int fd, len, n;
	char *buf;
	char ch;

	len = asprintf(&buf, SYSFS_GPIO_DIR "/%s/value", gpio2name(gpio));
	BUG_ON(len < 0);
 
	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return fd;
 
	n = read(fd, &ch, 1);

	*value = ch != '0' ? 1 : 0;
 
	free(buf);
	close(fd);

	return n;
}


int gpio_set_edge(unsigned int gpio, char *edge)
{
	int fd, len, n;
	char *buf;

	len = asprintf(&buf, SYSFS_GPIO_DIR "/%s/edge", gpio2name(gpio));
	BUG_ON(len < 0);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0)
		return fd;
 
	n = write(fd, edge, strlen(edge) + 1); 

	free(buf);
	close(fd);

	return n;
}

int gpio_fd_open(unsigned int gpio)
{
	int fd, len;
	char *buf;

	len = asprintf(&buf, SYSFS_GPIO_DIR "/%s/value", gpio2name(gpio));
	BUG_ON(len < 0);
 
	fd = open(buf, O_RDONLY | O_NONBLOCK );

	free(buf);

	return fd;
}

int gpio_fd_close(int fd)
{
	return close(fd);
}

/*
 * Clean up function
 */

static void cleanup(void)
{
	int i;

	dbg("clean up");

	for (i = 0; i < 2; i++)
		gpio_unexport(gpio[i]);
}

static void sighand_exit(int signo)
{
	dbg("sighand_exit");

	/* Call atexit() hooks */
	exit(EXIT_SUCCESS);
}

/*
 * Usage
 */

void usage(void)
{
	int i;

	fprintf(stderr, "usage: %s [--help|-h] [--debug|-d] gpio1# gpio2#\n",
			NAME);
	fprintf(stderr, "\n\tSupported GPIOs are:\n");
	for (i = 0; i < ARRAY_SIZE(lut); i++)
		if (lut[i])
			fprintf(stderr, "\t\tGPIO%d named as %s\n",
					i, gpio2name(i));

	exit(EXIT_FAILURE);
}

/*
 * Main
 */

int main(int argc, char *argv[])
{
	int c;
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"debug", no_argument, 0, 'd'},
	};
	sighandler_t sig_h;
	struct pollfd fdset[2];
	int i;
	int gpio_fd[2];
	unsigned int val;
	char v;
	int ret;

	/* Register signal handlers in order to do some clean up stuff
	 * at exit time...
	 */
	atexit(cleanup);
	sig_h = signal(SIGTERM, sighand_exit);  /* clean up on SIGTERM */
	if (sig_h == SIG_ERR) {
		err("unable to catch SIGTERM");
		exit(EXIT_FAILURE);
	}
	sig_h = signal(SIGINT, sighand_exit);
	if (sig_h == SIG_ERR) {
		err("unable to catch SIGINT");
		exit(EXIT_FAILURE);
	}

	/* Check the command line */
	while (1) {
		/* `getopt_long' stores the option index here. */
		int option_index = 0;

		c = getopt_long(argc, argv, "hd",
				long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
		case 0:
			/* If this option set a flag, do nothing else now */
			BUG_ON(long_options[option_index].flag == NULL);

			break;

		case 'h':       /* --help */
			usage();

		case 'd':       /* --debug */
			enable_debug++;

			break;

		case ':':
			/* "getopt_long" already printed an error message */
			exit(EXIT_FAILURE);

		case '?':
			/* "getopt_long" already printed an error message */
			err("unrecognized option \"%s\"", argv[optind - 1]);
			exit(EXIT_FAILURE);

		default:
			BUG();
		}
	}
	dbg("debug is on (level=%d)", enable_debug);

	/*
	 * Parse any remaining command line arguments (not options)
	 */

	argc -= optind;
	argv += optind;
	if (argc < 2)
		usage();

	for (i = 0; i < 2; i++) {
		ret = sscanf(argv[i], "%d", &gpio[i]);
		if (ret != 1) {
			err("invalid entry \"%s\"", argv[i]);
			exit(EXIT_FAILURE);
		}
		info("got GPIO%d named as %s", gpio[i], gpio2name(gpio[i]));
	}

	/*
	 * Now we have to set up both GPIOs as input falling edge
	 */

	for (i = 0; i < 2; i++) {
		ret = gpio_export(gpio[i]);
		if (ret < 0) {
			err("unable to export GPIO%d", gpio[i]);
			exit(EXIT_FAILURE);
		}

		ret = gpio_set_dir(gpio[i], GPIO_IN);
		if (ret < 0) {
			err("unable to set direction for GPIO%d", gpio[i]);
			exit(EXIT_FAILURE);
		}

		ret = gpio_set_edge(gpio[i], GPIO_FALLING);
		if (ret < 0) {
			err("unable to set edge for GPIO%d", gpio[i]);
			exit(EXIT_FAILURE);
		}

		ret = gpio_fd_open(gpio[i]);
		if (ret < 0) {
			err("unable to open GPIO%d", gpio[i]);
			exit(EXIT_FAILURE);
		}
		gpio_fd[i] = ret;
	}

	/*
	 * Do the job
	 */
 
	while (1) {
		/* Set up the fdset data structs */
		memset((void*) fdset, 0, sizeof(fdset));
		for (i = 0; i < 2; i++) {
			fdset[i].fd = gpio_fd[i];
			fdset[i].events = POLLPRI;
		}
      
		/* Do the poll() with timeout */
		ret = poll(fdset, 2, POLL_TIMEOUT);      
		BUG_ON(ret < 0);
		if (ret == 0) {
			/* No IRQs received!
			 * If debug is enabled then print GPIOs statuses,
			 * otherwise just print a dot "."
			 */
			if (enable_debug) {
				for (i = 0; i < 2; i++) {
					ret = gpio_get_value(gpio[i], &val);
					BUG_ON(ret < 0);
					dbg("read() GPIO%d=%d", gpio[i], val);
				}
			} else {
				printf(".");
				fflush(stdout);
			}
	   	} else { 
			/* IRQ received!
			 * Print out the new GPIO status
			 */
			for (i = 0; i < 2; i++) {
				if (fdset[i].revents & POLLPRI) {
					ret = lseek(fdset[i].fd, SEEK_SET, 0);
					BUG_ON(ret < 0);
					ret = read(fdset[i].fd, &v, 1);
					BUG_ON(ret < 1);

					if (ret == 1) {
						info("poll() GPIO%d=%c",
							gpio[i], v);
					}
				}
			}
		}
	}

	/* Never reached */

	return 0;
}
