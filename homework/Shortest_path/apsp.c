#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>
#include <time.h>

#define inf 65536
#define TERMINATE_TAG 1

int main(int argc, char *argv[]){
	// MPI start
	int rank, size;
	int E, V;
	int *local_mat = NULL;	
	int *GRAPH = NULL;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Status status;	

    /*********************** load graph **************************/
	// read in file first
    FILE *in_fp;
    in_fp = fopen(argv[1], "r");
    
    if(in_fp == NULL)
        printf("Failed on opening file\n");
    
    // read in edges and vertices
    fread(&V, sizeof(int), 1, in_fp);
    fread(&E, sizeof(int), 1, in_fp);
    // broadcast it
    // create graph 
    GRAPH = (int*)malloc(V*V*sizeof(int));
    for(int i = 0; i<V; i++){
        for(int j = 0; j<V; j++){
            if(i == j)
                GRAPH[i*V+j] = 0;
            else
                GRAPH[i*V+j] = inf;
        }
    }
    
    // read the graph data
    int src, dst, w;
    while(fread(&src, sizeof(int), 1, in_fp) == 1){
        fread(&dst, sizeof(int), 1, in_fp);
        fread(&w, sizeof(int), 1, in_fp);
        if(GRAPH[src*V+dst] > w)
            GRAPH[src*V+dst] = w;
    }
    fclose(in_fp);
    /*************************************************************/
    
	int vertice_num; 
    int *local_v;
    int *vertice_map = (int*)malloc(V*sizeof(int));
    int mod = V%size;
    int slice= (V-mod)/size;

    if(argc == 4){
        // read in partition file
        FILE *par_fp;
        par_fp = fopen(argv[3], "r");

        if(par_fp == NULL)
            printf("Failed on opening file\n");

        // count how many vertices for current rank
        int count = 0;
        int p;
        int idx = 0;
		
        while(fscanf(par_fp, "%d\n", &p) != EOF){
            if(p == rank) count++;
            vertice_map[idx] = p;
            idx++;
        }
		

        vertice_num = count;
        local_v = (int*)malloc(vertice_num*sizeof(int));
        fseek(par_fp, 0, SEEK_SET);
        // start read in vertex id and store to local_v
        int vid = 0;
        idx = 0;
        while(fscanf(par_fp, "%d\n", &p) != EOF){
            if(p == rank){
                local_v[idx] = vid;
                idx++;
            }
            vid++;
        }
    }else{
        if(rank != size-1)
            mod = 0;
        
        int end;    
        if(rank == size-1)
            end = V;
        else
            end = slice*(rank+1);
        
        vertice_num = slice+mod;
        local_v = (int*)malloc(vertice_num*sizeof(int));
        for(int idx = 0; idx < vertice_num; idx++){
            local_v[idx] = slice*rank + idx;
        }

        // for vertice_map
        for(int a=0;a<V;a++){
            if(floor(a/slice) >= size-1)
                vertice_map[a] = size-1;
            else
                vertice_map[a] = floor(a/slice);
        }
    }
    int i, j;
    int *tmp = (int*)malloc(V*sizeof(int));
    int root;

    for(int k = 0; k < V; ++k){
        root = vertice_map[k];
        if(root == rank)
            memcpy(tmp, GRAPH+k*V, V*sizeof(int));
        MPI_Bcast(tmp, V, MPI_INT, root, comm);
#pragma parallel schedule(dynamic) for private(i,j) 
        for(i = 0; i < vertice_num; i++){
            for(j = 0; j < V; ++j){
                GRAPH[local_v[i]*V+j] = GRAPH[local_v[i]*V+k] + tmp[j] < GRAPH[local_v[i]*V+j]? GRAPH[local_v[i]*V+k]+tmp[j]:GRAPH[local_v[i]*V+j];
            }
        }
    }
    
	if(rank == 0){
    // recieve result from others
        int terminate = 0;
        int *temp_graph = (int*)malloc(V*V*sizeof(int));
        while(terminate < size-1){
            MPI_Recv(temp_graph, V*V, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if(status.MPI_TAG == TERMINATE_TAG){
                terminate++;
            }
            for(int i=0;i<V;i++){
                if(vertice_map[i] == status.MPI_SOURCE)
                    memcpy(GRAPH+i*V, temp_graph+i*V, V*sizeof(int));
            }
        }
        free(temp_graph);
        
        // write the result to output file
        FILE *out_fp;
        out_fp = fopen(argv[2], "wb");
        for(int i = 0; i < V; i++){
            for(int j = 0; j < V; j++){
                fwrite(GRAPH+i*V+j, sizeof(int), 1, out_fp);
            }   
        }   
        fclose(out_fp);

    }else{
        MPI_Send(GRAPH, V*V, MPI_INT, 0, TERMINATE_TAG, comm);
    }
    
    free(local_v);
    free(vertice_map);
    free(tmp);
    free(GRAPH);
	MPI_Finalize();
    return 0;
}

