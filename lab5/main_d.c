#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>

#define QUEUE_NAME "/file_processor_queue"

typedef struct {
    char filename[256];
    int max_changes;
} task_t;

typedef int (*file_processor_t)(const char* input_path, int max_modifications);

int main(int arg_count, char *arg_values[]) 
{
    mqd_t mq = mq_open(QUEUE_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) 
    {
        fprintf(stderr, "Error opening message queue: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct mq_attr attr;
    if (mq_getattr(mq, &attr) == -1) {
        fprintf(stderr, "Error getting queue attributes: %s\n", strerror(errno));
        mq_close(mq);
        return EXIT_FAILURE;
    }

    task_t task;
    ssize_t bytes_read = mq_receive(mq, (char*)&task, attr.mq_msgsize, NULL);
    
    if (bytes_read != sizeof(task)) 
    {
        if (bytes_read == -1) {
            fprintf(stderr, "Error receiving from queue: %s\n", strerror(errno));
        } else {
            fprintf(stderr, "Incomplete message received\n");
        }
        mq_close(mq);
        return EXIT_FAILURE;
    }
    
    mq_close(mq);

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

    if (operation_result < 0) {
        return EXIT_FAILURE;
    }
    return operation_result;
}
