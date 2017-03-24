#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <syslog.h>
#include <time.h>
#include <mysql.h>
#include <mysql/errmsg.h>

#define NAME			program_invocation_short_name

void usage(void)
{
	fprintf(stderr, "usage: %s <name> <value>\n", NAME);

	exit(-1);
}
	
int main(int argc, char *argv[])
{
	const char *query = "REPLACE INTO status (t, n, v) "
			"VALUES(now(), '%s', '%s');";
	char *name, *value;
	MYSQL *c;
	char *sql;
	int ret = 0;

	/* Check the command line */
	if (argc < 3)
		usage();
	name = argv[1];
	value = argv[2];

	/* Get connect to MySQL daemon */
	c = mysql_init(NULL);
	if (!c) {
		fprintf(stderr, "unable to init MySQL data struct\n");
		return -1;
	}

	if (!mysql_real_connect(c, "127.0.0.1", "user", "userpass",
				    "sproject", 0, NULL, 0)) {
		fprintf(stderr, "unable to connect to MySQL daemon\n");
		ret = -1;
		goto close_db;
	}

	/* Ok, do the job! */
	ret = asprintf(&sql, query, name, value);
	if (ret < 0) {
		fprintf(stderr, "unable to allocate memory for query\n");
		goto close_db;
	}

	ret = mysql_query(c, sql);
	if (ret < 0)
		fprintf(stderr, "unable to access the database\n");

	free(sql);
close_db:
	mysql_close(c);

	return ret;
}
