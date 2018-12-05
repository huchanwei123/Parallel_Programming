#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <pthread.h>
#include <omp.h>
#include <math.h>

#define inf 65536
#define RESULT_TAG 100
#define TOKEN -1 
#define TERMINATE -2
#define WHITE 1
#define BLACK 0
#define NUM_THREADS 1

typedef struct{
    int proc;
    int thd_id;
    int vertex_id;    
}vertex;

// global variable 
static int rank, size, E, V;
static int *GRAPH;
int *dist;
vertex *v_arr;
MPI_Comm comm;

void *Moore(void *id){
	int *id_int = (int*)id;
	int tid = *id_int;
    int data_send[2], data_recv[2];
	int recv_vid, new_dist, my_token = WHITE, token_get;
    int my_rank = rank*NUM_THREADS + tid;
    MPI_Status status;
   
	// start from vertex 0, send data
	if(rank == v_arr[0].proc && tid == v_arr[0].thd_id){
        for(int v = 0; v < V; v++){
            if(GRAPH[v] != inf){
                // exists a path
                data_send[0] = v;           // target vertex
                data_send[1] = GRAPH[v];    // target distance
                // Send to relative process
                // [NOTE]! use thread id as tag
				int target_proc = v_arr[v].proc;
				int target_thd = v_arr[v].thd_id;
                MPI_Bsend(data_send, 2, MPI_INT, target_proc, target_thd, comm);
			}
        }
        // also pass the token for ring termination
	    data_send[0] = WHITE;
        data_send[1] = TOKEN;  // distance = -1 means data_send[0] is a token info
        MPI_Bsend(data_send, 2, MPI_INT, (my_rank+1)/NUM_THREADS % size, (my_rank+1) % NUM_THREADS, comm);
    }
    // me keep recieving data 
    while(MPI_Recv(data_recv, 2, MPI_INT, MPI_ANY_SOURCE, tid, comm, &status) == MPI_SUCCESS){
		if(data_recv[1] >= 0){
            // means we got the distance data
            recv_vid = data_recv[0];
            new_dist = data_recv[1];
            if(new_dist < dist[recv_vid]){
                // update minimum distance 
                dist[recv_vid] = new_dist;
                // send to destination vertex
                for(int v = 0; v < V; v++){
                    if(GRAPH[recv_vid*V+v] != inf){
                        if(v_arr[v].proc*NUM_THREADS + v_arr[v].thd_id < my_rank) my_token = BLACK;
                        // send to target vertex
                        data_send[0] = v;
                        data_send[1] = new_dist + GRAPH[recv_vid*V+v];
                        MPI_Bsend(data_send, 2, MPI_INT, v_arr[v].proc, v_arr[v].thd_id, comm);
                    }
                }
            }
        }else if(data_recv[1] == TOKEN){
            token_get = data_recv[0];
            data_send[1] = TOKEN;
            if(my_rank == 0){
                if(token_get == WHITE){
                    // send terminate!!!
                    data_send[1] = TERMINATE;
                    data_send[0] = TERMINATE;
                    MPI_Bsend(data_send, 2, MPI_INT, (my_rank+1)/NUM_THREADS % size, (my_rank+1) % NUM_THREADS, comm);
                    break;
                }else{
                    data_send[0] = WHITE;
                    MPI_Bsend(data_send, 2, MPI_INT, (my_rank+1)/NUM_THREADS % size, (my_rank+1) % NUM_THREADS, comm);
                }
            }else{
                // for other, we pass the token
                if(my_token == BLACK){
                    data_send[0] = BLACK;
                    my_token = WHITE;
                }
                else data_send[0] = WHITE;          
                MPI_Bsend(data_send, 2, MPI_INT, (my_rank+1)/NUM_THREADS % size, (my_rank+1) % NUM_THREADS, comm);
            }
        }else if(data_recv[1] == TERMINATE){
            // terminate flag!!!
            data_send[0] = TERMINATE;
            data_send[1] = TERMINATE;
            MPI_Bsend(data_send, 2, MPI_INT, (my_rank+1)/NUM_THREADS % size, (my_rank+1) % NUM_THREADS, comm);
            break;
        }else
            continue;
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
	// MPI start
    int r;
	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &r);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Status status;
    comm = MPI_COMM_WORLD;
	
    /****************** load graph data ******************/
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
    // read in data from binary file
    int src, dst, w;
    while(fread(&src, sizeof(int), 1, in_fp) == 1){
        fread(&dst, sizeof(int), 1, in_fp);
        fread(&w, sizeof(int), 1, in_fp);
        if(GRAPH[src*V+dst] > w)
            GRAPH[src*V+dst] = w;
    }
    fclose(in_fp);
    /*****************************************************/

    /* NOTE: will change after implementing graph partition */
    // get the vertices number for current process
    if(size > 1){
		int mod = V%size;
		int i, j;
		int slice= (V-mod)/size;
		if(rank != size-1)
			mod = 0;

		int vertice_num = slice + mod;
		
		// divide vertices into processes and threads
		v_arr = (vertex*)malloc(V*sizeof(vertex));
		dist = (int*)malloc(V*sizeof(int));
		for(i = 0; i < V; i++){
			if(floor(i/slice) >= size){
				v_arr[i].proc = size-1;
				v_arr[i].thd_id = i % NUM_THREADS;
				v_arr[i].vertex_id = i;
			}else{
				v_arr[i].proc = floor(i/slice);
				v_arr[i].thd_id = i % NUM_THREADS;
				v_arr[i].vertex_id = i;
			}
			if(i == 0)
				dist[i] = 0;
			else
				dist[i] = inf;
		}
		
		int msg_size = pow(2, 20);
		int buf[msg_size];
		MPI_Buffer_attach(buf, msg_size);

		/************** declare Pthread ***************/
		pthread_t thd[NUM_THREADS];
		int *ID = (int*)malloc(NUM_THREADS*sizeof(int));
		for(int thd_id = 0; thd_id < NUM_THREADS; thd_id++){
			ID[thd_id] = thd_id;
			int err = pthread_create(&thd[thd_id], NULL, Moore, (void*)&ID[thd_id]);
			if(err){
				printf("ERROR; return code from pthread_create() is %d\n", err);
				exit(-1);
			}
		}
		
		for(int thd_id = 0; thd_id < NUM_THREADS; thd_id++){
			pthread_join(thd[thd_id], NULL);
		}
		MPI_Buffer_detach(&buf, &msg_size);

		/***********************************************/
		// gather the result
		int *tmp_dist = (int*)malloc(V*sizeof(int));
		int *f_dist = (int*)malloc(V*sizeof(int));
		
		if(rank == 0){
			for(i=0;i<V;i++){
				if(rank == v_arr[i].proc) f_dist[i] = dist[i];
			}
			for(int id = 1; id < size; id++){
				MPI_Recv(tmp_dist, V, MPI_INT, id, RESULT_TAG, comm, &status);
				// put the data to final result
				for(i=0;i<V;i++){
					if(id == v_arr[i].proc) f_dist[i] = tmp_dist[i];
				}
			}
			// write to output
			FILE *out_fp;
			out_fp = fopen(argv[2], "wb");
			for(int i = 0; i < V; i++){
				fwrite(f_dist+i, sizeof(int), 1, out_fp);
			}
			fclose(out_fp);
		}
		else{
			MPI_Send(dist, V, MPI_INT, 0, RESULT_TAG, comm);
		}
		free(tmp_dist);
		free(ID);
		free(dist);
		free(v_arr);	
    }else{
		int *dist_seq = (int*)malloc(V*sizeof(int));
    	for(int i = 0; i<V;i++){
        	if(i == 0)
            	dist_seq[i] = 0;
        	else
            	dist_seq[i] = inf;
    	}
		for(int k = 1; k < V; k++){
            for(int l = 0; l < V; l++){
                for(int m = 0; m < V; m++){
                    if(dist_seq[l]!=inf && dist_seq[l] + GRAPH[l*V+m] < dist_seq[m])
                        dist_seq[m] = dist_seq[l] + GRAPH[l*V+m];
                }
            }
        }
		// write to output
        FILE *out_fp;
        out_fp = fopen(argv[2], "wb");
        for(int i = 0; i < V; i++){
            fwrite(dist_seq+i, sizeof(int), 1, out_fp);
        }
        fclose(out_fp);
		free(dist_seq);
	}
    
    free(GRAPH);
    MPI_Finalize();
    return 0;
}
