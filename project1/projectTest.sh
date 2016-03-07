#!/bin/bash
# Author: Mengwen Li (mli2)
# create DUMPSTER path first on test machine.
export DUMPSTER=/home/mengwen/Desktop/dumpster
totalRenameTime=0
# copy 20 version of test file.
for i in {1..20}
	do
		cp testRename "testRename$i"
	done
# remove these test files and time.
for i in {1..20}
	do
		sync
		# measure the first time.
		t1=$(date +%s%3N)
		./rm "testRename$i"
		sync
		# measure the second time.
		t2=$(date +%s%3N)
		# calculate the time used.
		timeUse=$((t2-t1))
		# add to total time.
		totalRenameTime=$((totalRenameTime+timeUse))
		# print out the time used for one run.
		echo "$timeUse" milliseconds
	done
# print out the total time needed and the average time.
echo "total rename time in ms is " "$totalRenameTime" "milliseconds"
echo "average rename time in ms is" $((totalRenameTime/20)) "milliseconds"
# empty the dumpster.
./dump

# measure copy a big file from another partition.
totalCopyTime=0
# get the size of the test file.
filename=/media/mengwen/OS/testCopy
filesize=$(stat -c%s "$filename")
# print out the size.
echo "Size of $filename = $filesize bytes."
# copy 20 version of test file.
for i in {1..20}
	do
		cp /media/mengwen/OS/testCopy "/media/mengwen/OS/testCopy$i"
	done
# remove these test files and time.
for i in {1..20}
	do
		sync
		# get the start time.
		t1=$(date +%s%3N)
		./rm "/media/mengwen/OS/testCopy$i"
		sync
		# get the finish time.
		t2=$(date +%s%3N)
		# get the time used.
		timeUse=$((t2-t1))
		# add to the total time.
		totalCopyTime=$((totalCopyTime+timeUse))
		# print out the time used for one run.
		echo "$timeUse" milliseconds
	done
# print out the total time, average time and calculate the throughput.
echo "total copy time in ms is " "$totalCopyTime" "milliseconds"
echo "average copy time in ms is " $((totalCopyTime/20)) "milliseconds"
echo "throughput is " $(((filesize*20) / (totalCopyTime/1000))) "bytes/second"
# empty the dumpster.
./dump

# measure copy a big directory across partition.
# get the size of the directory.
filename=/media/mengwen/OS/testFolder
filesize=$(du -sb $filename | cut -f1)
echo "Size of $filename = $filesize bytes."

totalFolderTime=0
# copy 10 copies of test directory.
for i in {1..10}
	do
		cp -rf /media/mengwen/OS/testFolder "/media/mengwen/OS/testFolder$i"
	done
# remove these directories and time.
for i in {1..10}
	do
		sync
		# get the start time.
		t1=$(date +%s%3N)
		./rm -r "/media/mengwen/OS/testFolder$i"
		sync
		# get the end time.
		t2=$(date +%s%3N)
		# get the time used.
		timeUse=$((t2-t1))
		# add to the total time.
		totalFolderTime=$((totalFolderTime+timeUse))
		# print out the time used for a single run.
		echo "$timeUse" milliseconds
	done
# print out the total time and the average time.
echo "total folder copy time in ms is " "$totalFolderTime" "milliseconds"
echo "average folder copy time in ms is " $((totalFolderTime/10)) "milliseconds"
# empty the dumpster.
./dump


