#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    /******************************* load data *********************************/
    // open input file
    int E, V;
    FILE *in_fp, *in_fp_2;
    in_fp = fopen(argv[1], "r");
    in_fp_2 = fopen(argv[2], "r");
    if(in_fp == NULL) printf("Failed on opening file\n");

    int src, dst;
    int diff = 0; 
    while(fread(&src, sizeof(int), 1, in_fp) == 1){
        fread(&dst, sizeof(int), 1, in_fp_2);
        if(src != dst) diff++;
    }
    if(diff > 0){
        printf("\033[0;31m");
        printf("wrong answer\n");
        printf("\033[0m");
    }else{ 
        printf("\033[0;32m");
        printf("Pass !!\n");
        printf("\033[0m");
    }
    fclose(in_fp);
    fclose(in_fp_2);
} 
