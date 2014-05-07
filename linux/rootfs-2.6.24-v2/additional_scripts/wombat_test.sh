#!/bin/sh
i=1
j=31
k=0
l=41
start=`date +%s`
while(true);do
	echo -en "\033[$k""m"
        echo -en "\033[$j""mWOMBAT $i......"
	current=`date +%s`
	time_since=`expr $current - $start`
	echo -e "$time_since seconds\033[0m"
	i=`expr $i + 1`
	j=`expr $i % 8`
	j=`expr $j + 31`
	k=`expr $i % 7`
	l=`expr 48 - $k`
done
