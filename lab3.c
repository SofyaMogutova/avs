#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <time.h>
unsigned int block(char* arg){
    int i=0;
    unsigned int size=atoi(arg);
    while(arg[i]>='0'&&arg[i]<='9') i++;
    switch(arg[i]){
        case 'k':
        case 'K':
            size*=1024;
            break;
        case 'M':
            size*=1024*1024;
            break;
    }
    return size;
}
void test_RAM(unsigned int block_size,double* read_result, double* write_result){
    char* block=malloc(block_size);
    char tmp;
    struct timespec start,finish;
    double sec=0,nsec=0;
    for(unsigned int i=0; i<block_size;i++){
        tmp=rand()%256;
        clock_gettime(CLOCK_REALTIME,&start);
        block[i]=tmp;
        clock_gettime(CLOCK_REALTIME,&finish);
        sec+=finish.tv_sec-start.tv_sec;
        nsec+=finish.tv_nsec-start.tv_nsec;
        if(nsec>=10E9){
            sec++;
            nsec-=10E9;
        }
    }
    *write_result=sec+nsec/10E9;
    sec=nsec=0;
    for(unsigned int i=0; i<block_size;i++){
        clock_gettime(CLOCK_REALTIME,&start);
        tmp=block[i];
        clock_gettime(CLOCK_REALTIME,&finish);
        sec+=finish.tv_sec-start.tv_sec;
        nsec+=finish.tv_nsec-start.tv_nsec;
        if(nsec>=10E9){
            sec++;
            nsec-=10E9;
        }
    }
    *read_result=sec+nsec/10E9;
}
int main(int argc, char** argv){
    int opt;
    char memory_type[10];
    unsigned int block_size;
    unsigned int launch_count;
    while((opt=getopt(argc,argv,"m:b:l:"))!=-1){
        switch(opt){
            case 'm':
                strcpy(memory_type,optarg);
                break;
            case 'b':
                block_size=block(optarg);
                break;
            case 'l':
                launch_count=atoi(optarg);
                break;
            default:
                perror("getopt");
                exit(EXIT_FAILURE);
        }
    }
    printf("Memory type: %s\nBlock size: %d\nLaunch count: %d\n",memory_type,block_size,launch_count);
    double* write_results=malloc(launch_count*sizeof(double));
    double* read_results=malloc(launch_count*sizeof(double));
    for(int i=0;i<launch_count;i++){
        if(strcmp(memory_type,"RAM")) test_RAM(block_size,&read_results[i],&write_results[i]);
        printf("#%d\tRead: %fs\tWrite: %fs\n",i,read_results[i],write_results[i]);
    }
    return 0;
}