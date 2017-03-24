#!/usr/bin/python

from __future__ import print_function
import os
import sys
import smbus

# Defines
BUS = 0
ADDRESS = 0x5a
REG = 0x07

# Open the I2C bus /dev/i2c-X
bus = smbus.SMBus(BUS)

# Read a single register
raw_value = bus.read_word_data(ADDRESS, REG)

# Convert the data in C degrees and then display it
degrees = raw_value * .02 - 273.15
print("%0.2f" % degrees)
