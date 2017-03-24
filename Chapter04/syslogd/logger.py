import syslog

syslog.openlog("mydaemon", syslog.LOG_NOWAIT, syslog.LOG_USER)

syslog.syslog(syslog.LOG_INFO, "logging message in Python")

syslog.closelog()
