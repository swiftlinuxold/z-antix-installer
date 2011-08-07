#!/bin/bash
# Proper header for a Bash script.

# Check for root user login
if [ $( id -u ) -eq 0 ]; then
	echo "You must NOT be root to run this script."
	exit
fi

USERNAME=$(logname)

# This is the script for compiling the source code of the Swift Linux installer

cd /home/$USERNAME/develop/installer/src
rm minstall
qmake-qt4
make
