#!/bin/bash

# The server's root directory
base=/var/www/html

# Read the browser request
read request

# Now read the message header
while /bin/true; do
	read header
	[ "$header" == $'\r' ] && break;
done

# Parse the GET request
tmp="${request#GET }"
tmp="${tmp% HTTP/*}"

# Extract the code after the '?' char to capture a variable setting
var="${tmp#*\?}"
[ "$var" == "$tmp" ] && var=""

# Get the URL and replace it with "/index.html" in case it is set to "/"
url="${tmp%\?*}"
[ "$url" == "/" ] && url="/index.html"

# Extract the filename
filename="$base$url"
extension="${filename##*.}"

# Check for file exist
if [ -f "$filename" ]; then
	echo -e "HTTP/1.1 200 OK\r"
	echo -e "Contant-type: text/html\r"
	echo -e "\r"

	# If file's extension is "cgi" and it's executable the execute it,
	# otherwise just return its contents
	if [ "$extension" == "cgi" -a -x "$filename" ]; then
		$filename $var
	else
		cat "$filename"
	fi
	echo -e "\r"
else
	# If the file does not exist return an error
	echo -e "HTTP/1.1 404 Not Found\r"
	echo -e "Content-Type: text/html\r"
	echo -e "\r"
	echo -e "404 Not Found\r"
	echo -e "The requested resource was not found\r"
	echo -e "\r"
fi

exit 0
