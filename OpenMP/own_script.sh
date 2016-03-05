#PBS -q cs6210
#PBS -l nodes=1:fourcore
#PBS -l walltime=00:05:00
#PBS -N kk_job
$HOME/aos/omp_barrier/omp_sense -n 4 > $HOME/aos/omp_barrier/log_omp_sense_4.log
$HOME/aos/omp_barrier/omp_sense -n 6 > $HOME/aos/omp_barrier/log_omp_sense_6.log
$HOME/aos/omp_barrier/omp_sense -n 8 > $HOME/aos/omp_barrier/log_omp_sense_8.log
$HOME/aos/omp_barrier/omp_mcs > $HOME/aos/omp_barrier/log_omp_mcs.log