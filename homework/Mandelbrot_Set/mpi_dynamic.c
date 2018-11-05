/*  Description:                                                    *
 *      The Mandelbort Set code using MPI with dynamic scheduling   *
 *  Author:                                                         *
 *      Chan-Wei Hu                                                 *
 *******************************************************************/
#define PNG_NO_SETJMP

#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <mpi.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

// define some parameters
#define MAX_ITER 10000
#define DATA_TAG 0
#define RESULT_TAG 1

// define data structure for complex number
struct complex_num{
    double real, imag;
};

typedef struct complex_num CPLX;

void write_png(const char* filename, const int width, const int height, const int* buffer) {
    FILE* fp = fopen(filename, "wb");
    assert(fp);
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(png_ptr);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr);
    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                             PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);
    size_t row_size = 3 * width * sizeof(png_byte);
    png_bytep row = (png_bytep)malloc(row_size);
    for (int y = 0; y < height; ++y) {
        memset(row, 0, row_size);
        for (int x = 0; x < width; ++x) {
            int p = buffer[(height - 1 - y) * width + x];
            png_bytep color = row + x * 3;
            if (p != MAX_ITER) {
                if (p & 16) {
                    color[0] = 240;
                    color[1] = color[2] = p % 16 * 16;
                }else{
                    color[0] = p % 16 * 16;
                }
            }
        }
        png_write_row(png_ptr, row);
    }
    free(row);
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

int cal_pixel(CPLX C){
    // calculate pixel value for plotting
    int count = 0;
    double len_sq = 0;
    // start calculate pixel
    double real = 0;
    double imag = 0;
    while(count < MAX_ITER && len_sq < 4){
        double temp = real*real - imag*imag + C.real;
        imag = 2*real*imag + C.imag;
        real = temp;
        len_sq = real*real + imag*imag;
        ++count;
    }
    return count;
}

int main(int argc, char *argv[]){
    assert(argc==9);
    // Read in argument 
    int thd_per_proc = strtol(argv[1], 0, 10);
    double left = strtod(argv[2], 0);
    double right = strtod(argv[3], 0);
    double lower = strtod(argv[4], 0);
    double upper = strtod(argv[5], 0);
    int w = strtol(argv[6], 0, 10), h = strtol(argv[7], 0, 10);
    const char *out = argv[8];

    // initialize the MPI 
    int rc, id, size;
    MPI_Status status;
    MPI_Comm mpi_comm = MPI_COMM_WORLD;
    rc = MPI_Init(&argc, &argv);
    if(rc != MPI_SUCCESS){
        printf("Error starting MPI program. Terminating\n");
        MPI_Abort(mpi_comm, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    // define complex number C
    CPLX C;

    // for master process
    int count = 0;
    int row = 0;
    int get_row = 0;
    // allocate memeory for output image and initial to 0
    int *img = (int *)malloc(h*w*sizeof(int));
    memset(img, 0, h*w*sizeof(int));
    assert(img);
	
	// define bucket size
	int bucket_size = 1;

	int *local_result = (int*)malloc(bucket_size*w*sizeof(int));
	int thre = 10;
	long long int cnt = 0;
	if(id == 0){
        // lookup list for send head row to which processor
        int *lookup = (int*)malloc((size-1)*sizeof(int));
        memset(lookup, 0, (size-1)*sizeof(int));
        // send initial task to slave process
        for(int pid=1; pid<size; pid++){
            MPI_Send(&row, 1, MPI_INT, pid, DATA_TAG, mpi_comm);
            lookup[pid-1] = row;
            count++;
            row+=bucket_size;
        }
        
        // loop for keep recieving result and send data to slave processor
        int *tmp_row = (int *)malloc(bucket_size*w*sizeof(int));
		while(count > 0){
			// scan if any slave processor had send something...
            	MPI_Recv(tmp_row, bucket_size*w, MPI_INT, MPI_ANY_SOURCE, RESULT_TAG, mpi_comm, &status);
            	count--;
            	get_row = lookup[status.MPI_SOURCE-1];
            	memcpy(img+(get_row*w), tmp_row, bucket_size*w*sizeof(int));
            	// send another task to slave processor
            	if(row<h){
                	lookup[status.MPI_SOURCE-1] = row;
                	MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, DATA_TAG, mpi_comm);
#ifdef DEBUG
                	printf("Master processor send row %d to [%d]\n", row, status.MPI_SOURCE);
#endif
                	count++;
                	row+=bucket_size;
            	}else{
                	row = h+1;
                	MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, DATA_TAG, mpi_comm);
            	}
        }
        free(tmp_row);
        free(lookup);
        write_png(out, w, h, img);
    }else{
        // for slave process        
        while(1){
            MPI_Recv(&row, 1, MPI_INT, 0, DATA_TAG, mpi_comm, &status);
            if(row >= h+1)
                break;

            // start doing mandelbrot set for target row
			for(int j = row; j < row+bucket_size; ++j){
				C.imag = lower + j * ((upper-lower)/h);
            	// First, sample some pixel and do it
				for(int i = 0; i < w; i+=3){
					C.real = left + i * ((right - left) / w);
					local_result[(j-row)*w+i] = cal_pixel(C);
					C.real = left + (i+1) * ((right - left) / w);
					local_result[(j-row)*w+(i+1)] = cal_pixel(C);
				}
		
				// scan for which pixel should be calculate
				for(int i = 2; i < w; i+=3){
					for(int idx=thre; idx>0; --idx){
						cnt += local_result[(j-row)*w+(i-idx)];
					}			

					if(i < w-1 && i >= thre && local_result[(j-row)*w+(i+1)] == MAX_ITER && cnt == thre*MAX_ITER){
						local_result[(j-row)*w+i] = MAX_ITER;
					}else{
						C.real = left + i * ((right - left) / w);
						local_result[(j-row)*w+i] = cal_pixel(C);
					}
					cnt = 0;
				}
	
			}
            // send back the result to master
            MPI_Send(local_result, bucket_size*w, MPI_INT, 0, RESULT_TAG, mpi_comm);
        }
    }
	MPI_Finalize();
    free(img);
    free(local_result);
    return 0;
}

