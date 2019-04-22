#!/bin/bash

for f in $(seq 1.2 0.1 3.0)
do
	echo $f >> result
	sudo cpupower frequency-set -f ${f}Ghz
	ssh -f mytacc2 "cpupower frequency-set -f ${f}Ghz"
	sleep 5
	mpirun -np 24 -f host_file_tacc -ppn 12 ./a.out 1>> result
done
