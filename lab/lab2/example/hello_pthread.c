#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void* hello(void* threadid) {
    int* tid = (int*)threadid;
    printf("Hello, thread #%d!\n", *tid);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    assert(argc == 2);
    int num_threads = atoi(argv[1]);
    pthread_t threads[num_threads];
    int rc;
    int ID[num_threads];
    int t;
    for (t = 0; t < num_threads; t++) {
        ID[t] = t;
        printf("In main: creating thread %d\n", t);
        rc = pthread_create(&threads[t], NULL, hello, (void*)&ID[t]);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    pthread_exit(NULL);
}
