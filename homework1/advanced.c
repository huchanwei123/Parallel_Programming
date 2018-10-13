// Description:                                             *
//  Implementation of advanced odd-even sort which can use  *
//  any method to sort, but elements can only be transfered *
//  between neighbor processes.                             *
// Method:                                                  *
//  In even phase, process id is even exchange values with  *
//  right neighbors. In odd phase, process id is odd        *
//  exchange values with right neighbor.                    *
// Author:                                                  *
//  Chan-Wei Hu                                             *
/************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

// function initialization
float *MPI_Read(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, int offset, int count);
void MPI_Write(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, float *local_data, int offset, int count);
int comparator(const void *a, const void *b);
void keep_high(int size_a, int size_b, float *data_a, float *data_b);
void keep_low(int size_a, int size_b, float *data_a, float *data_b);
void Send_data(int target_id, int local_size, float *local_data, MPI_Comm mpi_comm);
void Recv_data(int from_id, int recv_size, float *recv_data, MPI_Comm mpi_comm);
float diff(struct timespec start, struct timespec end);

int main(int argc, char *argv[]){
    // initialization for parameters
    const int N = atoi(argv[1]);
    
    // initialize for MPI parameters
    int id, size, rc;
    MPI_File file_in, file_out;
    MPI_Status status;
    MPI_Comm mpi_comm = MPI_COMM_WORLD;
    MPI_Group old_group;

    // Initial MPI environment
    rc = MPI_Init(&argc, &argv);
    if(rc != MPI_SUCCESS){
        printf("Error starting MPI program. Terminating\n");
        MPI_Abort(mpi_comm, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
	
	// Print out information
	if(id == 0){
		printf("Running basic.c!\nN = %d\ntotal processor used: %d\n", N, size); 
	} 

    // if the number of process is larger than N...
    if(N < size){
    	// obtain the group of processes in the world comm.
        MPI_Comm_group(mpi_comm, &old_group);

        // Remove unnecessary processes
        MPI_Group new_group;
        int ranges[][3] = {{N, size-1, 1}};
        MPI_Group_range_excl(old_group, 1, ranges, &new_group);

        // Create new comm.
        MPI_Comm_create(mpi_comm, new_group, &mpi_comm);

        if(mpi_comm == MPI_COMM_NULL){
            MPI_Finalize();
            exit(0);
        }
        size = N;
    }

    // scatter the data to each processor
    int num_per_processor = N/size;
    int local_size;
    int offset = sizeof(float)*id*num_per_processor;

    // load in data using MPI IO
    if(id == size-1){
        local_size = num_per_processor + (N%size);
    }else{
        local_size = num_per_processor;
    }

    // declaration of time measurement 
    struct timespec start, end;
    float IO_time, compute_time, comm_time;

    float *local_data = malloc(local_size * sizeof(float));
    clock_gettime(CLOCK_REALTIME, &start);
    local_data = MPI_Read(mpi_comm, argv[2], MPI_MODE_RDONLY, MPI_INFO_NULL, file_in, offset, local_size);
    MPI_Barrier(mpi_comm);
    clock_gettime(CLOCK_REALTIME, &end);
    IO_time += diff(start, end);

    // Start processor level odd-even sort
    // perform local sort first
    clock_gettime(CLOCK_REALTIME, &start);
    qsort(local_data, local_size, sizeof(float), comparator);
    MPI_Barrier(mpi_comm);
    clock_gettime(CLOCK_REALTIME, &end);
    compute_time += diff(start, end);

    // The last processor should communicate with left processor of theri size, others just num_per_processor
    int right_size = num_per_processor, left_size = num_per_processor;
    
    clock_gettime(CLOCK_REALTIME, &start);
    if(id == size-1){
	MPI_Send(&local_size, 1, MPI_INT, id-1, 2, mpi_comm);
        MPI_Recv(&left_size, 1, MPI_INT, id-1, 3, mpi_comm, MPI_STATUS_IGNORE);
    }
    if(id == size-2){
        MPI_Recv(&right_size, 1, MPI_INT, id+1, 2, mpi_comm, MPI_STATUS_IGNORE);
        MPI_Send(&local_size, 1, MPI_INT, id+1, 3, mpi_comm);
    }
    MPI_Barrier(mpi_comm);
    clock_gettime(CLOCK_REALTIME, &end);
    comm_time += diff(start, end);


    // start merge sort
    int phase = 0;
    for(phase=0; phase<=size; phase++){
        if((id+phase)%2 == 0 && id != size-1){
            float *my_temp = malloc(right_size * sizeof(float));

	    // measure comm time
	    clock_gettime(CLOCK_REALTIME, &start);
            Recv_data(id+1, right_size, my_temp, mpi_comm);
            Send_data(id+1, local_size, local_data, mpi_comm);
	    //MPI_Barrier(mpi_comm);
    	    clock_gettime(CLOCK_REALTIME, &end);
    	    comm_time += diff(start, end);

	    // measure compute time
	    clock_gettime(CLOCK_REALTIME, &start);
            keep_low(local_size, right_size, local_data, my_temp);
	    clock_gettime(CLOCK_REALTIME, &end);
    	    compute_time += diff(start, end);

            free(my_temp);
        }else if((id+phase)%2 == 1 && id != 0){
            float *my_temp = malloc(left_size * sizeof(float));

	    // measure comm time
	    clock_gettime(CLOCK_REALTIME, &start);
            Send_data(id-1, local_size, local_data, mpi_comm);
            Recv_data(id-1, left_size, my_temp, mpi_comm);
	    //MPI_Barrier(mpi_comm);
    	    clock_gettime(CLOCK_REALTIME, &end);
    	    comm_time += diff(start, end);

	    // measure compute time
	    clock_gettime(CLOCK_REALTIME, &start);
            keep_high(left_size, local_size, my_temp, local_data);
	    clock_gettime(CLOCK_REALTIME, &end);
    	    compute_time += diff(start, end);

            free(my_temp);
        }else;
        MPI_Barrier(mpi_comm);
    }

    // write the result
    clock_gettime(CLOCK_REALTIME, &start);
    MPI_Write(mpi_comm, argv[3], MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, file_out, local_data, offset, local_size);
    clock_gettime(CLOCK_REALTIME, &end);
    IO_time += diff(start, end);

    if(id==0)
	printf("I/O time: %f\nCompute time: %f\nComm. time: %f\n", IO_time, compute_time, comm_time);

    // free the data
    free(local_data);
   
    // Finalize MPI program
    MPI_Finalize();

    return 0;
}

void MPI_Write(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, float *local_data, int offset, int count){
    // Open file using MPI API
    int rc;
    rc = MPI_File_open(comm, filename, amode, info_, &fh);
    if(rc != MPI_SUCCESS){
        printf("[Error] output create FAILED");
        MPI_Abort(comm, rc);
    }
    // Read at certain offset
    MPI_File_write_at(fh, offset, local_data, count, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    return;
}

float *MPI_Read(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, int offset, int count){
    // Open file using MPI API
    int rc;
    float *local_data = malloc(count * sizeof(float));
    rc = MPI_File_open(comm, filename, amode, info_, &fh);
    if(rc != MPI_SUCCESS){
        printf("[Error] input read in FAILED");
        MPI_Abort(comm, rc);
    }
    // Read at certain offset
    MPI_File_read_at(fh, offset, local_data, count, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    return local_data;
}

void Send_data(int target_id, int local_size, float *local_data, MPI_Comm mpi_comm){
    // send data to target_id
    MPI_Send(local_data, local_size, MPI_FLOAT, target_id, 1, mpi_comm);
    return;
}

void Recv_data(int from_id, int recv_size, float *recv_data, MPI_Comm mpi_comm){
    // recveive data from from_id
    MPI_Recv(recv_data, recv_size, MPI_FLOAT, from_id, 1, mpi_comm, MPI_STATUS_IGNORE);
    return;
}

int comparator(const void *a, const void *b){
    // compare function for qsort()
    // Note that if we simply return the substraction of these two value, 
    // the return value may overflow!!! (From Stack overflow XD).
    // So the better way is to compare it first and return the value
    // If *(float *)a > *(float *)b, then it returns 1, else return -1.
    return (*(float *)a > *(float *)b) - (*(float *)a < *(float *)b);
}

void keep_high(int size_a, int size_b, float *data_a, float *data_b){
    float *tmp = malloc(sizeof(float)*size_b);
    // Merge the list with lower order to tmp
    int idx_a = size_a-1, idx_b = size_b-1, idx_out = size_b-1, i = 0;
  	
    while(idx_a >= 0 && idx_b >= 0 && idx_out >= 0){
        if(data_a[idx_a] < data_b[idx_b])
            tmp[idx_out--] = data_b[idx_b--];
        else
	        tmp[idx_out--] = data_a[idx_a--];
    	//printf("%f\n", tmp[idx_out+1]);
    }
    while(idx_out >= 0)
	tmp[idx_out--] = (idx_a >= 0)? data_a[idx_a--]:data_b[idx_b--];

    // split the data
    for(i = 0; i < size_b; i++){
        data_b[i] = tmp[i];
    }
    free(tmp);
}

void keep_low(int size_a, int size_b, float *data_a, float *data_b){
    float *tmp = malloc(sizeof(float)*size_a);
    // Merge the list with lower order to tmp
    int idx_a = 0, idx_b = 0, idx_out = 0, i = 0;
   
    while(idx_a < size_a && idx_b < size_b && idx_out < size_a){
        if(data_a[idx_a] < data_b[idx_b])
            tmp[idx_out++] = data_a[idx_a++];
        else
	    tmp[idx_out++] = data_b[idx_b++];
    }
	
    while(idx_out < size_a)
	tmp[idx_out++] = (idx_a < size_a)? data_a[idx_a++]:data_b[idx_b++];
   
    // split the data
    for(i = 0; i < size_a; i++)
        data_a[i] = tmp[i];

    free(tmp);
}

float diff(struct timespec start, struct timespec end){
    // function used to measure time in nano resolution
    float output;
    float nano = 1000000000.0;
    if(end.tv_nsec < start.tv_nsec)
        output = ((end.tv_sec - start.tv_sec -1)+(nano+end.tv_nsec-start.tv_nsec)/nano);
    else
	output = ((end.tv_sec - start.tv_sec)+(end.tv_nsec-start.tv_nsec)/nano);
    return output;
}
