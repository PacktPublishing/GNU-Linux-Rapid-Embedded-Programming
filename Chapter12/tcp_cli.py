#!/usr/bin/env python

from __future__ import print_function
import os
import sys
import getopt
import string
import socket

NAME = os.path.basename(sys.argv[0])

#
# Local functions
#

def error(*objs):
    print(NAME, ": ", *objs, file = sys.stderr)

def info(*objs):
    print(NAME, ": ", *objs)

def usage():
	print("usage: ", NAME, " [-h] <address> <port>", file = sys.stderr)
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
if len(args) < 2:
        usage()

# Check the user inputs
host = args[0]
try:
	port = int(args[1])
except ValueError, err:
	error("invalid number for <port>:", args[1])
	sys.exit(1)
if port < 0 or port > 65535:
	error("invalid number for port, must be [0, 65535]:", args[0])
	sys.exit(1)

# Create a TCP/IP socket object
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect with the server
info("starting new connection...")
s.connect((host, port))

# Print the server's hello message
info("server says:", s.recv(1024))

# Close the connection
s.close()
info("connection closed!")
