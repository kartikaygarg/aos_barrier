#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

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

rounds_t *rounds;

// Initialization function

void mpi_tournament_initialize(int P, int rank)
{
	int round_count;
	int k;
//	int power2k = 1;
//	int power2k_prev;
	
	// MPI_Comm_size(MPI_COMM_WORLD,&P);
    // MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	round_count = (int) ceil(log2(P));
	//round_t* rounds;
        rounds = (rounds_t*) calloc((round_count+1) * P, sizeof(rounds_t));
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

void mpi_tournament_finalize(){
	if(rounds != NULL)
		free(rounds);
}

void mpi_tournament(int P, int rank)
{
	int round_count;
	round_count = (int)ceil(log2(P));
	int k = 0;
	int arrival = 0;
	while(!arrival)
	{
		k++;
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
				printf("impossible!!!\n");
				continue;
		}
	}
	
	
	int wakeup = 0;
	while(!wakeup)
	{
		k--;
		switch(rounds[(rank*(round_count+1) )+k].role)
		{
			case winner:
				MPI_Send(&(rounds[(rank*(round_count+1) )+k].send_buf), 1, MPI_INT, rounds[(rank*(round_count+1) )+k].opponent, k, MPI_COMM_WORLD);
				continue;
			case dropout:
				wakeup = 1;
				break;
			case loser:			// impossible
				printf("impossible!!!\n");
				continue;
			case bye:			// do nothing
			 	continue;
			case champion:		// impossible
				printf("impossible!!!\n");
				continue;
		}
	}
}



int main(int argc, char **argv)
{
  int rc;
  int P;
  double *t1;
  double *t2;
  	MPI_Status status_msg;
  // double timer1,timer2;
  double timer_pad=0;
  rc= MPI_Init(&argc, &argv);
  int rank, ptr = 0;
  	if (rc != MPI_SUCCESS)
	{
		printf("Failure initializing MPI\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	} 
	
	if(argc > 1){
		for(ptr = 1; ptr < argc; ++ptr){
			if (argv[ptr][0] == '-'){
				if (!strcmp(argv[ptr], "-n") || !strcmp(argv[ptr], "-num_proc")) {
					P = atoi(argv[++ptr]);
				}
			}
		}
	}
	else{		//Num of threads not passed | Default no of threads = 8
		P = 8;
	}
	
  //MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
  MPI_Comm_size(MPI_COMM_WORLD, &P);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if(rank == 0){
	  t1 = (double *) malloc((1000)*P*sizeof(double));
	  t2 = (double *) malloc((1000)*sizeof(double));
	}
	else{
		t1 = (double *) malloc((1000)*sizeof(double));
		t2 = (double *) malloc((1000)*sizeof(double));
	}
  timer_pad = 0;
  
  // mpi_tournament_initialize(P,rank);
  for(rc=0;rc<1000;++rc){
	for(ptr=0;ptr<1000;++ptr){
		mpi_tournament_initialize(P,rank);
		// printf("Hello for iteration: %d from rank: %d\n",ptr,rank);
		//fflush(stdout);
		t1[ptr] = MPI_Wtime();
		// printf("\t || Time (st %d): %f",ptr,timer1[ptr]);
		//fflush(stdout);
		mpi_tournament(P, rank);
		t2[ptr] = MPI_Wtime();
		timer_pad += t2[ptr] - t1[ptr];
		mpi_tournament_finalize();  
		// printf("\nFinish for iteration: %d from: %d\n",ptr,rank);
		//fflush(stdout);
	}
  }
  if(rank == 0){
	    for(ptr=1;ptr<P;++ptr){
			MPI_Recv(&t1[ptr*1000], 1000, MPI_DOUBLE, ptr, 1, MPI_COMM_WORLD, &status_msg);
		}
		for(ptr=0;ptr<1000;++ptr){
			for(rc=1;rc<P;++rc){
				if(t2[ptr]<=t1[(rc*1000)+ptr]){
					printf("ERROR in order!!!!");
				}
			}  
		}
		printf("\n\n-------------------------\n\n");
		printf("Total timer: %f || MASTER || from rank: %d\n",timer_pad,rank);
		timer_pad = timer_pad/1000000;
		printf("Avg timer: %f || MASTER || from rank: %d",timer_pad,rank);
   }
   else{
	   // for(ptr=1;ptr<P;++ptr){
			MPI_Send(t1, 1000, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD); 
		// }
		printf("\n\n-------------------------\n\n");
		printf("Total timer: %f from rank: %d\n",timer_pad,rank);
		timer_pad = timer_pad/1000000;
		printf("Avg timer: %f",timer_pad);
	}
	
  MPI_Finalize();
  return 0; 
}
