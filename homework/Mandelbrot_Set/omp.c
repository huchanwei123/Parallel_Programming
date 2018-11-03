/*  Description:                                                    *
 *      The sequential Mandelbort Set code                          *
 *  Author:                                                         *
 *      Chan-Wei Hu                                                 *
 *******************************************************************/
#define PNG_NO_SETJMP

#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <omp.h>
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

#ifdef DEBUG
    printf("Thread per proc: %d\nReal range: [%f %f]\nImagine range: [%f %f]\n", thd_per_proc, left, right, lower, upper);
    printf("w: %d\nh: %d\nout path: %s\n", w, h, out);
#endif

    // define complex number C
    CPLX C;
    
	// allocate memory for local image and initialize to 0
	int *img = (int *)malloc(h*w*sizeof(int));
	memset(img, 0, h*w*sizeof(int));
	assert(img);
	
	int thre = 10;
	long long int cnt = 0;
#pragma omp parallel for schedule(dynamic, thd_per_proc) private(C,cnt) 	
	/* start mandelbrot sort with load balance */
	for(int j = 0; j < h; ++j){
        C.imag = lower + j * ((upper - lower) / h);
        // First, sample some pixel and do it
		for(int i = 0; i < w; i+=3){
			C.real = left + i * ((right - left) / w);
			img[j*w+i] = cal_pixel(C);
			C.real = left + (i+1) * ((right - left) / w);
			img[j*w+(i+1)] = cal_pixel(C);
		}
		
		// scan for which pixel should be calculate
		for(int i = 2; i < w; i+=3){
			for(int idx=thre; idx>0; --idx){
				cnt += img[j*w+(i-idx)];
			}			
			if(i < w-1 && i >= thre && img[j*w+(i+1)] == MAX_ITER && cnt == thre*MAX_ITER){
				img[j*w+i] = MAX_ITER;
			}else{
				C.real = left + i * ((right - left) / w);
				img[j*w+i] = cal_pixel(C);
			}
			cnt = 0;
		}		
		// Sequentail version of each row
		/*for(int i = 0; i < w; ++i){
            C.real = left + i * ((right - left) / w);
			img[j*w+i] = cal_pixel(C);
        }*/
    }
    
    write_png(out, w, h, img);
    free(img);
    return 0;
}

