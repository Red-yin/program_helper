#!/bin/bash

if [[ $# -lt 1 ]];then
	echo "usage: ./memScan.sh process1 process2 ..."
	exit
fi

while [ 1 -eq 1 ]
do
	for name in $@
	do
	#之所以awk的参数要放到单引号里面，是因为这些参数不能被shell解释，是要交给awk处理的
		pid=`ps -aux | grep $name | awk -v p=$name '$11 ~ /\/'$name'$/ {print $2}'`
		if [ ! $pid ];then
			echo "$name is not running"
			continue
		fi
		echo $pid
		cat /proc/$pid/status | grep "Vm"
	done
	sleep 1
done
