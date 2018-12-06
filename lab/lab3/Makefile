NVCCFLAGS  = -O3
NVCC     = nvcc 
CXXFLAGS = -O3
CXX      = g++

TARGETS = sobel sobel_cuda

.PHONY: all
all: $(TARGETS)

sobel_cuda: sobel.cu
	$(NVCC) $(NVCCFLAGS) -o $@ $<

sobel: sobel.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(TARGETS)
