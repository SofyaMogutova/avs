#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <omp.h>

typedef struct _matrix{
    double** m;
    unsigned int size;
}matrix;

void init_rand_matrix(matrix *m, unsigned int size){
    m->size=size;
    m->m=malloc(size*sizeof(double*));
    #pragma omp parallel for
    for(int i=0;i<size;i++){
        m->m[i]=malloc(size*sizeof(double));
        for(int j=0;j<size;j++){
            m->m[i][j]=rand()%100;
        }
    }
}

void init_zero_matrix(matrix *m, unsigned int size){
    m->size=size;
    m->m=calloc(size,sizeof(double*));
    #pragma omp parallel for
    for(int i=0;i<size;i++){
        m->m[i]=calloc(size,sizeof(double));
    }
}

void free_matrix(matrix *m){
    for(int i=0;i<m->size;i++){
        free(m->m[i]);
    }
    free(m->m);
}

matrix DGEMM(matrix *m1, matrix *m2){
    unsigned int size = m1->size;
    matrix result;
    init_zero_matrix(&result, size);
    #pragma omp parallel for
    for(int i=0;i<size;i++){
        for(int j=0;j<size;j++){
            for(int k=0;k<size;k++){
                result.m[i][j]+=m1->m[i][k]*m2->m[k][j];
            }
        }
    }
    return result;
}

int main(int argc, char** argv){
    int threads_count=1;
    int size=0;
    char c;
    while ((c=getopt(argc, argv, "s:t:"))!=-1)
    {
        switch (c)
        {
        case 't':
            threads_count=atoi(optarg);
            break;
        case 's':
            size=atoi(optarg);
            break;
        default:
            perror("getopt");
            exit(EXIT_FAILURE);
        }
    }
    printf("Size: %d, Threads: %d\n",size,threads_count);
    omp_set_num_threads(threads_count);
    struct timespec start,finish;
    matrix m1,m2;
    clock_gettime(CLOCK_REALTIME,&start);
    matrix result;
    init_rand_matrix(&m1,size);
    init_rand_matrix(&m2,size);
    result = DGEMM(&m1,&m2);
    clock_gettime(CLOCK_REALTIME,&finish);
    double sec,nsec;
    sec=finish.tv_sec-start.tv_sec;
    nsec=finish.tv_nsec-start.tv_nsec;
    double time=sec+nsec/1E9;
    printf("Time: %fs\n",time);
    FILE* csv = fopen("lab5.csv","a");
    fprintf(csv,"OpenMP Threads;%d;%d;%f\n",size,threads_count,time);
    fclose(csv);
}