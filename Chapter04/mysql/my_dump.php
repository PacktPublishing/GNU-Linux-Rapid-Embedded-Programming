#!/usr/bin/php
<?php

# Get connect to MySQL daemon
$ret = mysql_connect("127.0.0.1", "user", "userpass");
if (!$ret)
	die("unable to connect with MySQL daemon");

$ret = mysql_select_db("sproject");
if (!$ret)
	die("unable to select database");

# Ok, do the job!
$query = "SELECT * FROM status;";
$dbres = mysql_query($query);
if (!$dbres)
	die("unable to execute the query");

# Do the dump of the fields' names
$n = mysql_num_fields($dbres);
for ($i = 0; $i < $n; $i++)
	printf("%s\t", mysql_field_name($dbres, $i));
printf("\n");

# Do the dump one line at time
while ($row = mysql_fetch_array($dbres)) {
	for ($i = 0; $i < $n; $i++)
		printf("%s\t", $row[$i]);
	printf("\n");
}

mysql_close();

?>
