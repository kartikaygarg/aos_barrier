#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

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

// void omp_mcs_init(node* all_nodes, int thread_id, bool* local_sense, int ary, int arrv_ary, int num_of_threads){
void omp_mcs_init(node* all_nodes, int thread_id, int ary, int arrv_ary, int num_of_threads){
		int temp=0,i=0;
		int LEVEL1_DCACHE_LINESIZE = 64;
		
		// *local_sense = false;
		all_nodes[thread_id].local_sense = false;
		
		// all_nodes[thread_id].have_child = 
		
		posix_memalign((void *)(&(all_nodes[thread_id].have_child)), LEVEL1_DCACHE_LINESIZE, LEVEL1_DCACHE_LINESIZE);	
		posix_memalign((void *)(&(all_nodes[thread_id].child_NotReady)), LEVEL1_DCACHE_LINESIZE, LEVEL1_DCACHE_LINESIZE);	
		//[thread_id].child_ptr = NULL;
		
//		posix_memalign((void *)(&(all_nodes[thread_id].child_ptr)), LEVEL1_DCACHE_LINESIZE, arrv_ary*sizeof(bool));	
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
				// all_nodes[thread_id].child_NotReady[i] = NULL;
				++temp;
				++i;
			}
		}
		while(i < LEVEL1_DCACHE_LINESIZE/sizeof(bool)){			//Initializing the zombie place holders of ptr
			// all_nodes[thread_id].have_child[i] = NULL;
			all_nodes[thread_id].have_child[i] = false;
			//all_nodes[thread_id].child_NotReady[i] = NULL;
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
		/*
		while( (temp <= num_of_threads-1) && (temp <= ((thread_id*2)+arrv_ary) ) ){
			//all_nodes[thread_id].(child_ptr+i) = &(all_nodes[temp].parent_sense);
			
			
			 all_nodes[thread_id].child_ptr[i] = &(all_nodes[temp].parent_sense);
		//	*((all_nodes[thread_id].child_ptr)+(i*sizeof(bool))) = &(all_nodes[temp].parent_sense);
			
			++temp;
		}
		*/
		/*
		while(temp <= ((thread_id*2)+arrv_ary)){
			*(all_nodes[thread_id].child_ptr[i]) = false;				//no child to point to
			//all_nodes[thread_id].child_ptr[i] = NULL;				//no child to point to
			++temp;
			++i;
		}
		*/
		for(i=0;i<arrv_ary;++i){
			all_nodes[thread_id].child_ptr[i] = false;
		}
}

// void omp_mcs(int P,int ary,int arrv_ary,node* all_nodes,bool* local_sense,int thread_id){
void omp_mcs(int P,int ary,int arrv_ary,node* all_nodes,int thread_id){
	int i=0;
	int j=0;
	int flag=0;
	// *local_sense = !(*local_sense);
	all_nodes[thread_id].local_sense = !(all_nodes[thread_id].local_sense);
	
	while( (all_nodes[thread_id].have_child[i] == true) && (i<ary) ){
		while(all_nodes[thread_id].child_NotReady[i] == false){
			// printf("\nWait1 from thread: %d || on child: %d",thread_id, i);
			// if((thread_id==0)){
				// printf("\n FOr thread: %d", thread_id);
				// for(j=0;j<ary;++j){
					// printf("\nChild: %d || Bool: %b , Int: %d",j,all_nodes[thread_id].child_NotReady[j],all_nodes[thread_id].child_NotReady[j] );
				// }
			// }
		}
		++i;
	}
	i=0;
	for(i=0;((all_nodes[thread_id].have_child[i] == true) && (i<ary) );++i){
		all_nodes[thread_id].child_NotReady[i] = false;
	}
	if(thread_id == 0){
		*(all_nodes[thread_id].parent_sense) = !(*(all_nodes[thread_id].parent_sense));
	}
	else{
		*(all_nodes[thread_id].parent_ptr) = true;
		// while(*local_sense != all_nodes[thread_id].parent_sense){}
		while(all_nodes[thread_id].local_sense != *(all_nodes[thread_id].parent_sense)){
			if(((thread_id == 0) || (thread_id == 1) ) && (flag<2) ){
				++flag;
				 // printf("\nThread: %d 's parent sense: %d at add: 0x%p || Local sense: %d", thread_id, *(all_nodes[thread_id].parent_sense), all_nodes[thread_id].parent_sense, all_nodes[thread_id].local_sense);
			}
			 
			
		}
	}
	i=0;
	//for(i=0;all_nodes[thread_id].child_ptr[i] != NULL;++i){
	// for(i=0;( (all_nodes[thread_id].child_ptr[i] != false) && (i<arrv_ary) );++i){	
	//for(i=0;( (all_nodes[thread_id].child_ptr[i] != *local_sense) && (i<arrv_ary) );++i){	
	
	
	for(i=0;( ( all_nodes[thread_id].child_ptr[i] != all_nodes[thread_id].local_sense) && (i<arrv_ary) );++i){	
		all_nodes[thread_id].child_ptr[i] = !(all_nodes[thread_id].child_ptr[i]);
	 }
	
	/*
	// for(i=0;( (i<arrv_ary) );++i){	
	for(i = (thread_id*2)+1;i <= ((thread_id*2)+arrv_ary);++i) {

		// if(){
			// printf("\nThread: %d ,waking its child: %d || at add: 0x%x",thread_id,i, *((all_nodes[thread_id].child_ptr)+(sizeof(bool)*i)));
			
			printf("\nThread: %d ,waking its child: %d || at add: 0x%p",thread_id,i, all_nodes[thread_id].child_ptr[i]);
			
			*(all_nodes[thread_id].child_ptr[i]) = !(*(all_nodes[thread_id].child_ptr[i]));
			
			//all_nodes[i].parent_sense = !all_nodes[i].parent_sense;
			
		// }
	}
	*/
}

