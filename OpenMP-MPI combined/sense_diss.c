#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>

void mpi_disseminate(int P)
{
  MPI_Status status_array;
  // P = num_processes;
  int i, rank, k;
  int n = 0; 
  int source_buffer, dest_buffer;                                    // the round number starting from 0th round
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
    k = (int)ceil(log2(P));
    //int send_id[k];
    //int recv_id[k];
	
	int* send_id;
	int* recv_id;	
    send_id = (int*) calloc(k, sizeof(int));	
    recv_id = (int*) calloc(k, sizeof(int));

  int temp;
  for(i=0;i<k;i++)
  {  
     temp = (int) pow(2,n);
     send_id[i] = ((rank + temp)% P );
     recv_id[i] = (rank - temp >= 0) ? ((rank - temp)% P ) : ((P+ rank - temp)% P ) ;
     n++;
  }
  
  for(i = 0; i < k; i++)
	{
		  // printf("\nRank %d sending to id: %d\t || Round: %d",rank, send_id[i],i);
          MPI_Send(&source_buffer, 1, MPI_INT, send_id[i], 1, MPI_COMM_WORLD);
          // printf("\nRank %d receiving from id: %d\t || Round: %d",rank, recv_id[i],i);
		  MPI_Recv(&dest_buffer, 1, MPI_INT, recv_id[i], 1, MPI_COMM_WORLD, &status_array);
	}
  
}

void omp_sense(int num_of_threads, int* count, bool* local_sense, bool* global_sense, int num_proc, int rank, int thread_id){
	*local_sense = !(*local_sense);
	if(__sync_fetch_and_sub(count,1) == 1){
		*count = num_of_threads;
		// mpi_tournament_initialize(num_proc,rank, rounds);	
		// mpi_tournament(num_proc,rank, thread_id, rounds);
		// mpi_tournament_finalize(rounds);
		*global_sense = *local_sense;
	}
	while(*local_sense != *global_sense){}	
}

void comb_sense_diss(int num_of_threads, int* count, bool* local_sense, bool* global_sense, int num_proc, int rank, int thread_id){
	*local_sense = !(*local_sense);
	if(__sync_fetch_and_sub(count,1) == 1){
		*count = num_of_threads;
		mpi_disseminate(num_proc);
		// mpi_tournament_initialize(num_proc,rank, rounds);	
		// mpi_tournament(num_proc,rank, thread_id, rounds);
		// mpi_tournament_finalize(rounds);
		*global_sense = *local_sense;
	}
	while(*local_sense != *global_sense){}	
}

int main(int argc,char **argv){
	int num_proc, rc, i, num_of_threads;
	int ptr = 1;
	// MPI_Status* status_array;
	// int P;
	int rank,thread_num;
	int count;
	// rounds_t *rounds;
	// rounds = NULL;
	//double timer1,timer2;
	
	// int thread_id;
	struct timeval timer1,timer2;
	double timer_pad=0,timer_fin=0;
	bool local_sense, global_sense;
	global_sense = true;
	local_sense = true;
	// char flag1=0;//,flag2=0;
	
	rc= MPI_Init(&argc, &argv);
	if (rc != MPI_SUCCESS)
	{
		printf("Failure initializing MPI\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	} 
	if(argc > 1){
		for(ptr = 1; ptr < argc; ++ptr){
			if (argv[ptr][0] == '-'){
				if (!strcmp(argv[ptr], "-np") || !strcmp(argv[ptr], "-num_proc")) {
					num_proc = atoi(argv[++ptr]);
				}	
				else if (!strcmp(argv[ptr], "-nt") || !strcmp(argv[ptr], "-num_th")) {
					num_of_threads = atoi(argv[++ptr]);
				}
			}
		}
	}
	else{		//Num of threads not passed | Default no of threads = 8
		num_proc = 2;
		num_of_threads = 4;
	}
	// elapsed.tv_sec = 0;
	// elapsed.tv_usec = 0;
	timer_pad = 0;
	timer_fin = 0;
	// printf("\nORIGINALLY || Number of proc in MPI: %d", num_proc);
	//num_of_threads = (num_proc*num_of_threads);
	
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// status_array = (MPI_Status*) malloc( num_proc * sizeof(MPI_Status));
	// rounds = (rounds_t*) calloc((round_count+1) * num_proc, sizeof(rounds_t));
	// printf("\nNumber of proc in MPI: %d", num_proc);
	// num_proc = num_proc/2;
	//mpi_tournament_initialize(num_proc,rank);
	thread_num = 0;
	for(i=0;i<1000;++i){
		for(ptr=0;ptr<1000;++ptr){
			timer_pad = 0;
			// flag1 = 0;
			count = num_of_threads;
			#pragma omp parallel num_threads(num_of_threads) firstprivate(thread_num, local_sense, timer1, timer2) shared(global_sense, count, num_proc, rank) reduction(+:timer_pad)
			{
				thread_num = omp_get_thread_num();		
				// printf("\nHello from thread: %d for Round: %d || Rank: %d ",thread_num,ptr,rank);
				// fflush(stdout);
				gettimeofday(&timer1, NULL);
				comb_sense_diss(num_of_threads, &count, &local_sense, &global_sense, num_proc, rank, thread_num);
				
				// omp_sense(num_of_threads, &count, &local_sense, &global_sense, num_proc, rank, thread_num);
				// if(thread_num ==0)
				// { 
					// printf("\nValue of P is: %d",P);
					// mpi_disseminate(num_proc);
				
				// }
				// omp_sense(num_of_threads, &count, &local_sense, &global_sense, num_proc, rank, thread_num);
				// #pragma omp critical
				// {   
					// if(__sync_fetch_and_add(&flag1,1) == 0){
						// flag1 = 1;
						// mpi_tournament_initialize(num_proc,rank);	
						// mpi_tournament(num_proc,rank);
						// mpi_tournament_finalize();
						// flag1=2;
					// }
					// while(flag1!=2){}
					
				// }
			/*	#pragma omp master 
				{
					mpi_tournament_initialize(num_proc,rank, rounds);	
					mpi_tournament(num_proc,rank, thread_num, rounds);
					mpi_tournament_finalize(rounds);
				}
			*/	
				gettimeofday(&timer2, NULL);
				// printf("\nFinish from thread: %d for Round: %d || Rank: %d",thread_num,ptr,rank);
				// fflush(stdout);
				timer_pad += ( (timer2.tv_sec - timer1.tv_sec) * 1000000) + (timer2.tv_usec - timer1.tv_usec);
			}	
			timer_fin += timer_pad;
		}
	}
	//thread_num = 0;
	//mpi_tournament_finalize();
	MPI_Finalize();
	printf("\n\n-------------------------\n\n");
	
	printf("\nTotal time: %f microsec\n----------------------\n",timer_fin);
	timer_fin = timer_fin/1000000;
	printf("\nAverage time: %f microsec",timer_fin);
	return 0;
}