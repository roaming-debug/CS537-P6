#include <sys/types.h>
#include <sys/mman.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

typedef struct chunk_info {
    void* start_addr;
    void* end_addr;
} chunk_info;

void sort(chunk_info* info);

int main(int argc, char** argv) {
    char* input_name = argv[1];
    char* output_name = argv[2];
    int num_threads = atoi(argv[3]);
    
    pthread_t* ps = malloc(sizeof(pthread_t) * num_threads);
    FILE* fp = fopen(input_name, "rw");
    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    int fd = fileno(fp);
    void* start_mapp_addr = mmap(NULL, file_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);

    while(num_threads >= 1) {
        void* start_add = start_mapp_addr;
        int chunksize = file_size/num_threads;
        for (int i = 0; i < num_threads; i++) {
            chunk_info info;
            info.start_addr = start_add;
            info.end_addr = start_add+chunksize-1;
            pthread_create(&ps[i], NULL, sort, &info);
        }
    }

    free(ps);
    fclose(fp);
    return 0;
}