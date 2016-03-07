#!/bin/bash
# Create 5 random files with size 1 to 10 MB.
filesize=1000000
for i in {1..10}
do
	base64 /dev/urandom | head -c $((filesize*i)) > testFile$i
done
# Generate a 10MB file called "tpTest" to test throughput.
base64 /dev/urandom | head -c $((10000000)) > tpTest
