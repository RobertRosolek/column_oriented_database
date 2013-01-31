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

N=12
OK=1

# change server to the old one
cp server.cc server_backup.cc
rm -f server.cc
cp server_old.cc server.cc

make -s clean
make -s DEBUGFLAG=-DDEBUG_ON=1

for ((i=1; i <= $N; ++i))
do
	echo "Test $i"
	./exec_plan $i oldqueries/q$i.ascii >result$i.out
	g++ tester$i.cpp
	if ./a.out <result$i.out; then
		echo "${txtgrn}OK!${txtrst}"
		rm result$i.out
	else
		echo "${txtred}WRONG ANSWER!${txtrst}"
		OK=0
		break	
	fi
done

# restore server
cp server_backup.cc server.cc
rm -f server_backup.cc

if [ "$OK" -eq 1 ] 
then
	echo ""
	echo "${txtgrn} SYSTEM TESTS PASSED ${txtrst}"
fi

make -s clean
