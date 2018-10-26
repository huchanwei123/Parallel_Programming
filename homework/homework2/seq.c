/*  Description:                                                    *
 *      The sequential Mandelbort Set code                          *
 *  Author:                                                         *
 *      Chan-Wei Hu                                                 *
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <math.h>
#include <assert.h>

#define MAX_ITER 10000

// define data structure for complex number
struct complex_num{
    float real, imag;
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
    int count = 0, max_iter = 256;
    float len_sq = 0;
    // start calculate pixel
    while((count < max_iter) && len_sq < 4.0){
        float ori_real = C.real;
        C.real = pow(C.real, 2) - pow(C.imag, 2) + C.real;
        C.imag = 2*(ori_real)*(C.imag) + C.imag;
        len_sq = (C.real*C.real) + (C.imag*C.imag);
        count++;
    }
    return count;
}

int main(int argc, char *argv[]){
    assert(argc==9);
    // Read in argument 
    int thd_per_proc = atoi(argv[1]);
    float real_lower = atof(argv[2]);
    float real_upper = atof(argv[3]);
    float imag_lower = atof(argv[4]);
    float imag_upper = atof(argv[5]);
    int w = atoi(argv[6]), h = atoi(argv[7]);
    const char *out = argv[8];

    // divide the resolution to value

#ifdef DEBUG
    printf("Thread per proc: %d\nReal range: [%f %f]\nImagine range: [%f %f]\n", thd_per_proc, real_lower, real_upper, imag_lower, imag_upper);
    printf("w: %d\nh: %d\nout path: %s\n", w, h, out);
#endif

    // check real_intv and imag_intv
    float real_intv = (real_upper - real_lower)/w;
    float imag_intv = (imag_upper - imag_lower)/h;

    // define complex number C
    CPLX C;

    // calculate pixel value
    int *img = (int *)malloc(w*h*sizeof(int));
    int pixel = 0;
    int i, j;
    for(j = 0; j < h; j++){
        for(i = 0; i < w; i++){
            C.real = real_lower + i*real_intv;
            C.imag = imag_lower + j*imag_intv;
            img[j*w+i] = cal_pixel(C);
        }
    }
    write_png(out, w, h, img);
    free(img);
    return 0;
}

