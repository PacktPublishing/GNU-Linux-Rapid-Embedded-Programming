#!/bin/bash

# Read the browser request
read request

# Now read the message header
while /bin/true; do
	read header
	echo "$header"
	[ "$header" == $'\r' ] && break;
done

# And then produce an answer with a message dump
echo -e "HTTP/1.1 200 OK\r"
echo -e "Content-type: text/html\r"
echo -e "\r"

echo -e "request=$request\r"

exit 0
