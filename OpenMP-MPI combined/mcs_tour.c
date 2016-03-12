#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>

typedef struct _node{
	 // volatile bool parent_sense;
	bool* parent_sense;
	bool local_sense;
	bool* have_child;
	bool* parent_ptr;
	// volatile bool* child_NotReady;			//False denotes: child is not ready || TRUE: denotes child has enetered barrier and is ready
	bool* child_NotReady;
	bool* child_ptr;
}node;

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

	round_count = (int) ceil(log2(P));

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
    	
    }
}

void mpi_tournament(int P, int rank, rounds_t* rounds)
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

void omp_mcs_init(node* all_nodes, int thread_id, int ary, int arrv_ary, int num_of_threads){
		int temp=0,i=0;
		int LEVEL1_DCACHE_LINESIZE = 64;
		
		all_nodes[thread_id].local_sense = false;
		
		posix_memalign((void *)(&(all_nodes[thread_id].have_child)), LEVEL1_DCACHE_LINESIZE, LEVEL1_DCACHE_LINESIZE);	
		posix_memalign((void *)(&(all_nodes[thread_id].child_NotReady)), LEVEL1_DCACHE_LINESIZE, LEVEL1_DCACHE_LINESIZE);	
		posix_memalign((void*)(&(all_nodes[thread_id].child_ptr)), LEVEL1_DCACHE_LINESIZE, LEVEL1_DCACHE_LINESIZE);	
		temp = (thread_id*ary)+1;
		if(temp >= num_of_threads){
			i=0;
		}
		else{
			for (i=0; (temp < ((thread_id*ary)+ary+1)) && (temp < num_of_threads);++temp){			//Set the child pointer values
				//Set to TRUE if has a child
				all_nodes[thread_id].have_child[i] = true;
				all_nodes[thread_id].child_NotReady[i] = false;
				++i;
			}
			while(temp < ((thread_id*ary)+ary+1) ){			//Invalidate the non-child ptrs
				//set the entry in has_child to FALSE
				all_nodes[thread_id].have_child[i] = false;
				all_nodes[thread_id].child_NotReady[i] = false;
				++temp;
				++i;
			}
		}
		while(i < LEVEL1_DCACHE_LINESIZE/sizeof(bool)){			//Initializing the zombie place holders of ptr
			all_nodes[thread_id].have_child[i] = false;
			all_nodes[thread_id].child_NotReady[i] = false;
			++i;
		}
		if(thread_id != 0){			//Not the root node
			all_nodes[thread_id].parent_ptr = &(all_nodes[(thread_id-1)/ary].child_NotReady[(thread_id-1)%ary]); 
			all_nodes[thread_id].parent_sense = &(all_nodes[(thread_id-1)/arrv_ary].child_ptr[(thread_id-1)%arrv_ary]);
		}
		else{
			all_nodes[thread_id].parent_ptr = NULL;
			all_nodes[thread_id].parent_sense = (bool*) malloc(sizeof(bool));
			*(all_nodes[thread_id].parent_sense) = all_nodes[thread_id].local_sense;

		}
		temp = ((thread_id*2)+1);
		i=0;
		
		for(i=0;i<arrv_ary;++i){
			all_nodes[thread_id].child_ptr[i] = false;
		}
}

void comb_mcs_tour(int ary,int arrv_ary, node* all_nodes,int thread_id, int num_proc, int rank, rounds_t* rounds){
	int i=0;
	// int j=0;
	// int flag=0;
	all_nodes[thread_id].local_sense = !(all_nodes[thread_id].local_sense);
	
	while( (all_nodes[thread_id].have_child[i] == true) && (i<ary) ){
		while(all_nodes[thread_id].child_NotReady[i] == false){}
		++i;
	}
	i=0;
	for(i=0;((all_nodes[thread_id].have_child[i] == true) && (i<ary) );++i){
		all_nodes[thread_id].child_NotReady[i] = false;
	}
	if(thread_id == 0){
		*(all_nodes[thread_id].parent_sense) = !(*(all_nodes[thread_id].parent_sense));
		mpi_tournament(num_proc,rank, rounds);
	}
	else{
		*(all_nodes[thread_id].parent_ptr) = true;
		while(all_nodes[thread_id].local_sense != *(all_nodes[thread_id].parent_sense)){}
	}
	i=0;
	
	for(i=0;( ( all_nodes[thread_id].child_ptr[i] != all_nodes[thread_id].local_sense) && (i<arrv_ary) );++i){	
		all_nodes[thread_id].child_ptr[i] = !(all_nodes[thread_id].child_ptr[i]);
	 }
	
}

