#!/bin/bash
#PBS -q cs6210
#PBS -l nodes=12:sixcore
#PBS -l walltime=00:20:00
#PBS -N kr_job_mpi
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 12 $HOME/aos/mpi_barrier/mpi_diss -n 12 > $HOME/aos/mpi_barrier/log_mpi_diss_12.log
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 12 $HOME/aos/mpi_barrier/mpi_tour -n 12 > $HOME/aos/mpi_barrier/log_mpi_tour_12.log
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 12 $HOME/aos/mpi_barrier/mpi_sense -n 12 > $HOME/aos/mpi_barrier/log_mpi_sense_12.log