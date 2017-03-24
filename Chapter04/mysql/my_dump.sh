#!/bin/sh

NAME=$(basename $0)

PASSWD="userpass"

# Ok, do the job!
mysql -u user --password=$PASSWD -D sproject <<__EOF__

# Do the whole dump
SELECT * FROM status;

__EOF__

exit 0
