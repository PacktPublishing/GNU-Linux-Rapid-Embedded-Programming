#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

#define NAME		program_invocation_short_name

#define I2C_DEV		"/dev/i2c-1"
#define I2C_SLAVE_ADDR	(0x90 >> 1)

static void reg2value(unsigned char b1, unsigned char b2, int *val)
{
	*val = (b1 << 2) | (b2 >> 6);
}

static void value2reg(int val, unsigned char *b1, unsigned char *b2)
{
	*b1 = val >> 2;
	*b2 = val << 6;
}

int main(int argc, char *argv[])
{
	int fd;
	int val;
	unsigned char wbuf[] = {
		0,	/* register address */
		0,	/* data 1st byte */
		0,	/* data 2nd byte */
	};
	unsigned char rbuf[] = {
		0,	/* data 1st byte */
		0,	/* data 2nd byte */
	};
	int ret;

	/* Open the I2C bus */
	ret = open(I2C_DEV, O_RDWR);
	if (ret < 0) {
		fprintf(stderr, "%s: unable to open the i2c bus device %s\n",
			NAME, I2C_DEV);
		exit(1);
	}
	fd = ret;

	/* Select the I2C bus to talk with */
	ret = ioctl(fd, I2C_SLAVE, I2C_SLAVE_ADDR);
	if (ret < 0) {
		fprintf(stderr, "%s: cannot acquire access to address 0x%x\n",
			NAME, I2C_SLAVE_ADDR);
		exit(1);
	}

	/*
	 * Check the command line in order to know if we must
	 * read or write data
	 */
	if (argc == 1) { /* must read */
		ret = read(fd, rbuf, sizeof(rbuf));
		if (ret != sizeof(rbuf)) {
			fprintf(stderr, "%s: failed to read: %m\n", NAME);
			exit(1);
		}

		/* Convert the just read data to a readable form */
		reg2value(rbuf[0], rbuf[1], &val);

		printf("%d\n", val);
	} else {	/* must write */
		ret = sscanf(argv[1], "%d", &val);
		if (ret !=1 || val < 0 || val >= (1 << 10)) {
			fprintf(stderr, "%s: invalid value! Must be in [0:%d]\n",
				NAME, (1 << 10) - 1);
			exit(-1);
		}
	
		/* Convert the user's value into a suitable form for the DAC */
		value2reg(val, &wbuf[1], &wbuf[2]);

		/* Write the data to the device */
		ret = write(fd, wbuf, sizeof(wbuf));
		if (ret != sizeof(wbuf)) {
			fprintf(stderr, "%s: failed to write: %m\n", NAME);
			exit(1);
		}
	}

	return 0;
}
