#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <syslog.h>
#include <signal.h>

#define NAME		program_invocation_short_name

int debug = 0;
int daemonize = 1;
int logstderr = 0;

/*
 * Logging functions
 */

#define dbg(fmt, args...)						\
	do {								\
		if (debug)						\
			syslog(LOG_DEBUG, fmt , ## args);		\
	} while (0)

#define info(fmt, args...)					      \
	syslog(LOG_INFO, fmt , ## args)

/*
 * Signals handler
 */

void sig_handler(int signo)
{
	dbg("signal trapped!");
	exit(0);
}

/* 
 * The daemon body
 */

void daemon_body(void)
{
	/* The main loop */
	dbg("start main loop");
	while (sleep(1) == 0)
		info("I'm working hard!");
}

/*
 * Usage function
 */

void usage(void)
{
	fprintf(stderr, "usage: %s [-h] [-d] [-f] [-l]\n", NAME);
	fprintf(stderr, "    -h    - show this message\n");
	fprintf(stderr, "    -d    - enable debugging messages\n");
	fprintf(stderr, "    -f    - do not daemonize\n");
	fprintf(stderr, "    -l    - log on stderr\n");

	exit(-1);
}

/*
 * Main
 */

int main(int argc, char *argv[])
{
	int c, option_index;
	int loglevel;
	sighandler_t sig_h;
	int ret;

	opterr = 0;	  /* disbale default error message */
	while (1) {
		/* getopt_long stores the option index here */
		option_index = 0;

		c = getopt_long(argc, argv, "hdfl", NULL, &option_index);

		/* Detect the end of the options */
		if (c == -1)
			break;

		switch (c) {
		case 0:
			break;

		case 'h':
			usage();

		case 'd':
			debug = 1;
			break;

		case 'f':
			daemonize = 0;
			break;

		case 'l':
			logstderr = 1;
			break;

		case '?':
			fprintf(stderr, "unhandled option");
			exit(-1);

		default:
			exit(-1);
		}
	}

	/* Open the communication with syslogd */
	loglevel = LOG_PID;
	if (logstderr)
		loglevel |= LOG_PERROR;
	openlog(NAME, loglevel, LOG_USER);

	/* Install the signals traps */
	sig_h = signal(SIGTERM, sig_handler);
	if (sig_h == SIG_ERR) {
		fprintf(stderr, "unable to catch SIGTERM");
		exit(-1);
	}
	sig_h = signal(SIGINT, sig_handler);
	if (sig_h == SIG_ERR) {
		fprintf(stderr, "unable to catch SIGINT");
		exit(-1);
	}
	dbg("signals traps installed");

	/* Should run as a daemon? */
	if (daemonize) {
		ret = daemon(!daemonize, 1);
		if (ret) {
			fprintf(stderr, "unable to daemonize the process");
			exit(-1);
		}
	}

	daemon_body();

	return 0;
}

