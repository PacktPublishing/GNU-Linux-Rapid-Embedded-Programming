#!/usr/bin/env python

import signal
import daemon
import sys
import logging
from time import sleep
from smartcard.CardMonitoring import CardMonitor, CardObserver
from smartcard.util import *

logging.basicConfig(level = logging.INFO)

#
# Signals handler
#

def sig_handler(sig, frame):
        sys.exit(0)

#
# Smart Card Observer
#

class printobserver(CardObserver):
	def update(self, observable, (addedcards, removedcards)):
		for card in addedcards:
			logging.info("->] " + toHexString(card.atr))
		for card in removedcards:
			logging.info("<-] " + toHexString(card.atr))

#
# The daemon body
#

def daemon_body():
        # The main loop
	logging.info("INFO waiting for card... (hit CTRL+C to stop)")

	try:
		cardmonitor = CardMonitor()
		cardobserver = printobserver()
		cardmonitor.addObserver(cardobserver)

		while True:
			sleep(1000000) # sleep forever

	except:
		cardmonitor.deleteObserver(cardobserver)

#
# Main
#

# Define the daemon context and install the signals traps
context = daemon.DaemonContext(
        detach_process = False,
	stdout = sys.stdout,
	stderr = sys.stderr,
)
context.signal_map = {
        signal.SIGTERM: sig_handler,
        signal.SIGINT: sig_handler,
}

# Start the daemon
with context:
        daemon_body()

sys.exit(0)
