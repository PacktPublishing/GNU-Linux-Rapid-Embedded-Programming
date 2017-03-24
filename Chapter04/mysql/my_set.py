#!/usr/bin/python

from __future__ import print_function
import os
import sys
import MySQLdb

NAME = os.path.basename(sys.argv[0])

def usage():
	print("usage: ", NAME, " <name> <value>", file = sys.stderr)
	sys.exit(2);

# Check the command line
if len(sys.argv) < 3:
	usage()

# Get connect to MySQL daemon
db = MySQLdb.connect(host = "localhost", user = "user", passwd = "userpass",
				db = "sproject")

# Create the Cursor object to execute all queries
c = db.cursor() 

# Ok, do the job!
query = "REPLACE INTO status (t, n, v) " \
	"VALUES(now(), '%s', '%s');" % (sys.argv[1], sys.argv[2])
c.execute(query)


c.close()
db.close()
