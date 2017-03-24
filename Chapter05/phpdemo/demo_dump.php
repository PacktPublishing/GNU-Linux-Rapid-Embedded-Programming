<h1>OpenWRT PHP demo</h1>

<table cellpadding='5' cellspacing='1' border='1'>
<?php

# Get connect to MySQL daemon
$conn = mysqli_connect("localhost", "user", "userpass", "phpdemo",
		0, "/var/run/mysqld.sock");
if (mysqli_connect_errno())
	die("unable to connect with MySQL daemon");

# Ok, do the job!
$query = "SELECT * FROM status;";
$dbres = mysqli_query($conn, $query);
if (!$dbres)
	die("unable to execute the query");

# Do the dump of the fields' names
echo "<thead>\n";
$hdrrow = mysqli_fetch_assoc($dbres);
echo "  <tr>\n";
foreach ($hdrrow as $hdr => $value)
	echo "    <th>$hdr</th>\n";
echo "</thead>\n";

# Seek to row 0
mysqli_data_seek($dbres, 0);

# Do the dump one line at time
while ($row = mysqli_fetch_assoc($dbres)) {
	echo "  <tr>\n";
	foreach ($row as $cell)
                echo "<TD>$cell</TD>";
	echo "  </tr>\n";
}

mysqli_close($conn);

?>
</table>
