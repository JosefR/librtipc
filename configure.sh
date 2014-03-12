#!/bin/bash

#
# configure
#
# Description: Script to set up librtipc to compile for a specific architecture and system 
# Author: Josef Raschen <josef@raschen.org>
#


# check parameters

ARCH="unknown"
SYSTEM="unknown"

if [ $# != 2 ]
	then
		echo "ERROR: wrong parameter count" 
		exit 1
fi

if [ -d "arch/$1" ]
	then
		ARCH="$1"
		echo "Select architecure: $ARCH"
	else
		echo "ERROR: cannot find architecture subdirectory arch/$1"
		exit 1
fi

if [ -d "system/$2" ]
	then
		SYSTEM="$2"
		echo "Selected system: $SYSTEM"
	else
		echo "ERROR: cannot find system subdirectory system/$2"
		exit 1
fi


# set symlinks 

echo "Configure for architecture ${ARCH}."

rm -f include/arch
ln -s ../arch/${ARCH}/include include/arch
rm -f src/arch
ln -s ../arch/${ARCH}/src src/arch

echo "Configure for system: ${SYSTEM}."

rm -f include/system
ln -s ../system/${SYSTEM}/include include/system
rm -f src/system
ln -s ../system/${SYSTEM}/src src/system

echo "Successfully configured librtipc."
