#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
/*
    From the MCS Paper: A sense-reversing centralized barrier

    shared count : integer := P
    shared sense : Boolean := true
    processor private local_sense : Boolean := true

    procedure central_barrier
        local_sense := not local_sense // each processor toggles its own sense
	if fetch_and_decrement (&count) = 1
	    count := P
	    sense := local_sense // last processor toggles global sense
        else
           repeat until sense = local_sense
*/


static MPI_Status* status_array;
//MPI_Status* status_array;
static int P;
//int P;


void mpi_sense_initialize()
{
   status_array = (MPI_Status*) malloc( P * sizeof(MPI_Status));
}

void mpi_sense(int P){
  int i, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int count;
  bool global_sense = true;
  bool local_sense = true;
  
  if (rank==0)
  {
  MPI_Request reqs[P];
  count = P;
  count--;
  local_sense = !local_sense;
  for(i = 1; i < P; i++)
   {
    MPI_Send(&count, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
    MPI_Recv(&count, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status_array[i-1]);
    }
    global_sense = local_sense;
    for(i=1; i<P; i++)
     MPI_Isend(&global_sense, 1, MPI_CHAR , i, 1 , MPI_COMM_WORLD, &reqs[i-1]);
	 //MPI_Send(&global_sense, 1, MPI_CHAR , i, 1 , MPI_COMM_WORLD);
   }
   
   else 
   {
    MPI_Request reqs;  
    MPI_Status Status_msg;
    local_sense =!local_sense;
    MPI_Recv(&count, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &Status_msg);
    count--;
    MPI_Send(&count, 1, MPI_INT, 0, 1, MPI_COMM_WORLD); 
    //MPI_Irecv(&global_sense, 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &reqs);
//	printf("\tglobal: %d from rank: %d\t",global_sense,rank);
//	printf("\tlocal: %d  from rank: %d|| ",local_sense,rank);
	MPI_Recv(&global_sense, 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &Status_msg);
// v printf("\tglobal: %d from rank: %d",global_sense,rank);
    // printf("\tlocal: %d\t",local_sense);	
    // while(global_sense!=local_sense){
		// printf("\tSpin. %d",rank);
	// }
   }
 //   for (i=0;i<20000;i++);
}

void mpi_sense_finalize(){
  if(status_array != NULL){
    free(status_array);
  }
}

int main(int argc, char **argv)
{
  int rc;
  double *t1;
  double *t2;
  rc= MPI_Init(&argc, &argv);
  int rank, ptr;
  //double timer1,timer2;
  MPI_Status status_msg;
  double timer_pad=0;
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
  
  for(rc=0;rc<1000;++rc){
	for(ptr=0;ptr<1000;++ptr){
		mpi_sense_initialize();
		// printf("Hello for Round: %d from rank: %d\n",ptr,rank);
		//fflush(stdout);
		// timer1 = MPI_Wtime();
		t1[ptr] = MPI_Wtime();
		mpi_sense(P);
		// timer2 = MPI_Wtime();
		t2[ptr] = MPI_Wtime();
		// printf("\nFinish for Round: %d from: %d\n",ptr,rank);
		//fflush(stdout);
		timer_pad += t2[ptr] - t1[ptr];
		// mpi_sense(P);
		mpi_sense_finalize();  
	}
  }  
 
  //
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
		printf("Avg timer: %f",timer_pad);
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