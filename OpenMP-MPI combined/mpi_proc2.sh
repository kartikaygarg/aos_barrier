#!/bin/bash
#PBS -q cs6210
#PBS -l nodes=2:sixcore
#PBS -l walltime=00:20:00
#PBS -N kr_job_mpi
for i in 2 3 4 5 6 7 8 9 10 11 12
do 
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 2 $HOME/aos/combined/sense_diss -np 2 -nt $i > $HOME/aos/combined/log_sense_diss_np2_nt$i\_node2.log
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 2 $HOME/aos/combined/sense_tour -np 2 -nt $i > $HOME/aos/combined/log_sense_tour_np2_nt$i\_node2.log
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 2 $HOME/aos/combined/sense_sense -np 2 -nt $i > $HOME/aos/combined/log_sense_sense_np2_nt$i\_node2.log
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 2 $HOME/aos/combined/mcs_diss -np 2 -nt $i -ary 16 -wake_ary 2 > $HOME/aos/combined/log_mcs_arr16_diss_np2_nt$i\_node2.log
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 2 $HOME/aos/combined/mcs_tour -np 2 -nt $i -ary 16 -wake_ary 2 > $HOME/aos/combined/log_mcs_arr16_tour_np2_nt$i\_node2.log
	OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 2 $HOME/aos/combined/mcs_sense -np 2 -nt $i -ary 16 -wake_ary 2 > $HOME/aos/combined/log_mcs_arr16_sense_np2_nt$i\_node2.log
done
