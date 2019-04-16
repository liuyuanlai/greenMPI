#!/bin/bash

for f in $(seq 1.2 0.1 3.0)
do
	echo $f >> result
	sudo cpupower frequency-set -f ${f}Ghz
	ssh -f myuc2 "sudo cpupower frequency-set -f ${f}Ghz"
	sleep 2
	mpirun -np 2 -f host_file ./a.out 1>> result
done
