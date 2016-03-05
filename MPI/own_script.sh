#PBS -q cs6210
#PBS -l nodes=4:sixcore
#PBS -l walltime=00:15:00
#PBS -N kr_job
OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 48 $HOME/aos/mpi_barrier/mpi_sense > $HOME/aos/mpi_barrier/log_mpi_sense.log
OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 48 $HOME/aos/mpi_barrier/mpi_diss > $HOME/aos/mpi_barrier/log_mpi_diss.log
OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 48 $HOME/aos/mpi_barrier/mpi_tour > $HOME/aos/mpi_barrier/log_mpi_tour.log
OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np 48 $HOME/aos/mpi_barrier/openmp_mpi > $HOME/aos/mpi_barrier/log_openmp_mpi.log
