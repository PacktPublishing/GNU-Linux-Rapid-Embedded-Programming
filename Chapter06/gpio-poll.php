#!/usr/bin/php
<?php
        define("gpio24", "/sys/class/gpio/gpio24/value");
        define("gpio91", "/sys/class/gpio/gpio91/value");

	# Get the GPIOs streams
	$stream24 = fopen(gpio24, 'r');
	$stream91 = fopen(gpio91, 'r');

	while (true) {
		# Set up stream sets for the select()
		$read = NULL;
		$write = NULL;
		$exept = array($stream24, $stream91);

		# Wait for IRQs (without timeout)...
		$ret = stream_select($read, $write, $exept, NULL);
		if ($ret < 0)
			die("stream_select: error");

		foreach ($exept as $input => $stream) {
			# Read the GPIO status
			fseek($stream, 0, SEEK_SET);
        		$status = intval(fgets($stream));

			# Get the filename from "/sys/class/gpio/gpioXX/value"
			$meta_data = stream_get_meta_data($stream);
			$gpio = basename(dirname($meta_data["uri"]));

			printf("$gpio status=$status\n");
		}
	}
?>
