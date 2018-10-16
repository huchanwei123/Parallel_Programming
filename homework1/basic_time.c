// Description:                                          *
//  Implementation of basic odd-even sort which can only *
//  be swapped with its adjacent elements                *
// Author:                                               *
//  Chan-Wei Hu                                          *
/*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define ODD_PHASE   1
#define EVEN_PHASE  0
#define is_odd(in)  (in%2)
#define is_even(in) (!is_odd(in))
#define convert(in) ((in+1)%2)

// function initialization
float *MPI_Read(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, int offset, int count);
void MPI_Write(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, float *local_data, int offset, int count);
void swap(float *a, float *b);
int local_sort(int start_idx, int local_size, float *local_data, int local_done);
int tail_send2head(int cur_id, int local_size, float *local_data, int local_done, MPI_Comm mpi_comm);
int head_recv_from_tail(int cur_id, float *local_data, int local_done, MPI_Comm mpi_comm);
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

    // performance measurement declaration
    struct timespec start, end;
    float IO_time=0, compute_time=0, comm_time=0;


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
    clock_gettime(CLOCK_REALTIME, &start);
    local_data = MPI_Read(mpi_comm, argv[2], MPI_MODE_RDONLY, MPI_INFO_NULL, file_in, offset, local_size);
    MPI_Barrier(mpi_comm);
    clock_gettime(CLOCK_REALTIME, &end);
    IO_time += diff(start, end);

    // Start strictly odd-even sort
    int done = 0;
    int even_done = 0;
    int odd_done = 0;
    int phase = 0;
    
    // if only one processor...
    if(size == 1){
    	while(!even_done || !odd_done){
	    odd_done = 1;
	    even_done = 1;
	    clock_gettime(CLOCK_REALTIME, &start);
            if(is_odd(phase))
            	odd_done = local_sort(phase, local_size, local_data, odd_done);
	    else
            	even_done = local_sort(phase, local_size, local_data, even_done);
	    clock_gettime(CLOCK_REALTIME, &end);
	    compute_time += diff(start, end);
	    phase = ((phase == ODD_PHASE)? EVEN_PHASE : ODD_PHASE);
	}
    }else{
	while(!even_done || !odd_done){
	done = 1;
	// for local sort
	clock_gettime(CLOCK_REALTIME, &start);
	if(is_odd(num_per_processor) && is_odd(id))
		done = local_sort(convert(phase), local_size, local_data, done);
	else
		done = local_sort(phase, local_size, local_data, done);
	MPI_Barrier(mpi_comm);
	clock_gettime(CLOCK_REALTIME, &end);
	compute_time += diff(start, end);
	
	clock_gettime(CLOCK_REALTIME, &start);
	if(phase == ODD_PHASE){
		// in odd phase now
		// for cross processor communication
		if(num_per_processor%2==0){
			// send and compare head and tail
			if(id != size-1)
				done = tail_send2head(id, local_size, local_data, done, mpi_comm);
			if(id != 0)
				done = head_recv_from_tail(id, local_data, done, mpi_comm);
			}else{
			// send and compare cross processor
			if(id%2==1 && id != size-1){
				done = tail_send2head(id, local_size, local_data, done, mpi_comm);
			}
			if(id%2==0 && id != 0){
				done = head_recv_from_tail(id, local_data, done, mpi_comm);
			}
		}
	}else if(phase == EVEN_PHASE){
		// for cross processor communication
		if(num_per_processor%2==1){
			// send and compare cross processor
			if(id%2==0 && id != size-1){
				done = tail_send2head(id, local_size, local_data, done, mpi_comm);
			}
			if(id%2==1){
				done = head_recv_from_tail(id, local_data, done, mpi_comm);
			}
		}
	}
	else;

	// change phase
	phase = ((phase == ODD_PHASE)? EVEN_PHASE : ODD_PHASE);
	
	// wait all processes done
	MPI_Barrier(mpi_comm);
	clock_gettime(CLOCK_REALTIME, &end);
	comm_time += diff(start, end);
	
	if(phase == ODD_PHASE)
		MPI_Allreduce(&done, &even_done, 1, MPI_INT, MPI_LAND, mpi_comm);
	else
		MPI_Allreduce(&done, &odd_done, 1, MPI_INT, MPI_LAND, mpi_comm);
	}
    }

    // write the result
    clock_gettime(CLOCK_REALTIME, &start);
    MPI_Write(mpi_comm, argv[3], MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, file_out, local_data, offset, local_size);
    MPI_Barrier(mpi_comm);
    clock_gettime(CLOCK_REALTIME, &end);
    IO_time += diff(start, end);

    if(id==0)
	printf("I/O time: %f\nCompute time: %f\nComm. time: %f\n", IO_time, compute_time, comm_time);
    
    // free the data
    free(local_data);

    MPI_Barrier(mpi_comm);    
    MPI_Finalize();

    return 0;
}

void MPI_Write(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, float *local_data, int offset, int count){
    // Open file using MPI API
    int rc;
    rc = MPI_File_open(comm, filename, amode, info_, &fh);
    if(rc != MPI_SUCCESS){
        printf("[Error] output create FAILED\n");
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
        printf("[Error] input read in FAILED\n");
        MPI_Abort(comm, rc);
    }
    // Read at certain offset
    MPI_File_read_at(fh, offset, local_data, count, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    return local_data;
}

int tail_send2head(int cur_id, int local_size, float *local_data,int local_done, MPI_Comm mpi_comm){
    float *tail;
    float after_swap_tail;
    // get the tail data of current processor
    tail = &local_data[local_size-1];
    MPI_Send(tail, 1, MPI_REAL, cur_id+1, cur_id, mpi_comm);
    MPI_Recv(&after_swap_tail, 1, MPI_REAL, cur_id+1, cur_id, mpi_comm, MPI_STATUS_IGNORE);
    if(after_swap_tail != local_data[local_size-1]){
        swap(&local_data[local_size-1], &after_swap_tail);
        local_done = 0;
    }

    return local_done;
}

int head_recv_from_tail(int cur_id, float *local_data, int local_done, MPI_Comm mpi_comm){
    // get head data of current processor
    float recv_tail;
    MPI_Recv(&recv_tail, 1, MPI_REAL, cur_id-1, cur_id-1, mpi_comm, MPI_STATUS_IGNORE);
    if(recv_tail > *local_data){
        swap(&recv_tail, local_data);
        local_done = 0;
    }
    MPI_Send(&recv_tail, 1, MPI_REAL, cur_id-1, cur_id-1, mpi_comm);
    
    return local_done;
}

int local_sort(int start_idx, int local_size, float *local_data, int local_done){
    for(int i=start_idx; i<local_size-1; i+=2){
        if(local_data[i] > local_data[i+1]){
            swap(&local_data[i], &local_data[i+1]);
            local_done = 0;
        }
    }
    return local_done;
}

void swap(float *a, float *b){
    float tmp;
    tmp = *b;
    *b = *a;
    *a = tmp;
    return;
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
