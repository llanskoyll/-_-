#!/bin/bash


while true
do
	var_time=$(cat time_sec)
	echo $var_time >> display_text_test
	read time_5sec
	ans=$(($var_time-$time_5sec))
	sleep 5
	echo $ans
done
