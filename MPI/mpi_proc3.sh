#!/bin/bash
#PBS -q cs6210
#PBS -l nodes=3:sixcore
#PBS -l walltime=00:20:00
#PBS -N kr_job_mpi
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 3 $HOME/aos/mpi_barrier/mpi_diss -n 3 > $HOME/aos/mpi_barrier/log_mpi_diss_3.log
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 3 $HOME/aos/mpi_barrier/mpi_tour -n 3 > $HOME/aos/mpi_barrier/log_mpi_tour_3.log
/opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 3 $HOME/aos/mpi_barrier/mpi_sense -n 3 > $HOME/aos/mpi_barrier/log_mpi_sense_3.log