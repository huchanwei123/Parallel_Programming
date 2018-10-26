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
    double ori_real = C.real;
    double ori_imag = C.imag;
    while(count < MAX_ITER && len_sq < 4.0){
        double temp = C.real*C.real - C.imag*C.imag + ori_real;
        C.imag = 2*C.real*C.imag + ori_imag;
        C.real = temp;
        len_sq = C.real*C.real + C.imag*C.imag;
        ++count;
    }
    return count;
}

int main(int argc, char *argv[]){
    assert(argc==9);
    // Read in argument 
    int thd_per_proc = atoi(argv[1]);
    double left = atof(argv[2]);
    double right = atof(argv[3]);
    double lower = atof(argv[4]);
    double upper = atof(argv[5]);
    int w = atoi(argv[6]), h = atoi(argv[7]);
    const char *out = argv[8];

    // initialize the MPI 
    MPI_Status status;
    MPI_Comm mpi_comm = MPI_COMM_WORLD;
    rc = MPI_Init(&argc, &argv);
    if(rc != MPI_SUCCESS){
        printf("Error starting MPI program. Terminating\n");
        MPI_Abort(mpi_comm, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

#ifdef DEBUG
    printf("Thread per proc: %d\nReal range: [%f %f]\nImagine range: [%f %f]\n", thd_per_proc, left, right, lower, upper);
    printf("w: %d\nh: %d\nout path: %s\n", w, h, out);
#endif

    // define complex number C
    CPLX C;

    // calculate pixel value
    int *img = (int *)malloc(w*h*sizeof(int));
    int i, j;
    for(j = 0; j < h; j++){
        C.imag = lower + j * (upper - lower)/h;
        for(i = 0; i < w; i++){
            C.real = left + i * (right - left)/w;
            img[j*w+i] = cal_pixel(C);
        }
    }

    write_png(out, w, h, img);
    free(img);
    return 0;
}

