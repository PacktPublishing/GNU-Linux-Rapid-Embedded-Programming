#!/bin/bash

NAME=$(basename $0)

daemonize="&"

#
# Logging functions
#

function dbg () {
	[ $debug ] && logger -p user.debug -t $NAME $logstderr $1
}

function info () {
	logger -p user.info -t $NAME $logstderr $1
}

#
# Signals handler
#

function sig_handler () {
	dbg "signal trapped!"
	exit 0
}

#
# The daemon body
#

function daemon_body () {
	# The main loop
	dbg "start main loop"
	while sleep 1 ; do
		info "I'm working hard!"
	done
}

#
# Usage
#

function usage () {
	echo "usage: $NAME [-h] [-d] [-f] [-l]" >&2
	echo "    -h    - show this message" >&2
	echo "    -d    - enable debugging messages" >&2
	echo "    -f    - do not daemonize" >&2
	echo "    -l    - log on stderr" >&2

	exit 1
}

#
# Main
#

# Check the command line
TEMP=$(getopt -o hdfl -n $NAME -- "$@")
[ $? != 0 ] && exit 1
eval set -- "$TEMP"
while true ; do
	case "$1" in
	-h)
		usage
		;;

	-d)
		debug="true"
		shift
		;;

	-f)
		daemonize=""
		shift
		;;

	-l)
		logstderr="--stderr"
		shift
		;;

	--)
		shift
		break
		;;

	*)
		echo "$NAME: internal error!" >&2
		exit 1
		;;
	esac
done

# Install the signals traps
trap sig_handler SIGTERM SIGINT
dbg "signals traps installed"

# Start the daemon
if [ -n "$daemonize" ] ; then
	dbg "going in background..."

	# Set the working directory to /
	cd /
fi
[ -z "$logstderr" ] && tmp="2>&1"
eval daemon_body </dev/null >/dev/null $tmp $daemonize

exit 0
