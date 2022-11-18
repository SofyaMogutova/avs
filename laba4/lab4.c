#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
struct matrix{
    double** m;
    unsigned int size;
};

void init_rand_matrix(struct matrix *m, unsigned int size){
    m->size=size;
    m->m=malloc(size*sizeof(double*));
    for(int i=0;i<size;i++){
        m->m[i]=malloc(size*sizeof(double));
        for(int j=0;j<size;j++){
            m->m[i][j]=rand()%100;
        }
    }
}

void init_zero_matrix(struct matrix *m, unsigned int size){
    m->size=size;
    m->m=calloc(size,sizeof(double*));
    for(int i=0;i<size;i++){
        m->m[i]=calloc(size,sizeof(double));
    }
}

void free_matrix(struct matrix *m){
    for(int i=0;i<m->size;i++){
        free(m->m[i]);
    }
    free(m->m);
}

struct matrix DGEMM(struct matrix *m1, struct matrix *m2, double *time){
    unsigned int size = m1->size;
    struct matrix result;
    init_zero_matrix(&result, size);
    struct timespec start,finish;
    clock_gettime(CLOCK_REALTIME,&start);
    for(int i=0;i<size;i++){
        for(int j=0;j<size;j++){
            for(int k=0;k<size;k++){
                result.m[i][j]+=m1->m[i][k]*m2->m[k][j];
            }
        }
    }
    clock_gettime(CLOCK_REALTIME,&finish);
    double sec=finish.tv_sec-start.tv_sec;
    double nsec=finish.tv_nsec-start.tv_nsec;
    *time = sec+nsec/1E9;
    return result;
}

struct matrix DGEMM_opt_1(struct matrix *m1, struct matrix *m2, double *time){
    unsigned int size = m1->size;
    struct matrix result;
    init_zero_matrix(&result, size);
    struct timespec start,finish;
    clock_gettime(CLOCK_REALTIME,&start);
    for(int i=0;i<size;i++){
        for(int j=0;j<size;j++){
            for(int k=0;k<size;k++){
                result.m[i][k]+=m1->m[i][j]*m2->m[j][k];
            }
        }
    }
    clock_gettime(CLOCK_REALTIME,&finish);
    double sec=finish.tv_sec-start.tv_sec;
    double nsec=finish.tv_nsec-start.tv_nsec;
    *time = sec+nsec/1E9;
    return result;
}

int main(int argc, char** argv){
    int opt;
    int size=0;
    int optimization=0;
    while((opt=getopt(argc,argv,"s:o:"))!=-1){
        switch (opt)
        {
        case 's':
            size=atoi(optarg);
            break;
        case 'o':
            optimization=atoi(optarg);
            break;
        default:
            perror("getopt");
            exit(EXIT_FAILURE);
        }
    }
    printf("%d\n",size);
    if(size==0){
        printf("Enter size of matrix!\n");
        exit(EXIT_FAILURE);
    }
    //size=3;
    struct matrix m1;
    init_rand_matrix(&m1, size);
    struct matrix m2;
    init_rand_matrix(&m2, size);
    double time;
    // m1.m[0][0]=1;
    // m1.m[0][1]=2;
    // m1.m[0][2]=3;
    // m1.m[1][0]=4;
    // m1.m[1][1]=5;
    // m1.m[1][2]=6;
    // m1.m[2][0]=7;
    // m1.m[2][1]=8;
    // m1.m[2][2]=9;

    // m2.m[0][0]=1;
    // m2.m[0][1]=2;
    // m2.m[0][2]=3;
    // m2.m[1][0]=4;
    // m2.m[1][1]=5;
    // m2.m[1][2]=6;
    // m2.m[2][0]=7;
    // m2.m[2][1]=8;
    // m2.m[2][2]=9;
   
    
    // for(int i=0;i<3;i++){
    //     for(int j=0;j<3;j++){
    //         printf("%f ",result1.m[i][j]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");
    // for(int i=0;i<3;i++){
    //     for(int j=0;j<3;j++){
    //         printf("%f ",result2.m[i][j]);
    //     }
    //     printf("\n");
    // }
    FILE* csv=fopen("lab4.csv","a");
    if(!csv){
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    if(optimization==0){
        struct matrix result=DGEMM(&m1,&m2,&time);
        fprintf(csv,"DGEMM;%d;%f\n",size,time);
        free_matrix(&result);
    }
    else if(optimization==1){
        struct matrix result=DGEMM_opt_1(&m1,&m2,&time);
        fprintf(csv,"DGEMM_opt_1;%d;%f\n",size,time);
        free_matrix(&result);
    }
    fclose(csv);
    free_matrix(&m1);
    free_matrix(&m2);
}