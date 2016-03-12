#!/bin/bash
for i in 2 3 4 5 6 7 8
do
	qsub mpi_proc$i.sh
done