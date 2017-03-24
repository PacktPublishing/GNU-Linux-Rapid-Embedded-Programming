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
	const char *query = "SELECT * FROM status;";
	MYSQL *c;
	MYSQL_RES *q_res;
	MYSQL_FIELD *field;
	MYSQL_ROW row;
	int i, n;
	int ret = 0;

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
	ret = mysql_query(c, query);
	if (ret < 0)
		fprintf(stderr, "unable to access the database\n");

	/* Save the query result */
	q_res = mysql_store_result(c);
	if (!q_res) {
		fprintf(stderr, "unable to store query result\n");
		goto close_db;
	}

	/* Do the dump of the fields' names */
	while ((field = mysql_fetch_field(q_res))) 
		printf("%s\t", field->name);
	printf("\n");	   

	/* Do the dump one line at time */
	n = mysql_num_fields(q_res);
	while ((row = mysql_fetch_row(q_res))) {
		for (i = 0; i < n; i++)
			printf("%s\t", row[i] ? row[i] : NULL);
		printf("\n");
	}

	mysql_free_result(q_res);
close_db:
	mysql_close(c);

	return ret;
}
