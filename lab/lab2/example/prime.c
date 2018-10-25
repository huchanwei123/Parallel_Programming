#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
int isPrime(int i) {
    int sqrt_i = (int)sqrt((double)i);
    int j;
    for (j = 2; j <= sqrt_i; ++ j) {
        if (i % j == 0) return 0;
    }
    return 1;
}
int main(int argc, char** argv) {
    assert(argc == 2);
    int N = atoi(argv[1]);
    int i;
    int count = 0;
    for (i = 2; i <= N; i ++) {
        count += isPrime(i);
    }
    printf("There are %d prime numbers <= %d\n", count, N);
    return 0;
}
