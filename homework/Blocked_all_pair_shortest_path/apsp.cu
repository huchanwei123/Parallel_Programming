#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <assert.h>
#include <math.h>

#define inf 1e9

__global__ void update(int *adj_mat_d, int Xstart, int Ystart, int round, int B, int comp_V){
    int k;
    int i = B * Xstart + blockIdx.x * blockDim.x + threadIdx.x;
    int j = B * Ystart + blockIdx.y * blockDim.y + threadIdx.y;
    //if(blockIdx.x >= 11) printf("block id = %d\n", blockIdx.x);
    for(int offset = 0; offset < B; offset++){
        k = round*B+offset;
        if(adj_mat_d[i*comp_V+j] > adj_mat_d[i*comp_V+k] + adj_mat_d[k*comp_V+j]){
            adj_mat_d[i*comp_V+j] = adj_mat_d[i*comp_V+k] + adj_mat_d[k*comp_V+j];
        }
    }
}

int main(int argc, char *argv[]){
    /******************************* load data *********************************/
    // only two arguments are allowed
    assert(argc == 3);

    // open input file
    int E, comp_V, V;
    FILE *in_fp;
    in_fp = fopen(argv[1], "r");
    if(in_fp == NULL) printf("Failed on opening file\n");

    // read in data
    fread(&V, sizeof(int), 1, in_fp);
    fread(&E, sizeof(int), 1, in_fp);
    printf("Total vertices: %d\nTotal edges: %d\n", V, E);

    // start dividing data
    // block size
    int B = 16;
    printf("Block size B = %d\n", B);

    // check if V % B == 0
    int V_block = V % B ? V/B+1 : V/B;
    comp_V = B*V_block+1;
    // create adjacency matrix for new graph
    int *adj_mat = (int*)malloc(comp_V*comp_V*sizeof(int));
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
        if(adj_mat[src*comp_V+dst] > w) adj_mat[src*comp_V+dst] = w;
    }
    fclose(in_fp);
    /****************************************************************************/
    
    /****************************** Device info. ********************************/
    // only single GPU is used, so the device index is 0
    // print out the information of GPU
    int device_idx = 0;
    /*
    cudaDeviceProp deviceProp;
    cudaError_t cudaError;
    cudaError = cudaGetDeviceProperties(&deviceProp, device_idx);
    printf("Device max threads per block : %d\n", deviceProp.maxThreadsPerBlock);
    printf("Max thread dim: (%d, %d, %d)\n", deviceProp.maxThreadsDim[0], deviceProp.maxThreadsDim[1], deviceProp.maxThreadsDim[2]);
    */
    cudaSetDevice(device_idx);
    /****************************************************************************/
    // block size
    int round = ceil((float)V/B);
    int block_dim = 8;
    int grid_size = B/block_dim;
    //int gridFactor = 1;
    //int gridFactor = (1024/B)*(1024/B);
    //int gridFactor = B*B;
    printf("Will run %d rounds\n", round);

    dim3 blocks(grid_size, grid_size);
    dim3 threads(block_dim, block_dim);
    dim3 Col_Other(grid_size*round, grid_size);
    dim3 Row_Other(grid_size, grid_size*round);

    // malloc for cuda memory 
    int *adj_mat_d;
    cudaMalloc((void**) &adj_mat_d, comp_V*comp_V*sizeof(int));
    cudaMemcpy(adj_mat_d, adj_mat, comp_V*comp_V*sizeof(int), cudaMemcpyHostToDevice);

    // start iteration
    for(int r = 0; r < round; r++){
        /********************** phase 1 ***********************/
        update <<<blocks, threads>>> (adj_mat_d, r, r, r, B, comp_V);

        /********************** phase 2 ***********************/
        // column part
        update <<<Col_Other, threads>>> (adj_mat_d, 0, r, r, B, comp_V);
        // row part 
        update <<<Row_Other, threads>>> (adj_mat_d, r, 0, r, B, comp_V);
        
        /********************** phase 3 ************************/
        for(int i = 0; i < round; i++){
            update <<<Row_Other, threads>>> (adj_mat_d, i, 0, r, B, comp_V);
        }
    }
    cudaDeviceSynchronize();
    // copy result back to host
    cudaMemcpy(adj_mat, adj_mat_d, comp_V*comp_V*sizeof(int), cudaMemcpyDeviceToHost);
    // free memory
    cudaFree(adj_mat_d);

    // output the result
    FILE *out_fp;
    out_fp = fopen(argv[2], "wb");
    for(int i = 0; i < V; i++){
        for(int j = 0; j < V; j++){
            fwrite(adj_mat+i*comp_V+j, sizeof(int), 1, out_fp);
        }   
    }   
    fclose(out_fp);
    cudaFreeHost(adj_mat);

    return 0;
}

