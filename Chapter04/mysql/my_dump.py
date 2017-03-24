#!/usr/bin/python

import MySQLdb

# Get connect to MySQL daemon
db = MySQLdb.connect(host = "localhost", user = "user", passwd = "userpass",
				db = "sproject")

# Create the Cursor object to execute all queries
c = db.cursor() 

# Ok, do the job!
c.execute("SELECT * FROM status")

# Save the query result
data = c.fetchall()

# Do the dump of the fields' names
for field in c.description:
	print("%s\t" % (field[0])),
print

# Do the dump one line at time
n = len(c.description)
for row in data:
	for i in range(0, n):
		print("%s\t" % (row[i])),
	print

c.close()
db.close()
