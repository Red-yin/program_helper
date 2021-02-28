#!/bin/bash
#index.cgi

echo "Content-Type:text/html;charset=utf-8"
echo

for name in $@
do
	#之所以awk的参数要放到单引号里面，是因为这些参数不能被shell解释，是要交给awk处理的
	pid=`ps -aux | grep $name | awk -v p=$name '$11 ~ /\/'$name'$/ {print $2}'`
	if [ ! $pid ];then
		echo "$name is not running"
		continue
	fi
	#format data to json
	cat /proc/$pid/status | grep "Vm" | awk -F ":" '{key[NR]=$1; value[NR]=$2} END{printf("[");for(i=1;i<NR;i++){printf("{\"key\":\"%s\",\"value\":\"%s\"},", key[i], value[i])};printf("{\"key\":\"%s\",\"value\":\"%s\"}]", key[NR], value[NR]);}'
	#	cat /proc/$pid/status | grep "Vm" | awk -F ":" '{key[NR]=$1; value[NR]=$2} END{for(i=1;i<=NR;i++){print key[i], value[i]};}'
done
