#!/usr/bin/env python

from __future__ import print_function
import os
import sys
import getopt
import string
import socket

NAME = os.path.basename(sys.argv[0])

host = ""	# the empty string is treat as INADDR_ANY


#
# Local functions
#

def error(*objs):
    print(NAME, ": ", *objs, file = sys.stderr)

def info(*objs):
    print(NAME, ": ", *objs)

def usage():
	print("usage: ", NAME, " [-h] <port>", file = sys.stderr)
	sys.exit(2);

#
# Main
#

try:
        opts, args = getopt.getopt(sys.argv[1:], "h",
                        ["help"])
except getopt.GetoptError, err:
        # Print help information and exit:
        error(str(err))
        usage()

for o, a in opts:
        if o in ("-h", "--help"):
                usage()
        else:
                assert False, "unhandled option"

# Check command line
if len(args) < 1:
        usage()

# Check the user input
try:
	port = int(args[0])
except ValueError, err:
	error("invalid number for <port>:", args[0])
	sys.exit(1)
if port < 0 or port > 65535:
	error("invalid number for port, must be [0, 65535]:", args[0])
	sys.exit(1)

# Create a TCP/IP socket object
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET,
		socket.SO_REUSEADDR, 1) # avoid error: Address already in use
server_address = (host, port)
s.bind(server_address)
info("starting up on %s port %s" % s.getsockname())

# Now we can listen for an incoming client connection
s.listen(5)

# The main loop
while True:
	info("waiting for new connection...")

	# Establish connection with the client
	c, addr = s.accept()
	info("got connection from ", addr)

	# Send back an hello message
	c.send("Thank you for connecting!\n")

	# Close the connection
	c.close()
	info("connection closed!")
