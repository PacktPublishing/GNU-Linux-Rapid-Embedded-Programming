#!/bin/bash

while /bin/true; do
	read line

	line=$(echo $line | tr -d '\n\r')
	[ "$line" == "quit" ] && break;

	echo -e "$line\r"
done

exit 0
