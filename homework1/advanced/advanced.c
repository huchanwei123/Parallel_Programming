
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
#include "mpi.h"

#define ODD_PHASE   1
#define EVEN_PHASE  0
#define is_odd(in)  (in%2)
#define is_even(in) (!is_odd(in))
#define convert(in) ((in+1)%2)

// function initialization
float *MPI_Read(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, int offset, int count);
void MPI_Write(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, float *local_data, int offset, int count);
int comparator(const void *a, const void *b);
void swap(float *a, float *b);
void head_send_data(int cur_id, int local_size, float *local_data, MPI_Comm mpi_comm);
void recv_and_sort(int cur_id, int recv_size, float *recv_data, MPI_Comm mpi_comm);

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
    float *local_data = malloc(local_size * sizeof(float));
    local_data = MPI_Read(mpi_comm, argv[2], MPI_MODE_RDONLY, MPI_INFO_NULL, file_in, offset, local_size);
    
    // Start processor level odd-even sort
    int done = 0;
    int even_done = 0;
    int odd_done = 0;
    int phase = 0;
    
    // define pointer of pointer to local data :)
    float *p2local_data;
    float *get_data = malloc(local_size * sizeof(float));
    p2local_data = &(local_data[0]);
   
    if(id == 2){
        head_send_data(id, local_size, local_data, mpi_comm);
    }else if(id == 1){
        printf("[1] origin addr. of get_data: %p\n", get_data);
        recv_and_sort(id, local_size, get_data, mpi_comm);
        printf("[1] Addr. of get_data: %p\n", get_data);
        printf("[1] Get data from 2 with: %f\n", *(get_data+1));
    }else;
    /*
    while(!even_done || !odd_done){
        done = 1;
        qsort(local_data, local_size, sizeof(float), comparator);

        if(is_odd(id) && id != size-1){
            // in odd phase now
            done = tail_send2head(id, local_size, local_data, done, mpi_comm);
            done = head_recv_from_tail(id, local_data, done, mpi_comm);
        }else if(is_even(id) && id != size-1){
            // in even phase now
            done = tail_send2head(id, local_size, local_data, done, mpi_comm);
            done = head_recv_from_tail(id, local_data, done, mpi_comm);
        }
        else;
        // change phase
        phase = ((phase == ODD_PHASE)? EVEN_PHASE : ODD_PHASE);

        // wait all processes done
        MPI_Barrier(mpi_comm);
        if(phase == ODD_PHASE)
            MPI_Allreduce(&done, &even_done, 1, MPI_INT, MPI_LAND, mpi_comm);
        else
            MPI_Allreduce(&done, &odd_done, 1, MPI_INT, MPI_LAND, mpi_comm);
    }

    // write the result
    MPI_Write(mpi_comm, argv[3], MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, file_out, local_data, offset, local_size);

    // free the data
    free(local_data);
    */
    MPI_Barrier(mpi_comm);   
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

void head_send_data(int cur_id, int local_size, float *local_data, MPI_Comm mpi_comm){
    // send the address to cur_id-1
    MPI_Send(local_data, local_size, MPI_FLOAT, cur_id-1, 1, mpi_comm);
}

void recv_and_sort(int cur_id, int recv_size, float *recv_data, MPI_Comm mpi_comm){
    // recveive the address from cur_id+1
    MPI_Recv(recv_data, recv_size, MPI_FLOAT, cur_id+1, 1, mpi_comm, MPI_STATUS_IGNORE);
    return;
}

int comparator(const void *a, const void *b){
    return (*(float *)a - *(float *)b);
}

void swap(float *a, float *b){
    float tmp;
    tmp = *b;
    *b = *a;
    *a = tmp;
    return;
}
