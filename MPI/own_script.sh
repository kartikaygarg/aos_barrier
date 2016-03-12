#!/bin/bash
#PBS -q cs6210
#PBS -l nodes=12:sixcore
#PBS -l walltime=00:20:00
#PBS -N kr_job_mpi
for i in 2 4 6 8 10 12
do 
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np $i $HOME/aos/mpi_barrier/mpi_diss -n $i > $HOME/aos/mpi_barrier/log_mpi_diss_$i.log
done
for i in 2 4 6 8 10 12
do 
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np $i $HOME/aos/mpi_barrier/mpi_tour -n $i > $HOME/aos/mpi_barrier/log_mpi_tour_$i.log
done
for i in 2 4 6 8 10 12
do 
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np $i $HOME/aos/mpi_barrier/mpi_sense -n $i > $HOME/aos/mpi_barrier/log_mpi_sense_$i.log
done