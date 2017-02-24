#!/bin/bash

# exit if anything fails
set -e

make clean
make -f Makefile

# don't exit if any of the testers fail
set +e

for i in `seq 1 11`;
do
echo "Test $i running"
./mcontest testers_exe/tester-$i
echo "Test $i completed."
done

echo "All test completed."
