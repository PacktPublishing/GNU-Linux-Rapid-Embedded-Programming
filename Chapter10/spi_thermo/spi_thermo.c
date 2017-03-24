#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <endian.h>

#define NAME            program_invocation_short_name

#define SPI_DEV         "/dev/spidev1.0"

#define THERMO_SHIFT	18
#define	THERMO_MASK	((1 << 15) - 1)
#define THERMO_SIGN	(1 << 31)
#define FAULT_FLAG	(1 << 16)

int main(int argc, char **argv)
{
	int fd;
	unsigned int data;
	float t;
	int ret;

	/* Open the SPI bus device connected to the thermocouple chip */
	fd = open(SPI_DEV, O_RDWR);
	if (fd < 0) {
                fprintf(stderr, "%s: cannot get access to SPI bus\n", NAME);
                exit(1);
	}

	/* Read the 32-bit data */
	ret = read(fd, &data, 4);
	if (ret < 0) {
                fprintf(stderr, "%s: cannot read data from SPI device\n", NAME);
                exit(1);
	}
	if (ret != 4) {
                fprintf(stderr, "%s: short read\n", NAME);
                exit(1);
	}

	/* Adapt the endianness to the current host */
	data = be32toh(data);

	/* Check for errors */
	if (data & FAULT_FLAG) {
		fprintf(stderr, "%s: invalid emperature data\n", NAME);
		exit(1);
	}

	/* Get the temperature data */
	t = (data >> THERMO_SHIFT) & THERMO_MASK;
	t /= 4;
	if (data & THERMO_SIGN)
		t = -t;

	printf("%.2f\n", t);

	return 0;
}
