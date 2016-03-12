#PBS -q cs6210
#PBS -l nodes=1:fourcore
#PBS -l walltime=00:20:00
#PBS -N kk_job_omp
for i in 4 16
do 
	for j in 2 3 4 5 6 7 8 9 10 11 12
	do 
		$HOME/aos/omp_barrier/omp_mcs -n $j -ary $i > $HOME/aos/omp_barrier/log_omp_mcs_$j\_arr$i.log
	done
done