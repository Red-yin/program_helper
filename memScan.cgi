#!/bin/bash
#index.cgi

echo "Content-Type:text/html;charset=utf-8"
echo

OLD_IFS="$IFS"
IFS="\&"
names=($@)
IFS="$OLD_IFS"

index=0
info=""
for name in ${names[@]}
do
	#之所以awk的参数要放到单引号里面，是因为这些参数不能被shell解释，是要交给awk处理的
	text="\"name\":\"$name\""
	pid=`ps -aux | grep $name | awk -v p=$name '$11 ~ /\/'$name'$/ {print $2}'`
	if [ ! $pid ];then
		val="$name is not running"
		text="{$text,\"status\":\"$val\"}"
	else
		val=`cat /proc/$pid/status | grep "Vm" | awk -F "[: ]+" '{key[NR]=$1; value[NR]=$3} END{printf("[");for(i=1;i<NR;i++){printf("{\"key\":\"%s\",\"value\":\"%s\"},", key[i], value[i])};printf("{\"key\":\"%s\",\"value\":\"%s\"}]", key[NR], value[NR]);}'`
		text="{$text,\"status\":$val}"
	fi
	info[index]=$text
	#format data to json
	let index++ 
	#	cat /proc/$pid/status | grep "Vm" | awk -F ":" '{key[NR]=$1; value[NR]=$2} END{for(i=1;i<=NR;i++){print key[i], value[i]};}'
done
#echo "${info[@]}"
#echo "${#info[@]}"
i=0
cgi_str="["
while [ $i -lt ${#info[@]} ]
do
	cgi_str=$cgi_str${info[$i]}
	let i++
	if [ $i -ne ${#info[@]} ];then
		cgi_str="$cgi_str,"
	fi
done
cgi_str="$cgi_str]"
echo $cgi_str
