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
float *Binary2Float(char *filename, int id, int local_size, int num_per_processor);
void swap(float *a, float *b);

int main(int argc, char *argv[]){
    // initialization for parameters
    string in_str(argv[1]);
    int N = stoi(in_str);
    //list = Binary2Float(argv[1]);
    
    // initialize for MPI parameters
    int id, size;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // scatter the data to each processor
    int num_per_processor = N/size;
    int local_size;
    int head_idx = id*num_per_processor;
    float *local_data = new float(num_per_processor);
    
    // load data
    if(id == size-1){
        local_size = num_per_processor + (N%size);
    }else{
        local_size = num_per_processor;
    }
        
    local_data = Binary2Float(argv[2], id, num_per_processor, local_size);

    // print out some information
    
    cout << "[" << id<< "] Before search: ";
    for(int i; i<local_size; i++){
        cout << *(local_data+i) << endl;
    }
    

    // Start strictly odd-even sort
    int local_done, total_done;
    while(!total_done){

        if(phase == 1){
            if(local_size % 2 == 0){
                // local sort first
                for(int i=1; i<local_size; i+=2){
                    if(local_data[i] > local_data[i+1])
                        swap(&local_data[i], &local_data[i+1]);
                }
                // send and compare head and tail
                
            }
    
        }   
        else{
    
        }
    }
    
    
    // test for compare tail and head between two processors
    /*
    float *tail;
    float after_swap_tail;
    if(id == 1){
        // get the tail data of current processor
        tail = &local_data[local_size-1];
        cout << "Address of tail is: " << tail << endl;
        MPI_Send(tail, sizeof(float), MPI_DOUBLE_PRECISION, id+1, 1, MPI_COMM_WORLD);
        MPI_Recv(&after_swap_tail, sizeof(float), MPI_DOUBLE_PRECISION, id+1, 1, MPI_COMM_WORLD, &status);
        if(after_swap_tail == local_data[local_size-1])
            cout << "No swap" << endl;
        else
            compare_and_swap(&local_data[local_size-1], &after_swap_tail);

    }else if(id == 2){
        // get head data of current processor
        float recv_tail;
        MPI_Recv(&recv_tail, sizeof(float), MPI_DOUBLE_PRECISION, id-1, 1, MPI_COMM_WORLD, &status);
        compare_and_swap(&recv_tail, local_data);
        MPI_Send(&recv_tail, sizeof(float), MPI_DOUBLE_PRECISION, id-1, 1, MPI_COMM_WORLD);
    }
    else;
    */
    // print out some information
    
    cout << "[" << id<< "] After search: ";
    for(int i; i<local_size; i++){
        cout << *(local_data+i) << endl;
    }
    
    MPI_Finalize();

    return 0;
}

float *Binary2Float(char *filename, int id, int local_size, int num_per_processor){
    ifstream inFile;
    ifstream::pos_type size;

    // open the binary file
    inFile.open(filename, ios::binary);

    // set the pointer position to the end of file
    inFile.seekg(0, ios::end);
    size = inFile.tellg();
    
    // set the pointer position to the offset
    int offset;
    offset = sizeof(float)*id*num_per_processor;
    inFile.seekg(offset, ios::beg);
    
    // allocate memory for the array
    float *out = new float(local_size);
    for(int i=0; i<local_size; i++){
        inFile.read(reinterpret_cast<char*>(&out[i]), sizeof(float));
    }
    
    // check if read in successfully
    if (inFile);
        //cout << "[Info] ID:" << id << " Successfully read in..." << endl;
    else
        throw string("[Error] input read in FAILED");

    return out;
}

bool TAIL2HEAD(int cur_id, ){
    
}

void swap(float *a, float *b){
    float tmp;
    tmp = *b;
    *b = *a;
    *a = tmp;
    return;
}
