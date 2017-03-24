#!/bin/sh

NAME=$(basename $0)

echo -n "Warning, all data will be dropped!!! [CTRL-C to stop or ENTER to continue]"
read ans

# Ok, do the job!
mysql -u root -p <<__EOF__

# Drop all existing data!!!
DROP DATABASE IF EXISTS sproject;

# Create new database
CREATE DATABASE sproject;

# Grant privileges
GRANT USAGE ON *.* TO user@localhost IDENTIFIED BY 'userpass';
GRANT ALL PRIVILEGES ON sproject.* TO user@localhost;
FLUSH PRIVILEGES;

# Select database
USE sproject;

# Create the statuses table
CREATE TABLE status (
	t DATETIME NOT NULL,
	n VARCHAR(64) NOT NULL,
	v VARCHAR(64) NOT NULL,
	PRIMARY KEY (n),
	INDEX (n)
) ENGINE=MEMORY;

__EOF__

exit 0
