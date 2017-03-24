#!/bin/sh

NAME=$(basename $0)

PASSWD="userpass"

usage() {
        echo "usage: $NAME <name> <value>" >&2
        exit 1
}

[ $# -lt 2 ] && usage
name=$1
value=$2

# Ok, do the job!
mysql -u user --password=$PASSWD -D phpdemo <<__EOF__

REPLACE INTO status (t, n, v) VALUES(now(), '$name', '$value');

__EOF__

exit 0
