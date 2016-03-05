#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void omp_sense(int num_of_threads, int& count, bool& local_sense, bool& global_sense){
	int thread_num = -1;
	//thread_num = omp_get_thread_num();
	*local_sense = ! (*local_sense);
	#pragma omp critical
	{
		// If we don't make this section critical and just execute like that (possible many threads waiting to reading the *count, while everyone gets a chance to decrement count)
		//Then everyone reads *count = 0 one by one, and keep getting contexed out before being able to do anything
		//
		#pragma omp atomic
		{
			if( (--(*count)) == 0){
				*count = num_of_threads;
				*global_sense = *local_sense;
				return;
			}
		}
		// if(*count == 0){
			// *count = num_of_threads;
			// *global_sense = *local_sense;
		// }
		while(*local_sense != *global_sense){}
	}	 
}

void main(int argc, char **argv){
	int i, x = 0;
	int num_of_threads, thread_num = -1;
	int count;
	bool local_sense, global_sense;
	global_sense = true;
	local_sense = true;
	if(argc > 1){
		for(int ptr = 1; ptr < argc; ++ptr){
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
	#pragma omp parallel num_threads(num_of_threads) firstprivate(thread_num, local_sense) shared(global_sense, count)
	{
		thread_num = omp_get_thread_num();
		printf("\nThread id is: %d",thread_num);
		omp_sense(num_of_threads);
		printf("\nAfter barriers. Id: %d",thread_num);
	}
	
}