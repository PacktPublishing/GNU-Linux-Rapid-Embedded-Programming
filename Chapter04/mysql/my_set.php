#!/usr/bin/php
<?php

function usage($name)
{
	die("usage: $name <name> <value>\n");
}

/* Check the command line */
if ($argc < 3)
	usage($argv[0]);
$name = $argv[1];
$value = $argv[2];

# Get connect to MySQL daemon
$ret = mysql_connect("127.0.0.1", "user", "userpass");
if (!$ret)
	die("unable to connect with MySQL daemon");

$ret = mysql_select_db("sproject");
if (!$ret)
	die("unable to select database");

# Ok, do the job!
$query = "REPLACE INTO status (t, n, v) " .
	 "VALUES(now(), '$name', '$value');";
$dbres = mysql_query($query);
if (!$dbres)
	die("unable to execute the query");

mysql_close();

?>
