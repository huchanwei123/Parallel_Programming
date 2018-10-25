CC = gcc
MPICC = mpicc
CFLAGS = -O3 -pthread -lm -fopenmp

pthread_pi: pthread_pi.c
            $(CC) pthread_pi.c $(CFLAGS) -o pthread_pi

pthread_cond: pthread_cond.c
            $(CC) pthread_cond.c $(CFLAGS) -o pthread_cond

omp_prime: omp_prime.c
            $(CC) omp_prime.c $(CFLAGS) -o omp_prime

hybrid_pi: hybrid_pi.c
            $(MPICC) hybrid_pi.c $(CFLAGS) -o hybrid_pi
