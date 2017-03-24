#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>

#define NAME				program_invocation_short_name
#define VERSION			"0.10.0"

/*
 * Defines
 */

#define DEVICE			"/dev/ttyS0"
#define BAUDRATE		9600
#ifndef PAGE_SIZE
#define PAGE_SIZE		4096
#endif

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
 * Serial port management
 */

void flush_serial(int fd)
{
	tcflush(fd, TCIOFLUSH);
}

int set_serial(int fd, int rate)
{
	struct termios term;

	int ret;

	/* Sanity checks */
	switch (rate) {
	case 9600 :
		rate = B9600;
		break;

	case 19200 :
		rate = B19200;
		break;

	case 38400 :
		rate = B38400;
		break;

	case 57600 :
		rate = B57600;
		break;

	case 115200 :
		rate = B115200;
		break;

	default :	/* error */
		return -1;
	}

	ret = tcgetattr(fd, &term);
	if (ret < 0)
		return ret;

	ret = cfsetispeed(&term, rate);
	if (ret < 0)
		return ret;
	ret = cfsetospeed(&term, rate);
	if (ret < 0)
		return ret;

	cfmakeraw(&term);
	term.c_cc[VTIME] = 0;
	term.c_cc[VMIN] = 1;
	ret = tcsetattr(fd, TCSANOW, &term);

	return 0;
}

int open_serial(const char *name)
{
	char *dev;

	if (!name) {
		dev = getenv("SERIAL_DEVICE");
		if (dev)
			name = dev;
		else
			name = DEVICE;
	}

	return open(name, O_RDWR | O_NOCTTY);
}

void close_serial(int fd)
{
	close(fd);
}

/*
 * Global variables
 */

int enable_debug;

/*
 * Usage
 */

void usage(void)
{
	fprintf(stderr, "usage: %s <options>\n", NAME);
	fprintf(stderr, "   where <options> are:\n");
	fprintf(stderr, "      [-h | --help]         : show this help\n");
	fprintf(stderr, "      [-d | --debug]        : enable debug\n");
	fprintf(stderr, "      [-b | --baud <rate>]  : set baudrate (default %d)\n", BAUDRATE);
	fprintf(stderr, "      [-D | --device <dev>] : use device <dev> (default %s)\n", DEVICE);

	exit(EXIT_FAILURE);
}

/*
 * Main
 */

int main(int argc, char *argv[])
{
	int c;
	static struct option long_options[] = {
		{"help",	no_argument,       0, 'h'},
		{"debug",	no_argument,       0, 'd'},
		{"baud",	required_argument, 0, 'b'},
		{"device",	required_argument, 0, 'D'},
		{0, 0, 0, 0}
	};

	char *device = DEVICE;
	int baudrate = BAUDRATE;
	char buf[PAGE_SIZE];
	int fd;
	int i, n;
	int ret;

	/* Check the command line */
	while (1) {
		/* `getopt_long' stores the option index here. */
		int option_index = 0;

		c = getopt_long(argc, argv, "hdb:D:",
			 long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
                case 0:
                        /* If this option set a flag, do nothing else now */
                        BUG_ON(long_options[option_index].flag == NULL);

                        break;

		case 'h' :	/* --help */
			usage();

                case 'd':       /* --debug */
                        enable_debug++;

                        break;

		case 'b' :	/* --baud */
			if (sscanf(optarg, "%d", &baudrate) < 1) {
				err("must specify a baud rate value");
				exit(EXIT_FAILURE);
			}

			break;

		case 'D' :	/* --device */
			device = optarg;
			break;

		case ':' :
			/* "getopt_long" already printed an error message */
			exit(EXIT_FAILURE);

		case '?' :
			/* "getopt_long" already printed an error message */
			err("unrecognized option \"%s\"", argv[optind - 1]);
			exit(EXIT_FAILURE);

		default :
			BUG();
		}
	}
	        dbg("debug is on (level=%d)", enable_debug);

        /*
         * Parse any remaining command line arguments (not options)
         */

        argc -= optind;
        argv += optind;

	/* Open the serial device */
	ret = open_serial(device);
	if (ret < 0) {
		err("unable to open the serial device");
		exit(EXIT_FAILURE);
	}
	fd = ret;

	/* Set up the serial device by setting the user defined baudrate and
	 * the raw mode
	 */
	ret = set_serial(fd, baudrate);
	if (ret < 0) {
		err("unable to setup the serial device");
		exit(EXIT_FAILURE);
	}

        /*
         * Do the job
         */

        while (1) {
		/* Read the data from the serial port */
		ret = read(fd, buf, ARRAY_SIZE(buf));
		if (ret < 0) {
			err("error reading from the serial port");
			exit(EXIT_FAILURE);
		}
		n = ret;

		/* Check for End-Of-File condition */
		if (n == 0)
			break;

		/* Write the just read data to the stdout replacing the
		 * non printable characters with a "."
		 */
		for (i = 0; i < n; i++) {
			if (buf[i] == '\n' || buf[i] == '\r' ||
			    isprint((int) buf[i]))
				ret = printf("%c", buf[i]);
			else
				ret = printf(".");
			if (ret < 0) {
				err("error reading from the serial port");
				exit(EXIT_FAILURE);
			}
		}

		/* Flush out the data */
		fflush(stdout);
        }

	close_serial(fd);

	return 0;
}
