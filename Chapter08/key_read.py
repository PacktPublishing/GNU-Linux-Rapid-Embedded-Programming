#!/usr/bin/env python

from __future__ import print_function
import os
import sys
import getopt
import string
from evdev import InputDevice, ecodes
from select import select

NAME = os.path.basename(sys.argv[0])

keys = ".^1234567890....qwertzuiop....asdfghjkl.....yxcvbnm......................."

#
# Local functions
#

def error(*objs):
    print(NAME, ": ", *objs, file = sys.stderr)

def usage():
	print("usage: ", NAME, " [-h] <inputdev>", file = sys.stderr)
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

# Try to open the input device
try:
	dev = InputDevice(args[0])
except:
	error("invalid input device", args[0])
	sys.exit(1);

# Now read data from the input device printing only letters and numbers
while True:
	r, w, x = select([dev], [], [])
	for event in dev.read():
		# Print key pressed events only
		if event.type == ecodes.EV_KEY and event.value == 1:
			print(keys[event.code], end = "")
			sys.stdout.flush()	# needed by print()
