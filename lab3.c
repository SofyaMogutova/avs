#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
unsigned int block(char *arg)
{
    int i = 0;
    unsigned int size = atoi(arg);
    while (arg[i] >= '0' && arg[i] <= '9')
        i++;
    switch (arg[i])
    {
    case 'k':
    case 'K':
        size *= 1024;
        break;
    case 'M':
        size *= 1024 * 1024;
        break;
    }
    return size;
}
void test_RAM(unsigned int block_size, double *read_result, double *write_result)
{
    char *block = malloc(block_size);
    char *buf = malloc(block_size);
    struct timespec start, finish;
    double sec = 0, nsec = 0;
    for (int i = 0; i < block_size; i++)
    {
        buf[i] = rand() % 256;
    }
    clock_gettime(CLOCK_REALTIME, &start);
    for (int i = 0; i < block_size; i++)
    {
        block[i] = buf[i];
    }
    clock_gettime(CLOCK_REALTIME, &finish);
    sec = finish.tv_sec - start.tv_sec;
    nsec = finish.tv_nsec - start.tv_nsec;
    // if (nsec >= 10E9)
    // {
    //     sec++;
    //     nsec -= 10E9;
    // }
    *write_result = sec + nsec / 10E9;
    sec = nsec = 0;
    clock_gettime(CLOCK_REALTIME, &start);
    char tmp;
    for (unsigned int i = 0; i < block_size; i++)
    {
        tmp = block[i];
    }
    clock_gettime(CLOCK_REALTIME, &finish);
    sec = finish.tv_sec - start.tv_sec;
    nsec = finish.tv_nsec - start.tv_nsec;
    // if (nsec >= 10E9)
    // {
    //     sec++;
    //     nsec -= 10E9;
    // }
    *read_result = sec + nsec / 10E9;
    free(buf);
    free(block);
}
void test_SSD(int fd, unsigned int block_size, double *read_result, double *write_result)
{
    char *block = malloc(block_size);
    struct timespec start, finish;
    double sec = 0, nsec = 0;
    for (int i = 0; i < block_size; i++)
    {
        block[i] = rand() % 256;
    }
    clock_gettime(CLOCK_REALTIME, &start);
    write(fd, block, block_size);
    fsync(fd);
    clock_gettime(CLOCK_REALTIME, &finish);
    sec = finish.tv_sec - start.tv_sec;
    nsec = finish.tv_nsec - start.tv_nsec;
    // while (nsec >= 10E9)
    // {
    //     sec++;
    //     nsec -= 10E9;
    // }
    *write_result = sec + nsec / 10E9;
    sec = nsec = 0;
    memset(block,0,block_size);
    lseek(fd,0,SEEK_SET);
    clock_gettime(CLOCK_REALTIME, &start);
    read(fd, block, block_size);
    clock_gettime(CLOCK_REALTIME, &finish);
    sec = finish.tv_sec - start.tv_sec;
    nsec = finish.tv_nsec - start.tv_nsec;
    // while (nsec >= 10E9)
    // {
    //     sec++;
    //     nsec -= 10E9;
    // }
    *read_result = sec + nsec / 10E9;
    free(block);
}
void report(FILE *csv, char *memory_type, unsigned int block_size, int launch_num, double write_time, double average_write, double read_time, double average_read)
{
    fprintf(csv, "%s;", memory_type);
    fprintf(csv, "%d;", block_size);
    fprintf(csv, "char;");
    fprintf(csv, "%d;", block_size);
    fprintf(csv, "%d;", launch_num);
    fprintf(csv, "clock_gettime;");
    fprintf(csv, "%f;", write_time);
    fprintf(csv, "%f;", average_write);
    double wr_band = block_size / average_write / 10E6;
    fprintf(csv, "%f;", wr_band);
    double abs_err_wr = fabs(write_time - average_write);
    fprintf(csv, "%f;", abs_err_wr);
    fprintf(csv, "%f%%;", abs_err_wr / write_time * 100);
    fprintf(csv, "%f;", read_time);
    fprintf(csv, "%f;", average_read);
    double rd_band = block_size / average_read / 10E6;
    fprintf(csv, "%f;", rd_band);
    double abs_err_rd = fabs(read_time - average_read);
    fprintf(csv, "%f;", abs_err_rd);
    fprintf(csv, "%f%%\n", abs_err_rd / read_time * 100);
}
int main(int argc, char **argv)
{
    int opt;
    char memory_type[10];
    unsigned int block_size;
    unsigned int launch_count;
    while ((opt = getopt(argc, argv, "m:b:l:")) != -1)
    {
        switch (opt)
        {
        case 'm':
            strcpy(memory_type, optarg);
            break;
        case 'b':
            block_size = block(optarg);
            break;
        case 'l':
            launch_count = atoi(optarg);
            break;
        default:
            perror("getopt");
            exit(EXIT_FAILURE);
        }
    }
    double *write_results = malloc(launch_count * sizeof(double));
    double *read_results = malloc(launch_count * sizeof(double));
    double average_read = 0, average_write = 0;
    for (int i = 0; i < launch_count; i++)
    {
        printf("Testing %s with %d bytes of memory: %d/%d\n",memory_type,block_size,i+1,launch_count);
        if (strcmp(memory_type, "RAM") == 0)
            test_RAM(block_size, &read_results[i], &write_results[i]);
        else if (strcmp(memory_type, "SSD") == 0)
        {
            int fd = open("test_file.dat", O_RDWR | O_CREAT, S_IRWXU);
            test_SSD(fd, block_size, &read_results[i], &write_results[i]);
            unlink("test_file.dat");
        }
        else if(strcmp(memory_type, "flash") == 0){
            int fd = open("/media/sofya/TRANSCEND/test_file.dat", O_RDWR | O_CREAT, S_IRWXU);
            test_SSD(fd, block_size, &read_results[i], &write_results[i]);
            unlink("/media/sofya/TRANSCEND/test_file.dat");
        }
        printf("\033c");
        average_read += read_results[i];
        average_write += write_results[i];
    
    }
    average_write /= launch_count;
    average_read /= launch_count;
    FILE *csv = fopen("lab3.csv", "a+");
    for (int i = 0; i < launch_count; i++)
    {
        report(csv, memory_type, block_size, i + 1, write_results[i], average_write, read_results[i], average_read);
    }
    return 0;
}