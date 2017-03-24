#!/usr/bin/env python

from __future__ import print_function
import os
import sys
import getopt
import spidev

NAME = os.path.basename(sys.argv[0])

debug = 0

#
# Local functions
#

def error(*objs):
	print(NAME, ": ", *objs, file = sys.stderr)

def info(*objs):
	print(NAME, ": ", *objs)

def dbg(*objs):
	if (debug):
    		print(NAME, ": ", *objs)

def do_write(data):
	spi = spidev.SpiDev()
	spi.open(32765, 0)	# the SPI device for output

	# Do some SPI settings
	spi.max_speed_hz = 1000000
	spi.bits_per_word = 16

	# Compute the checksum
	p0	= data ^ (data >> 1)
	p0	= p0 ^ (p0 >> 2)
	p0	= p0 ^ (p0 >> 4)
	p0	= p0 & 1;
	p1	= data ^ (data >> 2)
	p1	= p1 ^ (p1 >> 4)
	p2	= p1 & 1
	p1	= p1 & 2
	p1	= p1 >> 1
	np0	= not p0
	tmp = (p2 << 3) | (p1 << 2) | (p0 << 1) | np0
	tmp = 0x01
	dbg("p2.p1.p0.np0=0x%01x" % (tmp))

	# Do the write
	dbg("write=0x%04x" % ((data << 8) | tmp))
	data = spi.xfer2([tmp, data])

	# Decode answer
	faults	= data[1]
	ok	= 1 if data[0] & 0b10000000 else 0
	twarn	= 0 if data[0] & 0b01000000 else 1
	pc	= 1 if data[0] & 0b00100000 else 0
	pg	= 0 if data[0] & 0b00010000 else 1
	p	= data[0] & 0b00001111
	dbg("faults=0x%02x ok=%d twarn=%d pc=%d pg=%d p2.p1.p0.np0=0x%01x" %
		(faults, ok, twarn, pc, pg, p))

	spi.close()

def do_read():
	spi = spidev.SpiDev()
	spi.open(32765, 1)	# the SPI device for input

	# Do some SPI settings
	spi.max_speed_hz = 1000000
	spi.bits_per_word = 16

	# Do the read
	data = spi.xfer2([0, 0]) 
	dbg("read=0x%04x" % ((data[1] << 8) | data[0]))

	spi.close()

	# Compute the checksum and extract alarms
	uva	= 1 if data[0] & 0b10000000 else 0
	ota	= 1 if data[0] & 0b01000000 else 0
	pc	= (data[0] >> 2) & 0b00001111
	ok	= 1 if (data[0] & 0b00000011) == 1 else 0

	dbg("inputs=0x%02x uva=%d ota=%d pc=0x%x ok=%d" %
		(data[1], uva, ota, pc, ok))

	return data[1]

def usage():
	print("usage: ", NAME, " [-h|--help -d|--debug] w <data> | r",
		file = sys.stderr)
        sys.exit(2);

#
# Main
#

try:
        opts, args = getopt.getopt(sys.argv[1:], "hd", ["help", "--debug"])
except getopt.GetoptError, err:
        # print help information and exit:
	error(str(err))
        usage()

for o, a in opts:
        if o in ("-h", "--help"):
                usage()
        elif o in ("-d", "--debug"):
                debug = 1
        else:
                assert False, "unhandled option"

# Check command line
if len(args) < 1:
    usage()

# Check the user inputs
cmd = args[0]
if cmd == "w":
	if len(args) < 1:
		error("missing <data> parameter!")
                sys.exit(1)
	try:
		data = int(args[1], 16)
	except ValueError, err:
		error("invalid number for <data>:", args[1])
		sys.exit(1)
	if data < 0 or data > 65535:
		error("invalid number for port, must be [0, 65535]:", args[0])
		sys.exit(1)

	do_write(data)
elif cmd == "r":
	data = do_read()

	print("0x%02x" % data)
else:
	error("invalid command, must be [w,r]", args[0])
