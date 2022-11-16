#!/bin/bash

while true
do
	sleep 1
	#curr_data=$(date +"%T.%N")
	echo $(date +%s) >> time_sec
done
