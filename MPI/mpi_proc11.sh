#!/bin/bash
#PBS -q cs6210
#PBS -l nodes=11:sixcore
#PBS -l walltime=00:20:00
#PBS -N kr_job_mpi
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 11 $HOME/aos/mpi_barrier/mpi_diss -n 11 > $HOME/aos/mpi_barrier/log_mpi_diss_11.log
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 11 $HOME/aos/mpi_barrier/mpi_tour -n 11 > $HOME/aos/mpi_barrier/log_mpi_tour_11.log
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 11 $HOME/aos/mpi_barrier/mpi_sense -n 11 > $HOME/aos/mpi_barrier/log_mpi_sense_11.log