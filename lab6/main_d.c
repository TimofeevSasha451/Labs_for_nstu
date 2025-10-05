#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <semaphore.h>

#define SHM_NAME "/file_processor_shm"
#define SEM_NAME "/file_processor_sem"

typedef struct {
    int total_files;
    int current_index;
    int max_changes;
    char filenames[0]; 
} shared_data_t;

typedef int (*file_processor_t)(const char* input_path, int max_modifications);

int main(int arg_count, char *arg_values[]) 
{
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) 
    {
        fprintf(stderr, "Error opening shared memory: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct stat sb;
    if (fstat(shm_fd, &sb) == -1) {
        fprintf(stderr, "Error getting shared memory size: %s\n", strerror(errno));
        close(shm_fd);
        return EXIT_FAILURE;
    }
    size_t shm_size = sb.st_size;

    shared_data_t *shared_data = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        fprintf(stderr, "Error mapping shared memory: %s\n", strerror(errno));
        close(shm_fd);
        return EXIT_FAILURE;
    }

    sem_t *sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        fprintf(stderr, "Error opening semaphore: %s\n", strerror(errno));
        munmap(shared_data, shm_size);
        close(shm_fd);
        return EXIT_FAILURE;
    }

    char filename[256];
    int max_changes;
    int task_index;

    if (sem_wait(sem) == -1) {
        fprintf(stderr, "Error waiting on semaphore: %s\n", strerror(errno));
        sem_close(sem);
        munmap(shared_data, shm_size);
        close(shm_fd);
        return EXIT_FAILURE;
    }

    task_index = shared_data->current_index;
    if (task_index < shared_data->total_files) {
        char *ptr = shared_data->filenames;
        for (int i = 0; i < task_index; i++) {
            ptr += strlen(ptr) + 1;
        }
        strncpy(filename, ptr, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        max_changes = shared_data->max_changes;
        
        shared_data->current_index++;
    } else {
        sem_post(sem);
        sem_close(sem);
        munmap(shared_data, shm_size);
        close(shm_fd);
        printf("No more tasks available\n");
        return EXIT_FAILURE;
    }

    sem_post(sem);

    printf("Child process received task: %s with max changes: %d\n", filename, max_changes);

    void *library_handle = dlopen("./libdll.so", RTLD_LAZY);
    if (!library_handle) 
    {
        fprintf(stderr, "Library loading error: %s\n", dlerror());
        sem_close(sem);
        munmap(shared_data, shm_size);
        close(shm_fd);
        return EXIT_FAILURE;
    }

    file_processor_t process_file = (file_processor_t)dlsym(library_handle, "execute_file_processing");
    if (!process_file) 
    {
        fprintf(stderr, "Symbol resolution error: %s\n", dlerror());
        dlclose(library_handle);
        sem_close(sem);
        munmap(shared_data, shm_size);
        close(shm_fd);
        return EXIT_FAILURE;
    }

    int operation_result = process_file(filename, max_changes);
    dlclose(library_handle);

    sem_close(sem);
    munmap(shared_data, shm_size);
    close(shm_fd);

    if (operation_result < 0) {
        return EXIT_FAILURE;
    }
    return operation_result;
}
