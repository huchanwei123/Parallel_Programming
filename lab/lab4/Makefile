NVCCFLAGS  = -O2 -std=c++11 -Xptxas=-v -arch=sm_61
NVCC     = nvcc 
LDFLAGS = -lpng -lz

TARGETS = sobel

.PHONY: all
all: $(TARGETS)

sobel: sobel_cuda.cu
	$(NVCC) $(NVCCFLAGS) $(LDFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(TARGETS)
