#include <stdio.h>

#include <omp.h>

int main(int argc, char** argv) {
    int omp_threads, omp_thread;

#pragma omp parallel
    {
        omp_threads = omp_get_num_threads();
        omp_thread = omp_get_thread_num();
        printf("Hello: thread %2d/%2d\n", omp_thread, omp_threads);
    }
    return 0;
}
