/*  Description:                                                    *
 *      The sequential Mandelbort Set code                          *
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

#define MAX_ITER 10000

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

    // decide current processor start from which row
    int row_per_proc = floor(h/size);

#ifdef DEBUG
    printf("Thread per proc: %d\nReal range: [%f %f]\nImagine range: [%f %f]\n", thd_per_proc, left, right, lower, upper);
    printf("w: %d\nh: %d\nout path: %s\n", w, h, out);
#endif

    // define complex number C
    CPLX C;

    // find start row and end row
    int start_row = id*row_per_proc;
    int end_row;
    if(id != size-1)
        end_row = (id+1)*row_per_proc;
    else
        end_row = h;

    // calculate pixel value
    int *img = (int *)malloc((end_row-start_row)*w*sizeof(int));
	assert(img);
	
	/* start mandelbrot sort */
    for(int j = start_row; j < end_row; ++j){
        C.imag = lower + j * ((upper - lower) / h);
        for(int i = 0; i < w; ++i){
            C.real = left + i * ((right - left) / w);
			img[(j-start_row)*w+i] = cal_pixel(C);
        }
    }
    
    int p;
    int *final_img = (int *)malloc(w*h*sizeof(int));
    if(id != 0){
        // send to master process
        MPI_Send(img, (end_row-start_row)*w, MPI_INT, 0, 1, mpi_comm);
    }else{
        int recieve_row = row_per_proc;
        memcpy(final_img, img, row_per_proc*w*sizeof(int));
        for(p=1;p<size;p++){
            if(p == size-1)
                recieve_row = h - p*row_per_proc;
            MPI_Recv(&(final_img[row_per_proc*p*w]), recieve_row*w, MPI_INT, p, 1, mpi_comm, &status);
        }
        write_png(out, w, h, final_img);
    }
    MPI_Barrier(mpi_comm);
	MPI_Finalize();
    free(final_img);
    free(img);
    return 0;
}

