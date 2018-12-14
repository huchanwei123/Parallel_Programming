NVFLAGS  := -std=c++11 -O3 -arch=sm_61
CXXFLAGS := -fopenmp -O3 -w
LDFLAGS  := -lm
MPILIBS  := -I/opt/intel/compilers_and_libraries_2018.2.199/linux/mpi/intel64/include \
			-L/opt/intel/compilers_and_libraries_2018.2.199/linux/mpi/intel64/lib -lmpi
EXES     := seq apsp multi_gpu multi_node 

alls: $(EXES)

clean:
	rm -f $(EXES)

seq: seq.cc
	g++ $(CXXFLAGS) -o $@ $?

apsp: apsp.cu
	nvcc $(NVFLAGS) -Xcompiler="$(CXXFLAGS)" $(LDFLAGS) -o $@ $?

multi_gpu: multi_gpu.cu
	nvcc $(NVFLAGS) -Xcompiler="$(CXXFLAGS)" $(LDFLAGS) -o $@ $?

multi_node: multi_node.cu
	nvcc $(NVFLAGS) $(MPILIBS) -Xcompiler="$(CXXFLAGS)" -o $@ $?
