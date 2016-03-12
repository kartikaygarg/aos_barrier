#!/bin/bash
#PBS -q cs6210
#PBS -l nodes=4:sixcore
#PBS -l walltime=00:20:00
#PBS -N kr_job_mpi
for i in 2 3 4 5 6 7 8 9 10 11 12
do 
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 4 $HOME/aos/combined/sense_diss -np 4 -nt $i > $HOME/aos/combined/log_sense_diss_np4_nt$i\_node4.log
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 4 $HOME/aos/combined/sense_tour -np 4 -nt $i > $HOME/aos/combined/log_sense_tour_np4_nt$i\_node4.log
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 4 $HOME/aos/combined/sense_sense -np 4 -nt $i > $HOME/aos/combined/log_sense_sense_np4_nt$i\_node4.log
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 4 $HOME/aos/combined/mcs_diss -np 4 -nt $i -ary 16 -wake_ary 4 > $HOME/aos/combined/log_mcs_arr16_diss_np4_nt$i\_node4.log
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 4 $HOME/aos/combined/mcs_tour -np 4 -nt $i -ary 16 -wake_ary 4 > $HOME/aos/combined/log_mcs_arr16_tour_np4_nt$i\_node4.log
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 4 $HOME/aos/combined/mcs_sense -np 4 -nt $i -ary 16 -wake_ary 4 > $HOME/aos/combined/log_mcs_arr16_sense_np4_nt$i\_node4.log
done
