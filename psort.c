#include <stdio.h>
#include <fcntl.h>

typedef struct {
  int key;
  char *pointer;
} key_pointer;

void sort(void* start_add, void* end_add);

int main(int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "%s", "Incorrect number of args\n");
        exit(1);
    }
    char* input_name = argv[1];
    char* output_name = argv[2];
    int num_threads = atoi(argv[3]);

    int input_fd;
    if ((input_fd = open(input_name, O_RDONLY)) == -1) {
        fprintf(stderr, "%s", "Error opening file\n");
        exit(1);
    }

    char* input_map;
    // input_map = mmap(), use fd above

    // idea is to create a key_pointer struct for each record (after loading into memory with mmap) 
    // since that is smaller in size and easier to sort with --> sort by key value

    // while()

    close(input_fd);
}