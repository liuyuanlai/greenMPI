#!/bin/bash

for f in $(seq 1.2 0.1 3.0)
do
	echo $f >> result_np2
	cpupower frequency-set -f ${f}Ghz
	ssh -f mytacc2 "cpupower frequency-set -f ${f}Ghz"
	sleep 5
	mpirun -np 2 -f host_file_tacc -ppn 1 ./a.out 1>> result_np2
done
