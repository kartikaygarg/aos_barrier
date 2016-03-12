#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>




typedef enum {
	winner,
	loser,
	bye,
	champion,
	dropout
} role_t;

typedef struct {
	role_t role;
	int opponent;
	int send_buf;
	int recv_buf;
	MPI_Status status_msg;
} rounds_t;

// rounds_t *rounds;

// Initialization function
void mpi_tournament_initialize(int P, int rank, rounds_t* rounds)
{
	int round_count;
	int k;
//	int power2k = 1;
//	int power2k_prev;
	
	// MPI_Comm_size(MPI_COMM_WORLD,&P);
    // MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	round_count = (int) ceil(log2(P));
	//rounds_t* rounds;
    //rounds = (rounds_t*) calloc((round_count+1) * P, sizeof(rounds_t));
	if(rounds == NULL){
		printf("\nAllocation FAILS.");
		exit(0);
	}
	// Assign roles and opponents to each level of this process
    int temp, pre_temp;
	for(k = 0; k <= round_count; k++)
    {    
        temp = (int)pow(2,k);
        pre_temp = (int)pow(2,k-1);
    	rounds[(rank*(round_count+1) )+k].send_buf = 1;
    	if(k == 0)
    		rounds[(rank*(round_count+1) )+k].role = dropout;
    	else
    	{
    		if ((rank % temp == 0) && (rank + pre_temp >= P) )
    		   	{
    				//if(rank + power2k_prev >= P)
    				rounds[(rank*(round_count+1) )+k].role = bye;
    	             	}
    	        else if ((rank % temp == 0) && (rank + pre_temp < P) && ( temp < P) ) 
    			{
    					rounds[(rank*(round_count+1) )+k].role = winner;
    					rounds[(rank*(round_count+1) )+k].opponent = rank + pre_temp;
    		     	}	
    		else if(rank % temp == pre_temp)
    		    	{
    			 	rounds[(rank*(round_count+1) )+k].role = loser;
    				rounds[(rank*(round_count+1) )+k].opponent = rank - pre_temp; 
    			}
    		else if(rank == 0 && temp >= P)
    			{
    				rounds[(rank*(round_count+1) )+k].role = champion;
    				rounds[(rank*(round_count+1) )+k].opponent = rank + pre_temp;
    			}
    	}
    	//power2k_prev = power2k;
    	//power2k = power2k * 2;
    }
}


//main barrier


void mpi_tournament(int P, int rank, int thread_id, rounds_t* rounds)
{
	int round_count;
	round_count = (int)ceil(log2(P));
	int k = 0;
	int temp;
	int arrival = 0;
	// printf("\nTournament called by Thread: %d of rank: %d",thread_id,rank);
	temp = rank;
	//MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// printf("\nNew rank: %d || for True rank: %d",rank, temp);
	while(!arrival)
	{
		++k;
		switch(rounds[(rank*(round_count+1) )+k].role)
		{
		case loser:
		MPI_Send(&(rounds[(rank*(round_count+1) )+k].send_buf), 1, MPI_INT, rounds[(rank*(round_count+1) )+k].opponent, k, MPI_COMM_WORLD);
		MPI_Recv(&rounds[(rank*(round_count+1) )+k].recv_buf, 1, MPI_INT, rounds[(rank*(round_count+1) )+k].opponent, k, MPI_COMM_WORLD, &rounds[(rank*(round_count+1) )+k].status_msg);
				arrival = 1;
				break;
		case winner:
		MPI_Recv(&rounds[(rank*(round_count+1) )+k].recv_buf, 1, MPI_INT, rounds[(rank*(round_count+1) )+k].opponent, k, MPI_COMM_WORLD, &rounds[(rank*(round_count+1) )+k].status_msg);
				break;
		case champion:
		MPI_Recv(&rounds[(rank*(round_count+1) )+k].recv_buf, 1, MPI_INT, rounds[(rank*(round_count+1) )+k].opponent, k, MPI_COMM_WORLD, &rounds[(rank*(round_count+1) )+k].status_msg);
		MPI_Send(&(rounds[(rank*(round_count+1) )+k].send_buf), 1, MPI_INT,rounds[(rank*(round_count+1) )+k].opponent, k, MPI_COMM_WORLD);
				arrival = 1;
				break;
		case bye:
				continue;
		case dropout:
				// printf("impossible!!!\n");
				continue;
		}
	}
	
	int wakeup = 0;
	while(!wakeup)
	{
		--k;
		switch(rounds[(rank*(round_count+1) )+k].role)
		{
			case winner:
				MPI_Send(&(rounds[(rank*(round_count+1) )+k].send_buf), 1, MPI_INT, rounds[(rank*(round_count+1) )+k].opponent, k, MPI_COMM_WORLD);
				continue;
			case dropout:
				wakeup = 1;
				break;
			case loser:			// impossible
				// printf("impossible!!!\n");
				continue;
			case bye:			// do nothing
			 	continue;
			case champion:		// impossible
				// printf("impossible!!!\n");
				continue;
		}
	}
}


