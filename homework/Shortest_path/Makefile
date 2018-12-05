CC = mpicc
CXX = mpicxx 
LDLIBS = -pthread
CFLAGS = -O3 -fopenmp -w -g3
CXXFLAGS = -O3 -fopenmp 
TARGETS = sssp apsp partition

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(TARGETS:=.o)
