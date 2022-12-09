#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

typedef struct _matrix{
    double** m;
    unsigned int size;
}matrix;

typedef struct _bounds
{
    int up,down;
}bounds;

typedef struct _matrix_block{
    matrix* matrix;
    bounds bounds;
}matrix_block;

void* init_rand_matrix_thr(void* arg){
    matrix_block *block=(matrix_block*)arg;
    for(int i=block->bounds.up;i<block->bounds.down;i++){
        block->matrix->m[i]=malloc(block->matrix->size*sizeof(double));
        for(int j=0;j<block->matrix->size;j++){
            block->matrix->m[i][j]=rand()%100;
        }
    }
    pthread_exit(NULL);
}

void* init_zero_matrix_thr(void* arg){
    matrix_block *block=(matrix_block*)arg;
    for(int i=block->bounds.up;i<block->bounds.down;i++){
        block->matrix->m[i]=calloc(block->matrix->size,sizeof(double));
    }
    pthread_exit(NULL);
}

void init_rand_matrix(matrix *m, unsigned int size, int threads_count){
    m->size=size;
    m->m=malloc(size*sizeof(double*));
    pthread_t *threads = malloc(threads_count*sizeof(pthread_t));
    matrix_block *blocks = malloc(threads_count*sizeof(matrix_block));
    for(int i=0;i<threads_count;i++){
        bounds b;
        b.up=i*size/threads_count;
        b.down=(i+1)*size/threads_count;
        blocks[i].bounds=b;
        blocks[i].matrix=m;
        pthread_create(&threads[i],NULL,init_rand_matrix_thr,&blocks[i]);
    }
    for(int i=0;i<threads_count;i++){
        pthread_join(threads[i],NULL);
    }
    free(threads);
    free(blocks);
}

void init_zero_matrix(matrix *m, unsigned int size, int threads_count){
    m->size=size;
    m->m=calloc(size,sizeof(double*));
    pthread_t *threads = malloc(threads_count*sizeof(pthread_t));
    matrix_block *blocks = malloc(threads_count*sizeof(matrix_block));
    for(int i=0;i<threads_count;i++){
        bounds b;
        b.up=i*size/threads_count;
        b.down=(i+1)*size/threads_count;
        blocks[i].bounds=b;
        blocks[i].matrix=m;
        pthread_create(&threads[i],NULL,init_zero_matrix_thr,&blocks[i]);
    }
    for(int i=0;i<threads_count;i++){
        pthread_join(threads[i],NULL);
    }
    free(threads);
    free(blocks);
}

void free_matrix(matrix *m){
    for(int i=0;i<m->size;i++){
        free(m->m[i]);
    }
    free(m->m);
}

typedef struct _matrices
{
    matrix *m1, *m2, *result;
    bounds bounds;
}matrices;


void* DGEMM_thr(void* arg){
    matrices* ms = (matrices*)arg;
    for(int i=ms->bounds.up;i<ms->bounds.down;i++){
        for(int j=0;j<ms->result->size;j++){
            for(int k=0;k<ms->result->size;k++){
                ms->result->m[i][j]+=ms->m1->m[i][k]*ms->m2->m[k][j];
            }
        }
    }
    return NULL;
}

matrix DGEMM(matrix *m1, matrix *m2, int threads_count){
    unsigned int size = m1->size;
    matrix result;
    init_zero_matrix(&result, size, threads_count);
    pthread_t *threads = malloc(threads_count*sizeof(pthread_t));
    matrices *ms = malloc(threads_count*sizeof(matrices));
    for(int i=0;i<threads_count;i++){
        bounds b;
        b.up=i*size/threads_count;
        b.down=(i+1)*size/threads_count;
        ms[i].bounds=b;
        ms[i].m1=m1;
        ms[i].m2=m2;
        ms[i].result=&result;
        pthread_create(&threads[i],NULL,DGEMM_thr,&ms[i]);
    }
    for(int i=0;i<threads_count;i++){
        pthread_join(threads[i],NULL);
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
    struct timespec start,finish;
    matrix m1,m2;
    clock_gettime(CLOCK_REALTIME,&start);
    init_rand_matrix(&m1,size,threads_count);
    init_rand_matrix(&m2,size,threads_count);
    matrix result = DGEMM(&m1,&m2,threads_count);
    clock_gettime(CLOCK_REALTIME,&finish);
    double sec,nsec;
    sec=finish.tv_sec-start.tv_sec;
    nsec=finish.tv_nsec-start.tv_nsec;
    double time=sec+nsec/1E9;
    FILE* csv = fopen("lab5.csv","a");
    fprintf(csv,"POSIX Threads;%d;%d;%f\n",size,threads_count,time);
    fclose(csv);
}