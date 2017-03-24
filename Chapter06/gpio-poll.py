#!/usr/bin/python

from __future__ import print_function
import os
import sys
import select

gpio24  = "/sys/class/gpio/gpio24/value"
gpio91  = "/sys/class/gpio/gpio91/value"

# Get the GPIOs streams
stream24 = open(gpio24, 'r');
stream91 = open(gpio91, 'r');

while True :
	# Set up stream sets for the select()
	read = []
	write = []
	exept = [stream24, stream91]

	# Wait for IRQs (without timeout)...
	r, w, e = select.select(read, write, exept)
	for i, input in enumerate(e) :
		# Read the GPIO status
		input.seek(0, 0)
    		status = input.read().rstrip("\n")

		# Get the filename from "/sys/class/gpio/gpioXX/value"
		path = os.path.dirname(input.name)
		gpio = os.path.basename(path)

		print("%s status=%s" % (gpio, status))
