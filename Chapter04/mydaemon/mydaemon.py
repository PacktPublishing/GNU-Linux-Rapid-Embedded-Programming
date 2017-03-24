#!/usr/bin/python

from __future__ import print_function
import os
import sys
import time
import getopt
import syslog
import signal
import daemon

NAME = os.path.basename(sys.argv[0])

debug = False
daemonize = True
logstderr = False

#
# Logging functions
#

def dbg(x):
	if not debug:
		return

	syslog.syslog(syslog.LOG_DEBUG, str(x))

def info(x):
	syslog.syslog(syslog.LOG_INFO, str(x))

#
# Signals handler
#

def sig_handler(sig, frame):
	dbg("signal trapped!")
	sys.exit(0)

#
# The daemon body
#

def daemon_body():
	# The main loop
	dbg("start main loop")
	while not time.sleep(1):
		info("I\'m working hard!")

#
# Usage
#

def usage():
	print("usage: ", NAME, " [-h] [-d] [-f] [-l]", file = sys.stderr)
	print("    -h    - show this message", file = sys.stderr)
	print("    -d    - enable debugging messages", file = sys.stderr)
	print("    -f    - do not daemonize", file = sys.stderr)
	print("    -l    - log on stderr", file = sys.stderr)

	sys.exit(1)

#
# Main
#

# Check the command line
try:
	opts, args = getopt.getopt(sys.argv[1:], "hdfl")
except getopt.GetoptError, err:
	print(str(err), file = sys.stderr)
	usage()

for o, a in opts:
	if o in ("-h"):
		usage()
	elif o in ("-d"):
		debug = True
	elif o in ("-f"):
		daemonize = False
	elif o in ("-l"):
		logstderr = True
	else:
		assert False, "unhandled option"

# Open the communication with syslogd
loglevel = syslog.LOG_PID
if logstderr:
	loglevel |= syslog.LOG_PERROR
syslog.openlog(NAME, loglevel, syslog.LOG_USER)

# Define the daemon context and install the signals traps
context = daemon.DaemonContext(
	detach_process = daemonize,
)
context.signal_map = {
	signal.SIGTERM: sig_handler,
	signal.SIGINT: sig_handler,
}
dbg("signals traps installed")

# Start the daemon
with context:
	daemon_body()

sys.exit(0)
