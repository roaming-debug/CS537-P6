#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

typedef struct chunk_info
{
    int si;
    int ei;
} chunk_info;

typedef struct two_chunk_info
{
    chunk_info chunk1;
    chunk_info chunk2;
} two_chunk_info;

typedef struct record
{
    uint32_t key;
    void* addr;
} record;

record* records;

void merge(two_chunk_info* two_chunk_info)
{
    // printf("merge:\n1st chunk: si: %d, ei: %d, 2nd chunk: si: %d, ei: %d\n", 
    // two_chunk_info->chunk1.si, two_chunk_info->chunk1.ei, two_chunk_info->chunk2.si, two_chunk_info->chunk2.ei);
    int ci = 0; // current block data that will be filled
    int i1 = two_chunk_info->chunk1.si;
    int i2 = two_chunk_info->chunk2.si;
    record* m = (record*) malloc(sizeof(record)*(two_chunk_info->chunk2.ei-two_chunk_info->chunk1.si+1));
    while(i1 <= two_chunk_info->chunk1.ei && i2 <= two_chunk_info->chunk2.ei)
    {
        if (records[i1].key < records[i2].key)
        {
            m[ci] = records[i1++];
        }
        else
        {
            m[ci] = records[i2++];
        }
        ci++;
    }
    while(i1 <= two_chunk_info->chunk1.ei)
    {
        m[ci++] = records[i1++];
    }
    while(i2 <= two_chunk_info->chunk2.ei)
    {
        m[ci++] = records[i2++];
    }
    for (int i = two_chunk_info->chunk1.si; i <= two_chunk_info->chunk2.ei; i++) 
    {
        records[i] = m[i-two_chunk_info->chunk1.si];
    }
    free(m);
}

void sort(chunk_info *info)
{
    // printf("si: %d\nei: %d\n", info->si, info->ei);
}


const int record_size = 100;

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

    // memcpy(out_mem, in_mem, in_stat.st_size);

    const int record_num = in_stat.st_size / record_size;
    records = (record*) malloc(sizeof(record) * in_stat.st_size/record_size);
    record* record_addr = (record*) in_mem;
    for (int i = 0; i < record_num; i++)
    {
        records[i].key = * (int*) record_addr;
        records[i].addr = (void*) record_addr;
    }

    int start_i = 0;
    int chunk_size = record_num / num_threads;
    chunk_info* chunk_infos = (chunk_info*) malloc(sizeof(chunk_info) * num_threads);
    int chunk_num = num_threads;
    for (int i = 0; i < num_threads; i++)
    {
        chunk_infos[i].si = start_i;
        chunk_infos[i].ei = i == num_threads-1? record_num - 1 : start_i + chunk_size - 1;
        printf("main: si: %d, ei: %d\n", chunk_infos[i].si, chunk_infos[i].ei);
        pthread_create(&ps[i], NULL, (void *(*)(void *))sort, &chunk_infos[i]);
        start_i += chunk_size;
    }
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(ps[i], NULL);
    }

    two_chunk_info* two_chunk_infos = (two_chunk_info*) malloc(sizeof(two_chunk_info) * (chunk_num/2));
    while (chunk_num > 1)
    {
        for (int i = 0; i < chunk_num/2; i++)
        {
            two_chunk_infos[i].chunk1.si = chunk_infos[2*i].si;
            two_chunk_infos[i].chunk1.ei = chunk_infos[2*i].ei;
            two_chunk_infos[i].chunk2.si = chunk_infos[2*i+1].si;
            two_chunk_infos[i].chunk2.ei = chunk_infos[2*i+1].ei;
            pthread_create(&ps[i], NULL, (void *(*)(void *))merge, &two_chunk_infos[i]);
        }
        for (int i = 0; i < chunk_num/2; i++)
        {
            pthread_join(ps[i], NULL);
            chunk_infos[i].si = two_chunk_infos[i].chunk1.si;
            chunk_infos[i].ei = two_chunk_infos[i].chunk2.ei;
        }
        if (chunk_num%2 == 1)
        {
            chunk_infos[chunk_num/2].si = chunk_infos[chunk_num-1].si;
            chunk_infos[chunk_num/2].ei = chunk_infos[chunk_num-1].ei;
            chunk_num = chunk_num/2+1;
        }
        else
        {
            chunk_num /= 2;
        }
    }
    free(two_chunk_infos);
    
    void* curr_out_mem = out_mem;
    for (int i = 0; i < record_num; i++)
    {
        memcpy(curr_out_mem, records[i].addr, record_size);
        curr_out_mem += record_size;
    }

    fsync(out_fd);
    free(records);
    free(ps);
    munmap(out_mem, out_stat.st_size);
    munmap(in_mem, in_stat.st_size);
    close(in_fd);
    close(out_fd);
    return 0;
}