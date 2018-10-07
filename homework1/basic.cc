// Description:                                          *
//  Implementation of basic odd-even sort which can only *
//  be swapped with its adjacent elements                *
// Author:                                               *
//  Chan-Wei Hu                                          *
/*********************************************************/
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "mpi.h"

using namespace std;

// function initialization
float *MPI_Read(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, int offset, int count);
void MPI_Write(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, float *local_data, int offset, int count);
void swap(float *a, float *b);
bool local_sort(int start_idx, int local_size, float *local_data);
bool tail_send2head(int cur_id, int local_size, float *local_data, bool local_done, MPI_Comm mpi_comm);
bool head_recv_from_tail(int cur_id, float *local_data, bool local_done, MPI_Comm mpi_comm);

int main(int argc, char *argv[]){
    // initialization for parameters
    string in_str(argv[1]);
    int N = stoi(in_str);
    
    // initialize for MPI parameters
    int id, size, rc;
    MPI_File file_in, file_out;
    MPI_Status status;
    MPI_Comm mpi_comm = MPI_COMM_WORLD;

    // Initial MPI environment
    rc = MPI_Init(&argc, &argv);
    if(rc != MPI_SUCCESS){
        throw string("Error starting MPI program. Terminating\n");
        MPI_Abort(mpi_comm, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // scatter the data to each processor
    int num_per_processor = N/size;
    int local_size;
    int offset = sizeof(float)*id*num_per_processor;
    int head_idx = id*num_per_processor;
    float *local_data = new float(num_per_processor);

    // load in data using MPI IO
    if(id == size-1){
        local_size = num_per_processor + (N%size);
    }else{
        local_size = num_per_processor;
    }
    local_data = MPI_Read(mpi_comm, argv[2], MPI_MODE_RDONLY, MPI_INFO_NULL, file_in, offset, local_size);
    
    // Start strictly odd-even sort
    int done = 0;
    int all_done = 0;
    int phase = 0;
    while(!all_done){
        done = 1;
        if(phase == 1){
            // in odd phase now
            if(local_size % 2 == 0){
                // local sort first
                done = local_sort(1, local_size, local_data);
                // send and compare head and tail
                if(id != size-1)
                    done = tail_send2head(id, local_size, local_data, done, mpi_comm);
                if(id != 0)
                    done = head_recv_from_tail(id, local_data, done, mpi_comm);
            }else{    
                // send and compare cross processor
                if(id % 2 == 1 && id != size-1){
                    done = local_sort(0, local_size, local_data);
                    done = tail_send2head(id, local_size, local_data, done, mpi_comm);
                }
                if(id % 2 == 0 && id != 0){
                    done = local_sort(1, local_size, local_data);
                    done = head_recv_from_tail(id, local_data, done, mpi_comm);
                }
            }
        }   
        else if(phase == 0){
            // in even phase now
            if(local_size % 2 == 0){
                // do local sort only
                done = local_sort(0, local_size, local_data);
            }else{
                // send and compare cross processor
                if(id % 2 == 0 && id != size-1){
                    done = local_sort(1, local_size, local_data);
                    done = tail_send2head(id, local_size, local_data, done, mpi_comm);
                }
                if(id % 2 == 1){
                    done = local_sort(0, local_size, local_data);
                    done = head_recv_from_tail(id, local_data, done, mpi_comm);
                }
            }
        }
        else;
        
        // change phase
        phase = ((phase == 1)? 0 : 1);
    
        // wait all processes done
        MPI_Barrier(mpi_comm);
        MPI_Allreduce(&done, &all_done, 1, MPI_INT, MPI_LAND, mpi_comm);
    }
    // print out some information
    /*
    cout << "[" << id<< "] After search: ";
    for(int i; i<local_size; i++){
        cout << *(local_data+i) << endl;
    }
    */

    // write the result
    MPI_Write(mpi_comm, argv[3], MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, file_out, local_data, offset, local_size);

    // free the data
    delete [] local_data;

    MPI_Barrier(mpi_comm);    
    MPI_Finalize();

    return 0;
}

void MPI_Write(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, float *local_data, int offset, int count){
    // Open file using MPI API
    int rc;
    rc = MPI_File_open(comm, filename, amode, info_, &fh);
    if(rc != MPI_SUCCESS)
        throw string("[Error] output create FAILED");
    
    // Read at certain offset
    MPI_File_write_at(fh, offset, local_data, count, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    return;
}

float *MPI_Read(MPI_Comm comm, char *filename, int amode, MPI_Info info_, MPI_File fh, int offset, int count){
    // Open file using MPI API
    int rc;
    float *local_data = new float(count);
    rc = MPI_File_open(comm, filename, amode, info_, &fh);
    if(rc != MPI_SUCCESS)
        throw string("[Error] input read in FAILED");
    
    // Read at certain offset
    MPI_File_read_at(fh, offset, local_data, count, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    return local_data;
}

bool tail_send2head(int cur_id, int local_size, float *local_data, bool local_done, MPI_Comm mpi_comm){
    float *tail;
    float after_swap_tail;
    // get the tail data of current processor
    tail = &local_data[local_size-1];
    MPI_Send(tail, 1, MPI_REAL, cur_id+1, 1, mpi_comm);
    MPI_Recv(&after_swap_tail, 1, MPI_REAL, cur_id+1, 1, mpi_comm, MPI_STATUS_IGNORE);
    if(after_swap_tail != local_data[local_size-1]){
        swap(&local_data[local_size-1], &after_swap_tail);
        local_done = false;
    }else
        local_done = true;

    return local_done;
}

bool head_recv_from_tail(int cur_id, float *local_data, bool local_done, MPI_Comm mpi_comm){
    // get head data of current processor
    float recv_tail;
    MPI_Recv(&recv_tail, 1, MPI_REAL, cur_id-1, 1, mpi_comm, MPI_STATUS_IGNORE);
    if(recv_tail > *local_data){
        swap(&recv_tail, local_data);
        local_done = false;
    }else
        local_done = true;
    MPI_Send(&recv_tail, 1, MPI_REAL, cur_id-1, 1, mpi_comm);
    
    return local_done;
}

bool local_sort(int start_idx, int local_size, float *local_data){
    bool local_done;
    for(int i=start_idx; i<local_size-1; i+=2){
        if(local_data[i] > local_data[i+1]){
            swap(&local_data[i], &local_data[i+1]);
            local_done = false;
        }else
            local_done = true;
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
