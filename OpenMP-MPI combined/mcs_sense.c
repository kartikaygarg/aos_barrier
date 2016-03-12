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

void mpi_sense_initialize(int P, MPI_Status* status_array)
{
   status_array = (MPI_Status*) malloc( P * sizeof(MPI_Status));
}

void mpi_sense(int P, MPI_Status* status_array){
  int i, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int count;
  int global_sense = 1;
  int local_sense = 1;
  
  if (rank==0)
  {
  //MPI_Request reqs[P];
  count = P;
  count--;
  local_sense = (local_sense+1)%2;
  for(i = 1; i < P; i++)
   {
    MPI_Send(&count, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
    MPI_Recv(&count, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status_array[i-1]);
    }
    global_sense = local_sense;
    for(i=1; i<P; i++)
     // MPI_Isend(&global_sense, 1, MPI_INT , i, 1 , MPI_COMM_WORLD, &reqs[i-1]);
	 MPI_Send(&global_sense, 1, MPI_INT , i, 1 , MPI_COMM_WORLD);
   }
   
   else 
   {
    //MPI_Request reqs;  
    MPI_Status Status_msg;
    local_sense =(local_sense+1)%2;
    MPI_Recv(&count, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &Status_msg);
    count--;
    MPI_Send(&count, 1, MPI_INT, 0, 1, MPI_COMM_WORLD); 
    //MPI_Irecv(&global_sense, 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &reqs);
//	printf("\tglobal: %d from rank: %d\t",global_sense,rank);
//	printf("\tlocal: %d  from rank: %d|| ",local_sense,rank);
	MPI_Recv(&global_sense, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &Status_msg);
// v printf("\tglobal: %d from rank: %d",global_sense,rank);
    // printf("\tlocal: %d\t",local_sense);	
    // while(global_sense!=local_sense){
		// printf("\tSpin. %d",rank);
	// }
   }
 //   for (i=0;i<20000;i++);
}

void mpi_sense_finalize(MPI_Status* status_array){
  if(status_array != NULL){
    free(status_array);
  }
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

void comb_mcs_sense(int P,int ary,int arrv_ary,node* all_nodes,int thread_id, int num_proc, MPI_Status* status_array){
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
		// mpi_disseminate(num_proc);
		mpi_sense( num_proc, status_array);
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
	int rank,thread_num;
	int ary, arrv_ary=2;
	// int count;
	// rounds_t *rounds;
	// rounds = NULL;
	//double timer1,timer2;
	
	// int thread_id;
	struct timeval timer1,timer2;
	double timer_pad=0,timer_fin=0;
	
	node* all_nodes = NULL;
	
	// bool local_sense, global_sense;
	// global_sense = true;
	// local_sense = true;
	// char flag1=0;//,flag2=0;
	MPI_Status* status_array = NULL;
	status_array = (MPI_Status*) malloc( num_proc * sizeof(MPI_Status));
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
	// rounds = (rounds_t*) calloc((round_count+1) * num_proc, sizeof(rounds_t));
	
	//mpi_tournament_initialize(num_proc,rank);
	thread_num = 0;
	all_nodes = (node*) malloc(num_of_threads* sizeof(node));
	mpi_sense_initialize(num_proc, status_array);
	// status_array = (MPI_Status*) malloc( num_proc * sizeof(MPI_Status));
	for(ptr=0;ptr<num_of_threads;++ptr){
		omp_mcs_init(all_nodes,ptr, ary, arrv_ary, num_of_threads);
	}
	
	for(i=0;i<1000;++i){
		for(ptr=0;ptr<1000;++ptr){
			timer_pad = 0;
			// flag1 = 0;
			// count = num_of_threads;
			#pragma omp parallel num_threads(num_of_threads) firstprivate(thread_num, timer1, timer2) shared(num_proc, rank, all_nodes, ary, arrv_ary, status_array) reduction(+:timer_pad)
			{
				thread_num = omp_get_thread_num();		
				// printf("\nHello from thread: %d for Round: %d || Rank: %d ",thread_num,ptr,rank);
				// fflush(stdout);
				
				gettimeofday(&timer1, NULL);
				
				comb_mcs_sense(num_of_threads, ary, arrv_ary, all_nodes, thread_num, num_proc, status_array);
			
				gettimeofday(&timer2, NULL);
				// printf("\nFinish from thread: %d for Round: %d || Rank: %d",thread_num,ptr,rank);
				// fflush(stdout);
				timer_pad += ( (timer2.tv_sec - timer1.tv_sec) * 1000000) + (timer2.tv_usec - timer1.tv_usec);
			}	
			timer_fin += timer_pad;
		}
	}
	mpi_sense_finalize(status_array);
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