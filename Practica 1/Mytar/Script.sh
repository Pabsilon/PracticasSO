#!/bin/bash

clear

if [ ! -x "mytar" ]
then 
	echo "Mytar is not executable"
	exit 1
fi

if [ -d "tmp" ]
then
	rm -rf tmp
fi

mkdir tmp

echo "Hello World!" > ./tmp/file1.txt

head /etc/passwd > ./tmp/file2.txt 

head -c 1024 /dev/random > ./tmp/file3.dat

cd ./tmp
../mytar -cf filetar.mtar file1.txt file2.txt file3.dat
cd ..

if [ ! -d "out" ]
then
	mkdir out
fi

cd out
../mytar -xf ../tmp/filetar.mtar

if diff ../tmp/file1.txt file1.txt > /dev/null 
then 
	true
else
	cd ..
	echo "file1.txt is different"
	exit 1
fi

if diff ../tmp/file2.txt file2.txt > /dev/null
then 
	true
else
	cd ..
	echo "file2.txt is different"
	exit 1
fi

if diff ../tmp/file3.dat file3.dat > /dev/null 
then 
	true
else
	cd ..
	echo "file3.dat is different"
	exit 1
fi

cd ..

echo "Correct"
exit 0
