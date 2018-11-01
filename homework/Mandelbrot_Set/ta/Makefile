CC = mpicc
CXX = mpicxx
LDLIBS = -lpng
CFLAGS = -O3 -fopenmp
CXXFLAGS = -O3 -fopenmp
TARGETS = seq mpi_static mpi_dynamic omp hybrid

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(TARGETS:=.o)
