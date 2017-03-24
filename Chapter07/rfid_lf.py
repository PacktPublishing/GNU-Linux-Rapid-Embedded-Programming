#!/usr/bin/env python

from __future__ import print_function
import os
import sys
import getopt
import string
import serial

NAME = os.path.basename(sys.argv[0])

#
# Local functions
#

def error(*objs):
    print(NAME, ": ", *objs, file = sys.stderr)


def reader(ser):
	while True:
		line = ser.readline()
		line = filter(lambda x: x in string.printable, line)
	        print(line.replace("\n", "")),

def usage():
        print("usage: ", NAME, " [-h] <serdev>", file = sys.stderr)
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
dev = args[0]

# Configure the serial connections
ser = serial.Serial(
	port		= dev,
	baudrate	= 9600,
	bytesize	= 8,
	parity		= 'N',
	stopbits	= 1,
	timeout		= None,
	xonxoff		= 0,
	rtscts		= 0
)

reader(ser)

ser.close()