void omp_mcs_fin(node* all_nodes, int thread_id)
{
		free(all_nodes[thread_id].have_child);
		free(all_nodes[thread_id].child_NotReady);
		free(all_nodes[thread_id].child_ptr);
}

int main(int argc, char** argv){
	int num_of_threads;
	// volatile int count;
	int ptr,i;
	int cnt;
	int ary = 4, arrv_ary = 2;
	int thread_id;
	struct timeval timer1,timer2;
	double timer_pad=0,timer_fin=0;
	// bool* have_child;
	// bool* parent_ptr;
	// double timer1,timer2;
	node* all_nodes = NULL;
	// double time_pad = 0;
	// volatile bool local_sense, parent_sense;
	// bool local_sense = false;
	// parent_sense = false;
	if(argc > 1){
		for(ptr = 1; ptr < argc; ++ptr){
			if (argv[ptr][0] == '-'){
				if (!strcmp(argv[ptr], "-n") || !strcmp(argv[ptr], "-num_threads")) {
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
		num_of_threads = 6;
		ary = 4;
		arrv_ary = 2;
	}
	all_nodes = (node*) malloc(num_of_threads* sizeof(node));
	// for(ptr = 0;ptr<num_of_threads;++ptr){
		// for(cnt=0;cnt<ary;++cnt){
			// all_nodes[ptr]->child_NotReady[cnt] = false;
			// all_nodes[ptr]->has_child[cnt] = false;
			// all_nodes[ptr]->child_ptr[0] = false;
		// }
		
	// }
	// int LEVEL1_DCACHE_LINESIZE = 64;
	timer_fin = 0;
	printf("\nThreads: %d || Ary: %d || Arrv_Ary: %d",num_of_threads,ary, arrv_ary);
	
	for(ptr=0;ptr<num_of_threads;++ptr){
		omp_mcs_init(all_nodes,ptr, ary, arrv_ary, num_of_threads);
	}
	for(i=0;i<1000;++i){
		for(cnt=0;cnt<1000;++cnt){
			timer_pad = 0;
			#pragma omp parallel num_threads(num_of_threads) shared(ary, arrv_ary, all_nodes, num_of_threads) private(thread_id, timer1, timer2) reduction(+:timer_pad)
			{
				thread_id = omp_get_thread_num();
		
				//omp_mcs_init(all_nodes,thread_id,&local_sense, ary, arrv_ary, num_of_threads);
				// printf("\nHello from Thread: %d",thread_id);
				gettimeofday(&timer1, NULL);
				// omp_mcs(num_of_threads, ary, arrv_ary, all_nodes, &local_sense,thread_id);
				omp_mcs(num_of_threads, ary, arrv_ary, all_nodes, thread_id);
				gettimeofday(&timer2, NULL);
				timer_pad += ( (timer2.tv_sec - timer1.tv_sec) * 1000000) + (timer2.tv_usec - timer1.tv_usec);
				// printf("\nFinish for Thread: %d",thread_id);
				//omp_mcs_fin(all_nodes,thread_id);		
			}
			timer_fin += timer_pad;
			
		}
	}
	for(ptr=0;ptr<num_of_threads;++ptr){
		omp_mcs_fin(all_nodes,ptr);	
	}
	free(all_nodes);
	printf("\n\n-------------------------\n\n");
	printf("\nTotal time: %f microsec\n----------------------\n",timer_fin);
	timer_fin = timer_fin/1000000;
	printf("\nAverage time: %f microsec",timer_fin);
	
	return 0;
}