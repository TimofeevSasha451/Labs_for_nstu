#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_NAME "/tmp/file_processor_fifo"

typedef struct {
    char filename[256];
    int max_changes;
} task_t;

typedef int (*file_processor_t)(const char* input_path, int max_modifications);

int main(int arg_count, char *arg_values[]) 
{
    int fifo_fd = open(FIFO_NAME, O_RDONLY);
    if (fifo_fd < 0) 
    {
        fprintf(stderr, "Error opening FIFO: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    task_t task;
    ssize_t bytes_read = read(fifo_fd, &task, sizeof(task));
    if (bytes_read != sizeof(task)) 
    {
        fprintf(stderr, "Error reading from FIFO: %s\n", strerror(errno));
        close(fifo_fd);
        return EXIT_FAILURE;
    }
    close(fifo_fd);

    printf("Child process received task: %s with max changes: %d\n", task.filename, task.max_changes);

    void *library_handle = dlopen("./libdll.so", RTLD_LAZY);
    if (!library_handle) 
    {
        fprintf(stderr, "Library loading error: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    file_processor_t process_file = (file_processor_t)dlsym(library_handle, "execute_file_processing");
    if (!process_file) 
    {
        fprintf(stderr, "Symbol resolution error: %s\n", dlerror());
        dlclose(library_handle);
        return EXIT_FAILURE;
    }

    int operation_result = process_file(task.filename, task.max_changes);
    dlclose(library_handle);

    return operation_result;
}
