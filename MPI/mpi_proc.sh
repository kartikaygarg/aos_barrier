#!/bin/bash
for i in 2 3 4 5 6 7 8 9 10 11 12
do
	qsub mpi_proc$i.sh
done