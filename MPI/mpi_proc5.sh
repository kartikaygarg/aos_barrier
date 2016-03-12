#!/bin/bash
#PBS -q cs6210
#PBS -l nodes=5:sixcore
#PBS -l walltime=00:20:00
#PBS -N kr_job_mpi
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 5 $HOME/aos/mpi_barrier/mpi_diss -n 5 > $HOME/aos/mpi_barrier/log_mpi_diss_5.log
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 5 $HOME/aos/mpi_barrier/mpi_tour -n 5 > $HOME/aos/mpi_barrier/log_mpi_tour_5.log
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 5 $HOME/aos/mpi_barrier/mpi_sense -n 5 > $HOME/aos/mpi_barrier/log_mpi_sense_5.log