void mpi_tournament_finalize(rounds_t* rounds){
	if(rounds != NULL)
		free(rounds);
}

void comb_sense_tour(int num_of_threads, int* count, bool* local_sense, bool* global_sense, int num_proc, int rank, int thread_id, rounds_t* rounds){
	*local_sense = !(*local_sense);
	if(__sync_fetch_and_sub(count,1) == 1){
		*count = num_of_threads;
		// mpi_sense_initialize(num_proc, status_array);
		mpi_tournament(num_proc,rank, thread_id, rounds);
		// mpi_sense_finalize(status_array);
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
	//rounds = NULL;
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
	int round_count;
	round_count = (int)ceil(log2(num_proc));
	rounds_t* rounds;
	rounds = (rounds_t*) calloc((round_count+1) * num_proc, sizeof(rounds_t));
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// status_array = (MPI_Status*) malloc( num_proc * sizeof(MPI_Status));
	// rounds = (rounds_t*) calloc((round_count+1) * num_proc, sizeof(rounds_t));
	// printf("\nNumber of proc in MPI: %d", num_proc);
	// num_proc = num_proc/2;
	//mpi_tournament_initialize(num_proc,rank);
	thread_num = 0;
	mpi_tournament_initialize(num_proc,rank, rounds);
	for(i=0;i<1000;++i){
		for(ptr=0;ptr<1000;++ptr){
			timer_pad = 0;
			// flag1 = 0;
			count = num_of_threads;
			#pragma omp parallel num_threads(num_of_threads) firstprivate(thread_num, local_sense, timer1, timer2) shared(global_sense, count, num_proc, rank, rounds) reduction(+:timer_pad)
			{
				thread_num = omp_get_thread_num();		
				// printf("\nHello from thread: %d for Round: %d || Rank: %d ",thread_num,ptr,rank);
				// fflush(stdout);
				gettimeofday(&timer1, NULL);
				
				comb_sense_tour(num_of_threads, &count, &local_sense, &global_sense, num_proc, rank, thread_num, rounds);
				
				
				gettimeofday(&timer2, NULL);
				// printf("\nFinish from thread: %d for Round: %d || Rank: %d",thread_num,ptr,rank);
				// fflush(stdout);
				timer_pad += ( (timer2.tv_sec - timer1.tv_sec) * 1000000) + (timer2.tv_usec - timer1.tv_usec);
			}	
			timer_fin += timer_pad;
		}
	}
	//thread_num = 0;
	mpi_tournament_finalize(rounds);
	MPI_Finalize();
	printf("\n\n-------------------------\n\n");
	
	printf("\nTotal time: %f microsec\n----------------------\n",timer_fin);
	timer_fin = timer_fin/1000000;
	printf("\nAverage time: %f microsec",timer_fin);
	return 0;
}