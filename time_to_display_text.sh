#!/bin/bash


while true
do
	#encoder -q | script.sh
	read time_5sec
	let "time_5sec = $time_5sec/18"
	var_time=$(cat time_data)
	cat time_data | xargs -I {} sudo lcd/lcd "{}"
	ans=$(($var_time-$time_5sec))
	sleep 1
	echo $ans
done
