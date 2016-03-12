#PBS -q cs6210
#PBS -l nodes=1:fourcore
#PBS -l walltime=00:20:00
#PBS -N kk_job_omp
for j in 2 3 4 5 6 7 8
do 
	$HOME/aos/omp_barrier/omp_sense -n $j > $HOME/aos/omp_barrier/log_omp_sense_4core_$j.log
done