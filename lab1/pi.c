// Description:
// 	Lab1 code for approximating pi by Riemann sum using MPI
// Author:
// 	Chan-Wei Hu
/***************************************************************/

#include <mpi.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[]){
	// Initialization
	int id, nproc;
	long long int N, startval, endval;
	long double pi, temp_sum;
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

	// start calculate partial pi
	long long int x;
	long double double_N = (long double) N;
	
	for(x=startval; x<endval; x++){
		pi += (4.0/double_N)*sqrt(1.0-pow(x/double_N, 2));
	}
	
	if(id!=0){
		MPI_Send(&pi, 4, MPI_DOUBLE_PRECISION, 0, 1, MPI_COMM_WORLD);
	} else{
		for(x=1; x<nproc; x++){
			MPI_Recv(&temp_sum, 4, MPI_DOUBLE_PRECISION, x, 1, MPI_COMM_WORLD, &status);
			pi += temp_sum;
		}
		printf("%Lf\n", pi);
	}
	
	MPI_Finalize();
    	return 0;

}

