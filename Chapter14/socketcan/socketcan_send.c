#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#define NAME			program_invocation_short_name

void usage(void)
{
	fprintf(stderr, "usage: %s <candev>\n", NAME);

	exit(-1);
}

/*
 * Main
 */

int main(int argc, char *argv[])
{
	int s;
	int n;
	char *ifname;
	struct sockaddr_can addr;
	struct can_frame frame;
	struct ifreq ifr;
	int ret;

	/* Check command line */
	if (argc < 1)
		usage();
	ifname = argv[1];
	
	/* Open the PF_CAN socket */
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0) {
		perror("Error while opening socket");
		exit(-1);
	}

	/* Find the CAN device */
	strcpy(ifr.ifr_name, ifname);
	ret = ioctl(s, SIOCGIFINDEX, &ifr);
        if (ret < 0) {
		perror("ioctl");
		exit(-1);
	}
	printf("%s: %s at index %d\n", NAME, ifname, ifr.ifr_ifindex);
	
	/* Bind the socket */
	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	ret = bind(s, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		perror("bind");
		exit(-1);
	}

	/* Fill the frame data */
	frame.can_id  = 0x123;
	frame.can_dlc = 2;
	frame.data[0] = 0x11;
	frame.data[1] = 0x22;

	/* Send the frame */
	n = write(s, &frame, sizeof(struct can_frame));
	if (ret < 0) {
		perror("write");
		exit(-1);
	}
	printf("%s: wrote %d bytes\n", NAME, n);
	
	return 0;
}
