#! /bin/bash

clear
echo "Checking if directory /temp exists..."
if [ -d "temp" ]
then
	echo "Deleting /temp..."
	rm -rf temp
fi

echo "Creating /temp..."
mkdir temp


echo "Copying fuseLib.c and myFS.h to /temp and /mount-point..."
cp fuseLib.c temp/fuseLib.c
cp fuseLib.c mount-point/fuseLib.c
cp myFS.h temp/myFS.h
cp myFS.h mount-point/myFS.h

echo "Checking Virtual Disk..."
./fsck virtual-disk

if diff temp/fuseLib.c mount-point/fuseLib.c
then
	true
else
	echo "fuseLib.c is different"
	exit 1
fi

if diff temp/myFS.h mount-point/myFS.h
then
	true
else
	echo "myFS.h is different"
	exit 1
fi

echo "Truncating fuseLib.c and myFS.h to one block less..."
truncate -s -1 -o temp/fuseLib.c
truncate -s -1 -o mount-point/fuseLib.c
truncate -s -1 -o temp/myFS.h
truncate -s -1 -o mount-point/myFS.h

echo "Checking Virtual Disk..."
./fsck virtual-disk

if diff temp/fuseLib.c mount-point/fuseLib.c
then
	true
else
	echo "fuseLib.c is different after truncate"
	exit 1
fi

if diff temp/myFS.h mount-point/myFS.h
then
	true
else
	echo "myFS.h is different after truncate"
	exit 1
fi

echo "Copying Makefile into /mount-point..."
cp Makefile mount-point/Makefile

echo "Checking Virtual Disk..."
./fsck virtual-disk

if diff Makefile mount-point/Makefile
then
	true
else
	echo "Makefile is different"
	exit 1
fi

echo "Truncanting myFS.h to one block more..."
truncate -s +1 -o temp/myFS.h
truncate -s +1 -o mount-point/myFS.h

echo "Checking virtual disk..."
./fsck virtual-disk

if diff temp/myFS.h mount-point/myFS.h
then
	true
else
	echo "myFS.h is different after the second truncate."
	exit 1
fi

echo "Everything OK!"