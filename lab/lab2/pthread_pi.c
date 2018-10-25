/*********************************************************************************
*	Description:																 *
*		Lab2 code for pi value estimation using pthread and mutex				 *
*	Author:																		 *
*		Chan-Wei Hu																 *
*********************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// initialize global variable
long double sum = 0;

// initialize mutex
pthread_mutex_t mutex;

// define a structure
struct arg{
	int thd_id;	
	long double N;
	long long int start;
	long long int end;
};

void *partial_sum(void *arg){
	struct arg *args = arg;
    long double partial_pi = 0;
	long long int x = 0;
	for(x=args->start; x<=args->end; x++){
		partial_pi += (4.0/args->N)*sqrt(1.0-pow(x/args->N, 2));
    }	
	// Actually, we don't need to use mutex here since the data is not sequential.
	pthread_mutex_lock(&mutex);
	sum += partial_pi;
	pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
	// thread number
    int num_threads = atoi(argv[1]);
	// point for pi estimation
	int N = atoi(argv[2]);
	struct arg args;
	args.N = (long double)N;

	// define element per thread
	int num_per_thd = floor(N/num_threads);

    pthread_t threads[num_threads];
	pthread_mutex_init(&mutex, NULL);

    int rc;
    int t;
    for (t = 0; t < num_threads; t++){
        args.thd_id = t;
		if(t == num_threads-1){
			args.start = t*(long long int)num_per_thd+1;
			args.end = N;
		}else{
			args.start = t*(long long int)num_per_thd;
			args.end = (t+1)*(long long int)num_per_thd;
		}
        rc = pthread_create(&threads[t], NULL, partial_sum, (void*)&args);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
		pthread_join(threads[t] , NULL);
    }

	printf("Approximate pi = %Lf\n", sum);
    pthread_exit(NULL);
	return 0;
}
