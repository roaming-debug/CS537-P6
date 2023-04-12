#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

typedef struct chunk_info
{
    void *start_addr;
    void *end_addr;
} chunk_info;

void sort(chunk_info *info);

void sort(chunk_info *info)
{
    // printf("1");
}

int main(int argc, char **argv)
{
    char *input_name = argv[1];
    char* output_name = argv[2];
    int num_threads = atoi(argv[3]);

    pthread_t *ps = malloc(sizeof(pthread_t) * num_threads);
    
    int in_fd = open(input_name, O_RDONLY);
    struct stat in_stat;
    struct stat out_stat;
    fstat(in_fd, &in_stat);
    int out_fd = open(output_name, O_CREAT|O_RDWR, in_stat.st_mode);
    fstat(out_fd, &out_stat);

    ftruncate(out_fd, in_stat.st_size);
    
    void *in_mem = mmap(NULL, in_stat.st_size, PROT_READ, MAP_SHARED_VALIDATE, in_fd, 0);
    void *out_mem = mmap(NULL, out_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED_VALIDATE, out_fd, 0);

    memcpy(out_mem, in_mem, in_stat.st_size);

    void *start_add = out_mem;
    int chunksize = in_stat.st_size / num_threads;
    for (int i = 0; i < num_threads; i++)
    {
        chunk_info info;
        info.start_addr = start_add;
        info.end_addr = start_add + chunksize - 1;
        pthread_create(&ps[i], NULL, (void *(*)(void *))sort, &info);
    }
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(ps[i], NULL);
    }
    num_threads /= 2;
    
    
    fsync(out_fd);
    free(ps);
    munmap(out_mem, out_stat.st_size);
    munmap(in_mem, in_stat.st_size);
    close(in_fd);
    close(out_fd);
    return 0;
}