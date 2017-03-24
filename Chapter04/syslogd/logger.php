<?php
openlog("mydaemon", LOG_NOWAIT, LOG_USER);

syslog(LOG_INFO, "logging message in PHP");

closelog();
?>
