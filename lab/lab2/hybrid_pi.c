// Description:
// 	Lab1 code for approximating pi by Riemann sum using MPI
// Author:
// 	Chan-Wei Hu
/***************************************************************/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
typedef long long int Lint;

int main(int argc, char *argv[]){
	// Initialization
	int id, nproc;
	Lint N, startval, endval;
	double pi, temp_sum;
	MPI_Status status;

    // Read in N and convert it to integer
	N = atoi(argv[1]);
		
	// initial MPI
	MPI_Init(&argc, &argv);

	// get number from total nodes
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);

	// get my id 
	MPI_Comm_rank(MPI_COMM_WORLD, &id);

	// set the partition
	pi = 0;
	startval = N*id/nproc;
	endval = N*(id+1)/nproc;
	double double_N = (double) N;
	// start calculate partial pi
	// OpenMP coming!
	#pragma omp parallel
    {
        Lint omp_threads = omp_get_num_threads();
        Lint omp_thread = omp_get_thread_num();
		Lint intv = endval - startval;
		Lint num_per_thd = floor(intv/omp_threads);
		Lint start = startval + omp_thread*num_per_thd;
		Lint end = 0;
		Lint idx = 0;
		double pi_tmp;
		if(omp_thread == omp_threads-1)	
			end = endval;
		else
			end = startval + (omp_thread+1)*num_per_thd;
	
		for(idx=start; idx<end; idx++){
			pi_tmp += (4.0/double_N)*sqrt(1.0-pow(idx/double_N, 2));
		}
		pi += pi_tmp;
	}
	int x;	
	if(id!=0){
		MPI_Send(&pi, 4, MPI_DOUBLE_PRECISION, 0, 1, MPI_COMM_WORLD);
	} else{
		for(x=1; x<nproc; x++){
			MPI_Recv(&temp_sum, 4, MPI_DOUBLE_PRECISION, x, 1, MPI_COMM_WORLD, &status);
			pi += temp_sum;
		}
		printf("%f\n", pi);
	}
	
	MPI_Finalize();
    	return 0;

}

