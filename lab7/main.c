#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>

#define MAX_FILES 100

typedef struct {
    char* filename;
    int replacements;
    int status; // 0: ok, 1: error
} Result;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Result results[MAX_FILES];

typedef int (*file_processor_t)(const char* input_path, int max_modifications);

typedef struct {
    char* file;
    int max_changes;
    int idx;
} ThreadArg;

void* thread_func(void* arg_) {
    ThreadArg* arg = (ThreadArg*)arg_;
    char* input_file = arg->file;
    int max_changes = arg->max_changes;
    int idx = arg->idx;

    int operation_result = -1;
    int status = 0;

    void* library_handle = dlopen("./libdll.so", RTLD_LAZY);
    if (!library_handle) {
        fprintf(stderr, "Library loading error: %s\n", dlerror());
        status = 1;
    } else {
        file_processor_t process_file = (file_processor_t)dlsym(library_handle, "execute_file_processing");
        if (!process_file) {
            fprintf(stderr, "Symbol resolution error: %s\n", dlerror());
            dlclose(library_handle);
            status = 1;
        } else {
            operation_result = process_file(input_file, max_changes);
            dlclose(library_handle);
            if (operation_result < 0) {
                status = 1;
            }
        }
    }

    int lock_ret = pthread_mutex_lock(&mutex);
    if (lock_ret != 0) {
        fprintf(stderr, "Mutex lock failed: %s\n", strerror(lock_ret));
        status = 1;
        operation_result = -1;
    } else {
        results[idx].replacements = operation_result;
        results[idx].status = status;
        pthread_mutex_unlock(&mutex);
    }

    free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <file1> [file2 ...] <max_changes>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int max_changes = atoi(argv[argc - 1]);
    int num_files = argc - 2;
    if (num_files > MAX_FILES || num_files <= 0) {
        fprintf(stderr, "Error: Invalid number of files.\n");
        return EXIT_FAILURE;
    }

    int total_replacements = 0;
    pthread_t threads[MAX_FILES];
    ThreadArg* args[MAX_FILES];

    int mutex_init_ret = pthread_mutex_init(&mutex, NULL);
    if (mutex_init_ret != 0) {
        fprintf(stderr, "Mutex initialization failed: %s\n", strerror(mutex_init_ret));
        return EXIT_FAILURE;
    }

    for (int i = 0; i < num_files; i++) {
        results[i].filename = argv[i + 1];
        results[i].replacements = 0;
        results[i].status = 0; // pending

        args[i] = malloc(sizeof(ThreadArg));
        if (!args[i]) {
            fprintf(stderr, "Memory allocation failed for thread arg.\n");
            continue;
        }
        args[i]->file = argv[i + 1];
        args[i]->max_changes = max_changes;
        args[i]->idx = i;

        if (pthread_create(&threads[i], NULL, thread_func, args[i]) != 0) {
            perror("pthread_create failed");
            free(args[i]);
            results[i].status = 1;
            results[i].replacements = -1;
        }
    }

    for (int i = 0; i < num_files; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    for (int i = 0; i < num_files; i++) {
        if (results[i].status == 0 && results[i].replacements >= 0) {
            printf("File: %s - Replacements: %d\n", results[i].filename, results[i].replacements);
            total_replacements += results[i].replacements;
        } else {
            printf("File: %s - Processing error\n", results[i].filename);
        }
    }

    printf("Total replacements: %d\n", total_replacements);
    return EXIT_SUCCESS;
}
