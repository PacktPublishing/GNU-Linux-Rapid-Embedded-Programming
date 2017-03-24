#!/usr/bin/php
<?php

define("NAME", basename($argv[0]));
declare(ticks = 1);

$debug = false;
$daemonize = true;
$logstderr = false;

#
# Logging functions
#

function dbg()
{
	global $debug;
	if (!isset($debug) || !$debug)
		return;

	$argv = func_get_args();
	$format = array_shift($argv);
	syslog(LOG_DEBUG, vsprintf($format, $argv));
}

function info()
{
	$argv = func_get_args();
	$format = array_shift($argv);
	syslog(LOG_INFO, vsprintf($format, $argv));
}

#
# Signals handler
#

function sig_handler($signo)
{
	dbg("signal trapped!");
	die();
}

#
# The daemon body
#

function daemon_body()
{
	# The main loop
	dbg("start main loop");
	while (sleep(1) == 0)
		info("I'm working hard!");
}

#
# Usage
#

function usage()
{
	fprintf(STDERR, "usage: %s [-h] [-d] [-f] [-l]\n", NAME);
	fprintf(STDERR, "    -h    - show this message\n");
	fprintf(STDERR, "    -d    - enable debugging messages\n");
	fprintf(STDERR, "    -f    - do not daemonize\n");
	fprintf(STDERR, "    -l    - log on stderr\n");

	die();
}

#
# Main
#

# Check the command line
$shortopts  = "hdfl";
$options = getopt($shortopts);
$nopts = 0;
foreach ($options as $o => $a) {
	switch ($o) {
	case "h":
		usage();

	case "d":
		$debug = true;
		$nopts++;
		break;

	case "f":
		$daemonize = false;
		$nopts++;
		break;

	case "l":
		$logstderr = true;
		$nopts++;
		break;
	}
}

# Open the communication with syslogd
$loglevel = LOG_PID;
if ($logstderr)
	$loglevel |= LOG_PERROR;
openlog(NAME, $loglevel, LOG_USER);

# Install the signals traps
pcntl_signal(SIGTERM, "sig_handler");
pcntl_signal(SIGINT,  "sig_handler");
dbg("signals traps installed");

# Start the daemon
if ($daemonize) {
	dbg("going in background...");
	$pid = pcntl_fork();
	if ($pid < 0) {
		die("unable to daemonize!");
	}
	if ($pid) {
		# The parent can exit...
		exit(0);
	}
	# ... while the children goes on!

	# Set the working directory to /
	chdir("/");

	# Close all of the standard file descriptors as we are running
	# as a daemon
	fclose(STDIN);
	fclose(STDOUT);
	fclose(STDERR);
}

daemon_body();

exit(0);
