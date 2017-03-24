#!/bin/bash

# 1st part - Global defines & functions
value_f="/sys/class/gpio/gpio66/value"

function pr_str ( ) {
	[ $1 == 1 ] && echo "on"
	[ $1 == 0 ] && echo "off"
}

# 2nd part - Set the new led status as requested
if [ -n "$1" ] ; then
	eval $1	;# this evaluate the query 'led=0'
	led_new_status=$led
	echo $led_new_status > $value_f
fi

led_status=$(cat $value_f)

led_new_status=$((1 - $led_status))

cat <<EOF
<html>
  <head>
    <title>Turing a led on/off using BASH</title>
  </head>

  <body>
    <h1>Turing a led on/off using BASH</h1>
    Current led status is: $(pr_str $led_status)
    <p>

    Press the button to turn the led $(pr_str $led_new_status)
    <p>

    <form method="get" action="/turn.cgi">
      <button type="submit" value="$led_new_status" name="led">Turn $(pr_str $led_new_status)</button>
    </form>
  </body>
</html>
EOF

exit 0
