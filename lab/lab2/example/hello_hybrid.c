#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include <mpi.h>
#include <omp.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int mpi_rank, mpi_ranks, omp_threads, omp_thread;
    char hostname[HOST_NAME_MAX];

    assert(!gethostname(hostname, HOST_NAME_MAX));
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_ranks);
#pragma omp parallel
    {
        omp_threads = omp_get_num_threads();
        omp_thread = omp_get_thread_num();
        printf("Hello %s: rank %2d/%2d, thread %2d/%2d\n", hostname, mpi_rank, mpi_ranks,
               omp_thread, omp_threads);
    }

    MPI_Finalize();
}
