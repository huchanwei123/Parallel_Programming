#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

int isPrime(int i) {
    int sqrt_i = (int)sqrt((double)i);
    int j;
    for (j = 2; j <= sqrt_i; ++ j) {
        if (i % j == 0) return 0;
    }
    return 1;
}
int main(int argc, char** argv) {
    int N = atoi(argv[1]);
    int count = 0;
    int i = 0;
	// OpenMP coming 
#pragma omp parallel
	{
	int omp_threads = omp_get_num_threads();
	int omp_thread = omp_get_thread_num();
	int start = omp_thread*floor(N/omp_threads)+2;
	int end;
	int tmp = 0;	

	if(omp_thread == omp_threads-1)
		end = N;
	else
		end = (omp_thread+1)*floor(N/omp_threads)+2;
	
    for (i = start; i < end; i ++) {
        tmp += isPrime(i);
    }
	count += tmp;
	}

    printf("There are %d prime numbers <= %d\n", count, N);
	return 0;
}
