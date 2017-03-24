#include <syslog.h>

int main(void)
{
	openlog("mydaemon", LOG_NOWAIT, LOG_USER);

	syslog(LOG_INFO, "logging message in C");

	closelog();

	return 0;
}
