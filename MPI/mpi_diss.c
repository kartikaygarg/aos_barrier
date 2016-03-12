#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>

static int P;

// void mpi_disseminate_initialize()
// {
   // status_array = (MPI_Status*) malloc((P) * sizeof(MPI_Status));
// }

void mpi_disseminate(int num_processes)
{
  MPI_Status status_array;
  P = num_processes;
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


// void mpi_disseminate_finalize(){
  // if(status_array != NULL){
    // free(status_array);
  // }
// }


int main(int argc, char **argv)
{
  int num_processes, rc;//, i;
  int ptr = 1;
  int rank;
  double *t1;
  double *t2;
  // double timer1,timer2;
  double timer_pad=0;
    MPI_Status status_msg;
  rc= MPI_Init(&argc, &argv);
 
  	if (rc != MPI_SUCCESS)
	{
		printf("Failure initializing MPI\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	} 
	num_processes = 16;
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
	num_processes = P;
	
  MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
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
  
  for(rc=0;rc<1000;++rc){
	for(ptr=0;ptr<1000;++ptr){
		// mpi_disseminate_initialize();
		 // printf("Hello for Round: %d from rank: %d\n",ptr,rank);
		 //fflush(stdout);
		// timer1 = MPI_Wtime();
		t1[ptr] = MPI_Wtime();
		mpi_disseminate(num_processes);
		t2[ptr] = MPI_Wtime();
		// timer2 = MPI_Wtime();
		timer_pad += t2[ptr] - t1[ptr];
		 // printf("\nFinish for Round: %d from: %d\n",ptr,rank);
		 //fflush(stdout);
		// mpi_disseminate_finalize();		
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