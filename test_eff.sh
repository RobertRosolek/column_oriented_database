#!/bin/bash

# Text color variables
txtund=$(tput sgr 0 1)    # Underline
txtbld=$(tput bold)       # Bold
txtred=$(tput setaf 1)    # Red
txtgrn=$(tput setaf 2)    # Green
txtylw=$(tput setaf 3)    # Yellow
txtblu=$(tput setaf 4)    # Blue
txtpur=$(tput setaf 5)    # Purple
txtcyn=$(tput setaf 6)    # Cyan
txtwht=$(tput setaf 7)    # White
txtrst=$(tput sgr0)       # Text reset

# times measured before optimisation phase
K=( 0 2728 1237 696 1773 119 189 1871 1898 17337 1458 32089 24333 )

# sum of proportions of obtained times and times in K
SUMP=0.0

N=12
OK=1
SUM=0

make -s clean
make -s
echo "Compilation completed"
echo ""

for ((i=1; i <= $N; ++i))
do
	echo "Test $i"
	output=$( { time ./exec_plan $i queries/q$i.ascii 2>/dev/null; } 2>&1 )
	#get everything to the right of first "*user "
	user=${output#*user }
	#get everything to the left of the first "s*"
	user=${user%%s*}
	#get everythig to let left of "m*"
	min=${user%%m*}
	#get everything to the right of "*m" and left of ".*"
	sec=${user#*m}
	sec=${sec%%.*}
	#get everything to the right of "*."
	usec=${user#*.}
	tim=$[ 10#$usec + (10#$sec) * 1000 ]
	echo "Time (usec): $tim"

	# update record for this test case
	if [ -f "logs/best$i.log" ];
	then
		rtim=`cat logs/best$i.log`
		COMP=`echo $tim "<=" $rtim | bc -l`
		if [ "$COMP" -eq 1 ]
		then
			rtim="$tim"
		fi
	else
		rtim="$tim"
	fi

	# save result to the file
	echo "$rtim" > "logs/best$i.log"
	
	echo "Time best (usec): $rtim"
	SUM=`expr $SUM + $tim`
	X=`echo $tim / ${K[$i]} | bc -l`
	echo "Proportion to unoptimized program: $X"
	SUMP=`echo $SUMP + $X | bc -l`
done

# output test summary
echo ""
echo ""
AVG=`expr $SUM / $N`
echo "Average time (usec): $AVG"
echo "Total time (usec): $SUM"
AVGP=`echo $SUMP / $N | bc -l`
echo "Average proportion to unoptimized program: $AVGP"


#update records
if [ -f logs/avg_record.log ];
then
	RAVG=`cat logs/avg_record.log`
	COMP=`echo $AVG "<=" $RAVG | bc -l`
	if [ "$COMP" -eq 1 ]
	then
		RAVG="$AVG"
	fi
else
	RAVG="$AVG"
fi

if [ -f logs/sum_record.log ];
then
	RSUM=`cat logs/sum_record.log`
	COMP=`echo $SUM "<=" $RSUM | bc -l`
	if [ "$COMP" -eq 1 ]
	then
		RSUM="$SUM"
	fi
else
	RSUM="$SUM"
fi

if [ -f logs/avgp_record.log ];
then
	RAVGP=`cat logs/avgp_record.log`
	COMP=`echo $AVGP "<=" $RAVGP | bc -l`
	if [ "$COMP" -eq 1 ]
	then
		RAVGP="$AVGP"
	fi
else
	RAVGP="$AVGP"
fi

# output records

echo ""
echo "Average time best (usec): $RAVG"
echo "Total time best (usec): $RSUM"
echo "Average proportion to unoptimized program best (usec): $RAVGP"

# save records
echo "$RAVG" > logs/avg_record.log
echo "$RSUM" > logs/sum_record.log
echo "$RAVGP" > logs/avgp_record.log
