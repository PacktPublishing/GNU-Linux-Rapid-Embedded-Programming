#!/bin/bash

NAME=$(basename $0)
declare -a valid_boards=(bbb a5d3)
declare -a valid_gpios_bbb=(69 68 45 44 23 26 47 46 27 65)
declare -a valid_gpios_a5d3=(A16)

function usage() {
	echo "usage: $NAME bbb|a5d3 <gpio> <Hz>" >&2
	exit 1
}

#
# Main
#

# Check the command line
[ $# -lt 3 ] && usage
board=$1
gpio=$2
hz=$3

# Sanity checks for input values
#
# - valid boards are: bbb, a5d3
case $board in
bbb)
	declare -a valid_gpios=${valid_gpios_bbb[*]}
;;

a5d3)
	declare -a valid_gpios=${valid_gpios_a5d3[*]}
;;
esac
#
# - valid GPIOs are: 69, 68, 45, 44, 23, 26, 47, 46, 27, 65
# - valid Hz values: [1:50]
if ! [[ ${valid_gpios[*]} =~ $(echo "\<$gpio\>") ]] ; then
	echo "$NAME: GPIO # not allowed, must be in [${valid_gpios[*]}]" >&2
	exit 1
fi
if [ $hz -lt 1 -o $hz -gt 50 ] ; then
	echo "$NAME: invalid Hz value, must be in [1:50]" >&2
	exit 1
fi

# Convert GPIO name -> number
case $board in
bbb)
	num=$gpio
	name=gpio$gpio
;;

a5d3)
	declare -A base
	base[A]=0
	port=${gpio:0:1}

	num=$(( ${gpio:1:2} + ${base[$port]} ))
	name=pio$gpio
;;
esac

# Export the GPIO and set it as output
echo $num > /sys/class/gpio/export 
echo out > /sys/class/gpio/$name/direction 

# Start to generate the pulse
# Period 1/Hz and duty cicle 50%
echo "$NAME: start in generate the waveform. Hit ENTER key to stop"

to=$(awk "BEGIN { print 1 / $hz / 2 }")	;# compute 1/hz/2
while true ; do 
	# Do the high pulse and the wait for the half period
	echo 1 > /sys/class/gpio/$name/value
	read -t $to val && break

	# Do the low pulse and the wait for the half period
	echo 0 > /sys/class/gpio/$name/value
	read -t $to val && break
done

# Unexport the GPIO
echo $num > /sys/class/gpio/unexport

exit 0
