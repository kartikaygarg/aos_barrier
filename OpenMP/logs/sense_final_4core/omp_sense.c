#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <string.h>

void omp_sense(int num_of_threads, volatile int* count, volatile bool* local_sense, volatile bool* global_sense){
	*local_sense = !(*local_sense);
	if(__sync_fetch_and_sub(count,1) == 1){
		*count = num_of_threads;
		*global_sense = *local_sense;
	}
	while(*local_sense != *global_sense){}	
}

int main(int argc, char **argv){
	// int i, x = 0;
	struct timeval timer1,timer2;
	// struct timeval elapsed;
	double timer_pad;
	int num_of_threads, thread_num = -1;
	volatile int count;
	int ptr,i;
	volatile bool local_sense, global_sense;
	global_sense = true;
	local_sense = true;
	if(argc > 1){
		for(ptr = 1; ptr < argc; ++ptr){
			if (argv[ptr][0] == '-'){
				if (!strcmp(argv[ptr], "-n") || !strcmp(argv[ptr], "-num_threads")) {
					num_of_threads = atoi(argv[++ptr]);
				}
			}
		}
	}
	else{		//Num of threads not passed | Default no of threads = 8
		num_of_threads = 6;
	}
	
	count = num_of_threads;
	//elapsed.tv_sec = 0;
	//elapsed.tv_usec = 0;
	timer_pad = 0;
	for(i=0;i<1000;++i){
		for(ptr=0;ptr<1000;++ptr){
			#pragma omp parallel num_threads(num_of_threads) firstprivate(thread_num, local_sense, timer1, timer2) shared(global_sense, count) reduction(+:timer_pad)
			{
				thread_num = omp_get_thread_num();		
				// printf("\nHello from: %d",thread_num);
				gettimeofday(&timer1, NULL);
				omp_sense(num_of_threads, &count, &local_sense, &global_sense);
				gettimeofday(&timer2, NULL);
				// printf("\nAfter barrier from: %d",thread_num);
				//omp_sense(num_of_threads, &count, &local_sense, &global_sense);
				timer_pad += ( (timer2.tv_sec - timer1.tv_sec) * 1000000) + (timer2.tv_usec - timer1.tv_usec);
				// elapsed.tv_sec += timer2.tv_sec - timer1.tv_sec;
				// elapsed.tv_usec += timer2.tv_usec - timer1.tv_usec;
				// printf("\nTime: %f",timer_pad/1000000);
			}	
		}
	}
	printf("\n\n-------------------------\n\n");
	
	//printf("Total timer: Sec: %f || Usec: %f\n",elapsed.tv_sec,elapsed.tv_usec);
    //timer_pad = (elapsed.tv_sec * 1000000) + elapsed.tv_usec;
	printf("\nTotal time: %f microsec\n----------------------\n",timer_pad);
	timer_pad = timer_pad/1000000;
	/*
	for(ptr=0;ptr<1000;++ptr){
		for(rc=0;rc<=ptr;++rc){
			if(timer2[ptr]<=timer1[rc]){
				printf("ERROR in order!!!!");
			}
		}  
	}
	*/
	printf("\nAverage time: %f microsec",timer_pad);
	
	return 0;
}
