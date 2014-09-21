#**********************
#*
#* Progam Name: MP1. Membership Protocol.
#*
#* Current file: Grader.sh
#* About this file: Grading Script.
#* 
#***********************
#!/bin/sh

function contains () {
  	local e
  	for e in "${@:2}"
	do 
		if [ "$e" == "$1" ]; then 
			echo 1
			return 1;
		fi
	done
  	echo 0
}

verbose=$(contains "-v" "$@")
grade=0

echo "============================================"
echo "Grading Started"
echo "============================================"
echo "Single Failure Scenario"
echo "============================"
if [ $verbose -eq 0 ]; then
	make clean > /dev/null
	make > /dev/null
	./app testcases/singlefailure.conf > /dev/null
else
	make clean
	make
	./app testcases/singlefailure.conf
fi
joincount=`grep joined dbg.log | cut -d" " -f2,4-7 | uniq | wc -l`
if [ $joincount -eq 100 ]; then
	grade=`expr $grade + 10`
	echo "Checking Join..................10/10"
else
	echo "Checking Join..................0/10"
fi
failednode=`grep failed dbg.log | uniq | awk '{print $1}'`
failcount=`grep removed dbg.log | uniq | grep $failednode | wc -l`
if [ $failcount -ge 9 ]; then
	grade=`expr $grade + 10`
	echo "Checking Completeness..........10/10"
else
	echo "Checking Completeness..........0/10"
fi
failednode=`grep failed dbg.log | uniq | awk '{print $1}'`
accuracycount=`grep removed dbg.log | uniq | grep -v $failednode | wc -l`
if [ $accuracycount -eq 0 ] && [ $failcount -gt 0 ]; then
	grade=`expr $grade + 10`
	echo "Checking Accuracy..............10/10"
else
	echo "Checking Accuracy..............0/10"
fi
echo "============================================"
echo "Multi Failure Scenario"
echo "============================"
if [ $verbose -eq 0 ]; then
	make clean > /dev/null
	make > /dev/null
	./app testcases/multifailure.conf > /dev/null
else
	make clean
	make
	./app testcases/multifailure.conf
fi
joincount=`grep joined dbg.log | cut -d" " -f2,4-7 | uniq | wc -l`
if [ $joincount -eq 100 ]; then
	grade=`expr $grade + 10`
	echo "Checking Join..................10/10"
else
	echo "Checking Join..................0/10"
fi
failednode=`grep failed dbg.log | uniq | awk '{print $1}'`
tmp=0
for i in $failednode
do
	failcount=`grep removed dbg.log | uniq | grep $i | wc -l`
	if [ $failcount -ge 5 ]; then
		tmp=`expr $tmp + 2`
		grade=`expr $grade + 2`
	fi
done
echo "Checking Completeness..........$tmp/10"
failednode=`grep failed dbg.log | uniq | awk '{print $1}'`
tmp=0
for i in $failednode
do
	accuracycount=`grep removed dbg.log | uniq | grep -v $i | wc -l`
	if [ $accuracycount -eq 20 ]; then
		tmp=`expr $tmp + 2`
		grade=`expr $grade + 2`
	fi
done
echo "Checking Accuracy..............$tmp/10"
echo "============================================"
echo "Message Drop Single Failure Scenario"
echo "============================"
if [ $verbose -eq 0 ]; then
	make clean > /dev/null
	make > /dev/null
	./app testcases/msgdropsinglefailure.conf > /dev/null
else
	make clean
	make
	./app testcases/msgdropsinglefailure.conf
fi
joincount=`grep joined dbg.log | cut -d" " -f2,4-7 | sort -u | wc -l`
if [ $joincount -eq 100 ]; then
	grade=`expr $grade + 10`
	echo "Checking Join..................10/10"
else
	echo "Checking Join..................0/10"
fi
failednode=`grep failed dbg.log | uniq | awk '{print $1}'`
failcount=`grep removed dbg.log | uniq | grep $failednode | wc -l`
if [ $failcount -ge 9 ]; then
	grade=`expr $grade + 10`
	echo "Checking Completeness..........10/10"
else
	echo "Checking Completeness..........0/10"
fi
failednode=`grep failed dbg.log | uniq | awk '{print $1}'`
accuracycount=`grep removed dbg.log | uniq | grep -v $failednode | wc -l`
if [ $accuracycount -eq 0 ] && [ $failcount -gt 0 ]; then
	grade=`expr $grade + 10`
	echo "Checking Accuracy..............10/10"
else
	echo "Checking Accuracy..............0/10"
fi
echo "============================================"
echo Final Grade $grade/90
