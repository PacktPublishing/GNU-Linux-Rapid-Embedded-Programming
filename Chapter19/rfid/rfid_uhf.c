#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <caenrfid.h>

#define NAME		program_invocation_short_name

int debug = 0;

static char nibble2hex(char c)
{
	switch (c) {
	case 0 ... 9:
		return '0' + c;

	case 0xa ... 0xf:
		return 'a' + (c - 10);
	}

	printf("got invalid data!\n");
	return '\0';
}

/*
 * Local functions
 */

char *bin2hex(uint8_t *data, size_t len)
{
	char *str;
	int i;

	str = malloc(len * 2 + 1);
	if (!str)
		return NULL;

	for (i = 0; i < len; i++) {
		str[i * 2] = nibble2hex(data[i] >> 4);
		str[i * 2 + 1] = nibble2hex(data[i] & 0x0f);
	}
	str[i * 2] = '\0';

	return str;
}

/*
 * Usage function
 */

void usage(void)
{
	fprintf(stderr, "usage: %s <port>\n", NAME);

	exit(-1);
}

/*
 * Main
 */

int main(int argc, char *argv[])
{
	int i;
	struct caenrfid_handle handle;
	char string[] = "Source_0";
	struct caenrfid_tag *tag;
	size_t size;
	char *str;
	int ret;

	if (argc < 2)
		usage();

	/* Start a new connection with the CAENRFIDD server */
	ret = caenrfid_open(CAENRFID_PORT_RS232, argv[1], &handle);
	if (ret < 0)
		usage();

	/* Set session "S2" for logical source 0 */
	ret = caenrfid_set_srcconf(&handle, "Source_0",
				CAENRFID_SRC_CFG_G2_SESSION, 2);
	if (ret < 0) {
		fprintf(stderr, "cannot set session 2 (err=%d)\n", ret);
		exit(EXIT_FAILURE);
	}

	while (1) {
		/* Do the inventory */
		ret = caenrfid_inventory(&handle, string, &tag, &size);
		if (ret < 0) {
			fprintf(stderr, "cannot get data (err=%d)\n", ret);
			exit(EXIT_FAILURE);
		}
	
		/* Report results */
		for (i = 0; i < size; i++) {
			str = bin2hex(tag[i].id, tag[i].len);
			if (!str) {
				fprintf(stderr,
					"cannot allocate data (err=%d)\n", ret);
				exit(EXIT_FAILURE);
			}
	
			printf("%.*s %.*s %.*s %d\n",
				tag[i].len * 2, str,
				CAENRFID_SOURCE_NAME_LEN, tag[i].source,
				CAENRFID_READPOINT_NAME_LEN, tag[i].readpoint,
				tag[i].type);
	
			free(str);
		}
	
		/* Free inventory data */
		free(tag);
	}

	caenrfid_close(&handle);

	return 0;
}
