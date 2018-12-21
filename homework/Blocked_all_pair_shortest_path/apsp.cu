/****************************************************************************************
    All-paired shortest path implementation in CUDA (single GPU version)
    Optimization:
        1. Unroll
        2. shared memory in phase 3
    Author:
        Chan-Wei Hu
******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <assert.h>
#include <math.h>
#include <omp.h>

#define inf 1e9
static int block_dim = 32;

// phase 1 kernel (done!!)
__global__ void Phase_1(int *adj_mat_d, int round, int block_dim, int comp_V) {
    
	int i = threadIdx.y, 
        j = threadIdx.x,
        offset = block_dim * round;
    
    extern __shared__ int shared_mem[];

	shared_mem[i * block_dim + j] = adj_mat_d[(i + offset) * comp_V + (j + offset)];
	__syncthreads();

    #pragma unroll
	for(int k = 0; k < block_dim; k++){
        if (shared_mem[i * block_dim + j] > shared_mem[i * block_dim + k] + shared_mem[k * block_dim + j]){
            shared_mem[i * block_dim + j] = shared_mem[i * block_dim + k] + shared_mem[k * block_dim + j];
        }
        __syncthreads();
	}
	adj_mat_d[(i + offset) * comp_V + (j + offset)] = shared_mem[i * block_dim + j];
    __syncthreads();
}

// phase 2 kernel (done !!!)
__global__ void Phase_2(int* adj_mat_d, int round, int block_dim, int comp_V) {
	
    int total_round = comp_V/block_dim;

    int i = threadIdx.y,
        j = threadIdx.x,
        // column or row?
        i_off = blockIdx.x == 1? block_dim * ((blockIdx.y + round + 1) % total_round): block_dim * round,
        j_off = blockIdx.x == 1? block_dim * round : block_dim * ((blockIdx.y + round + 1) % total_round);
	
    extern __shared__ int shared_mem[];
	
    shared_mem[i * block_dim + j] = adj_mat_d[(i + i_off) * comp_V + (j+j_off)];
	shared_mem[(i + block_dim) * block_dim + j] = adj_mat_d[(i + i_off) * comp_V + j + round*block_dim];
	shared_mem[(i + 2*block_dim) * block_dim + j] = adj_mat_d[(i + round * block_dim) * comp_V + (j + j_off)];
	__syncthreads();

    #pragma unroll
	for (int k = 0; k < block_dim; k++) {
		if (shared_mem[i * block_dim + j] > shared_mem[(i + block_dim) * block_dim + k] + shared_mem[(k + 2*block_dim) * block_dim + j]) {
            shared_mem[i * block_dim + j] = shared_mem[(i + block_dim) * block_dim + k] + shared_mem[(k + 2*block_dim) * block_dim + j]; 
            
            if (round == i_off/block_dim) 
                shared_mem[(i + 2*block_dim) * block_dim + j] = shared_mem[i * block_dim + j];
            if (round == j_off/block_dim) 
                shared_mem[(i + block_dim) * block_dim + j] = shared_mem[i * block_dim + j];
		}	
	}
	adj_mat_d[(i + i_off) * comp_V + (j+j_off)] = shared_mem[i * block_dim + j];
	__syncthreads();
}

// Phase 3 kernel (done !!!)
__global__ void Phase_3(int* adj_mat_d, int round, int block_dim, int comp_V){

    int i = threadIdx.y,
        j = threadIdx.x,
        i_off = block_dim * blockIdx.x,
        j_off = block_dim * blockIdx.y;

     
	extern __shared__ int shared_mem[];

	shared_mem[i * block_dim + j] = adj_mat_d[(i + i_off) * comp_V + (j+j_off)];
	shared_mem[(i + block_dim) * block_dim + j] = adj_mat_d[(i + i_off) * comp_V + j + round*block_dim];
	shared_mem[(i + 2*block_dim) * block_dim + j] = adj_mat_d[(i + round * block_dim) * comp_V + (j + j_off)];
    __syncthreads();
	
    #pragma unroll
	for (int k = 0; k < block_dim; k++) {
		if (shared_mem[i * block_dim + j] > shared_mem[(i + block_dim) * block_dim + k] + shared_mem[(k + 2*block_dim) * block_dim + j])
            shared_mem[i * block_dim + j] = shared_mem[(i + block_dim) * block_dim + k] + shared_mem[(k + 2*block_dim) * block_dim + j];
	}
	
	adj_mat_d[(i + i_off) * comp_V + (j+j_off)] = shared_mem[i * block_dim + j];
	__syncthreads();
}

int main(int argc, char *argv[]){
	/******************************* load data *********************************/
    // only two arguments are allowed
    assert(argc == 3);

    int E, V;
    FILE *in_fp;
    in_fp = fopen(argv[1], "r");
    if(in_fp == NULL) printf("Failed on opening file\n");
    // read in data
    fread(&V, sizeof(int), 1, in_fp);
    fread(&E, sizeof(int), 1, in_fp);

    // compensate V to make V % block_dim == 0
	int comp_V = V + (block_dim - ((V-1) % block_dim + 1));

	//allocate memory
    int *adj_mat; 
    size_t sz = comp_V * comp_V * sizeof(int);
	cudaMallocHost((void**) &adj_mat, sz);
	for(int i = 0; i < comp_V; i++){
        for(int j = 0; j < comp_V; j++){
            if(i == j) adj_mat[i*comp_V+j] = 0;
            else adj_mat[i*comp_V+j] = inf;
        }
    }
    // load data to graph
    int src, dst, w;
    while(E--){
        fread(&src, sizeof(int), 1, in_fp);
        fread(&dst, sizeof(int), 1, in_fp);
        fread(&w, sizeof(int), 1, in_fp);
        adj_mat[src*comp_V+dst] = w;
    }
    fclose(in_fp);
    /****************************************************************************/

	int round =  ceil((float) comp_V/block_dim);
    int *adj_mat_d;

    // 2D block
    dim3 threads(block_dim, block_dim);
	
	dim3 p1(1, 1);
	dim3 p2(2, round-1);
    dim3 p3(round, round);
    
    //size_t sz = comp_V * comp_V * sizeof(int);

    cudaSetDevice(0);
    // Malloc memory
    cudaMalloc((void**) &adj_mat_d, sz);
    cudaMemcpy(adj_mat_d, adj_mat, sz, cudaMemcpyHostToDevice);

    for(int r = 0; r < round; r++){    
		Phase_1 <<<p1, threads, sizeof(int)*block_dim*block_dim >>>(adj_mat_d, r, block_dim, comp_V);
            
   //     cudaDeviceSynchronize();
			
        Phase_2 <<<p2, threads, sizeof(int)*3*block_dim*block_dim >>>(adj_mat_d, r, block_dim, comp_V);
			
   //     cudaDeviceSynchronize();
			
        Phase_3 <<<p3, threads, sizeof(int)*3*block_dim*block_dim >>>(adj_mat_d, r, block_dim, comp_V);
	}
	
	// copy back to host
    cudaMemcpy(adj_mat, adj_mat_d, sz, cudaMemcpyDeviceToHost);

	// output
    FILE *out_fp;
    out_fp = fopen(argv[2], "wb");
    for(int i = 0; i < V; i++){
        for(int j = 0; j < V; j++){
            fwrite(adj_mat+i*comp_V+j, sizeof(int), 1, out_fp);
        }   
    }   
    fclose(out_fp);

	//free memory
	cudaFree(adj_mat_d);
    cudaFreeHost(adj_mat);
	return 0;
}
