#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>

#define inf 65536


int main(int argc, char *argv[]){
	// MPI start
	int rank, size;
	int E, V;
	int vertice_per_proc;
	int *local_mat = NULL;	
	int *GRAPH = NULL;
    int proc_num = strtol(argv[3], 0, 10);
        
    /********************* load graph ********************/
	// read in file first
    FILE *in_fp;
    in_fp = fopen(argv[1], "r");
    
    if(in_fp == NULL)
        printf("Failed on opening file\n");
    
    // read in edges and vertices
    fread(&V, sizeof(int), 1, in_fp);
    fread(&E, sizeof(int), 1, in_fp);
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
    /****************************************************/    
    int *vertices_map = (int*)malloc(V*sizeof(int));

    // start partition the graph 
    // simply detect the graph edge and partition the data
    int mod = V % proc_num;
    int edge_per_proc = (E - mod)/proc_num;
	int slice = (V - mod)/proc_num;    
	int count_e = 0;
    int proc_id = 0;
    for(int i = 0; i < V; i++){
		if(floor(i/slice) >= proc_num-1) vertices_map[i] = proc_num-1;
		else vertices_map[i] = floor(i/slice);
			
		/*
        for(int j = 0; j < V; j++){
            // first, we try to find the edge number that almost match edge_per_proc
            if(GRAPH[i*V+j] != inf)
                count_e++;
        }
        if(count_e <= edge_per_proc){
            // means the edge of current vertex
            vertices_map[i] = proc_id;
        }else{
            proc_id++;
            if(proc_id >= proc_num) proc_id = proc_num-1;
            vertices_map[i] = proc_id;
            count_e = 0;
        }
		*/
		
    }   

    // write the result to output file
    FILE *out_fp;
	out_fp = fopen(argv[2], "w");
	for(int i =0;i<V;i++){
		//printf("%d ", vertices_map[i]);
		fprintf(out_fp, "%d\n", vertices_map[i]);
	}
	fclose(out_fp);
	free(vertices_map);
    return 0;
}