void omp_mcs_fin(node* all_nodes, int thread_id)
{
		free(all_nodes[thread_id].have_child);
		free(all_nodes[thread_id].child_NotReady);
		free(all_nodes[thread_id].child_ptr);
}

int main(int argc,char **argv){
	int num_proc, rc, i, num_of_threads;
	int ptr = 1;
	// MPI_Status* status_array;
	// int P;
	int rank,thread_num;
	int ary, arrv_ary=2;
	// int count;
	rounds_t *rounds = NULL;
	// rounds = NULL;
	//double timer1,timer2;
	
	// int thread_id;
	struct timeval timer1,timer2;
	double timer_pad=0,timer_fin=0;
	int round_count;
		
	node* all_nodes = NULL;
	
	// bool local_sense, global_sense;
	// global_sense = true;
	// local_sense = true;
	// char flag1=0;//,flag2=0;
	
	arrv_ary = 2;
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
				else if (!strcmp(argv[ptr], "-ary")) {
					ary = atoi(argv[++ptr]);
				}
				else if (!strcmp(argv[ptr], "-wake_ary")) {
					arrv_ary = atoi(argv[++ptr]);
				}
			}
		}
	}
	else{		//Num of threads not passed | Default no of threads = 8
		num_proc = 2;
		num_of_threads = 4;
		ary = 4;
		arrv_ary = 2;
	}
	
	timer_pad = 0;
	timer_fin = 0;
	
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// status_array = (MPI_Status*) malloc( num_proc * sizeof(MPI_Status));
	
	round_count = (int)ceil(log2(num_proc));
	rounds = (rounds_t*) calloc((round_count+1) * num_proc, sizeof(rounds_t));
	
	mpi_tournament_initialize(num_proc,rank,rounds);
	thread_num = 0;
	all_nodes = (node*) malloc(num_of_threads* sizeof(node));
	
	for(ptr=0;ptr<num_of_threads;++ptr){
		omp_mcs_init(all_nodes,ptr, ary, arrv_ary, num_of_threads);
	}
	
	for(i=0;i<1000;++i){
		for(ptr=0;ptr<1000;++ptr){
			timer_pad = 0;
			// flag1 = 0;
			// count = num_of_threads;
			#pragma omp parallel num_threads(num_of_threads) firstprivate(thread_num, timer1, timer2) shared(num_proc, rounds, rank, all_nodes, ary, arrv_ary) reduction(+:timer_pad)
			{
				thread_num = omp_get_thread_num();		
				// printf("\nHello from thread: %d for Round: %d || Rank: %d ",thread_num,ptr,rank);
				// fflush(stdout);
				
				gettimeofday(&timer1, NULL);
				
				comb_mcs_tour(ary, arrv_ary, all_nodes, thread_num, num_proc, rank, rounds);
			
				gettimeofday(&timer2, NULL);
				// printf("\nFinish from thread: %d for Round: %d || Rank: %d",thread_num,ptr,rank);
				// fflush(stdout);
				timer_pad += ( (timer2.tv_sec - timer1.tv_sec) * 1000000) + (timer2.tv_usec - timer1.tv_usec);
			}	
			timer_fin += timer_pad;
		}
	}
	for(ptr=0;ptr<num_of_threads;++ptr){
		omp_mcs_fin(all_nodes,ptr);	
	}
	free(all_nodes);
	//mpi_tournament_finalize();
	MPI_Finalize();
	printf("\n\n-------------------------\n\n");
	
	printf("\nTotal time: %f microsec\n----------------------\n",timer_fin);
	timer_fin = timer_fin/1000000;
	printf("\nAverage time: %f microsec",timer_fin);
	return 0;
}