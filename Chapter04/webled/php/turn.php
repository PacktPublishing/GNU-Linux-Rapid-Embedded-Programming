<?php
	# 1st part - Global defines & functions
	define("value_f", "/sys/class/gpio/gpio66/value");

	function pr_str($val)
	{
		echo $val ? "on" : "off";
	}

	# 2nd part - Set the new led status as requested
	if (isset($_GET["led"])) {
		$led_new_status = $_GET["led"];
		file_put_contents(value_f, $led_new_status);
	}

	# 3rd part - Get the current led status
	$led_status = intval(file_get_contents(value_f));

	# 4th part - Logic to change the led status on the next call
	$led_new_status = 1 - $led_status;

	# 5th part - Render the led status by HTML code
?>
<html>
  <head>
    <title>Turning a led on/off using PHP</title>
  </head>

  <body>
    <h1>Turning a led on/off using PHP</h1>
    Current led status is: <? pr_str($led_status) ?>
    <p>

    Press the button to turn the led <? pr_str($led_new_status) ?>
    <p>

    <form method="get" action="/turn.php">
      <button type="submit" value="<? echo $led_new_status ?>"
		name="led">Turn <? pr_str($led_new_status) ?></button>
    </form>

  </body>
</html>
