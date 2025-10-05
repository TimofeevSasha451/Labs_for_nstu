#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef int (*file_processor_t)(const char* input_path, int max_modifications);

int main(int arg_count, char *arg_values[]) 
{
    if (arg_count != 3) 
    {
        fprintf(stderr, "Usage: %s <input_file> <max_changes>\n", arg_values[0]);
        return EXIT_FAILURE;
    }

    const char *input_file = arg_values[1];
    int max_changes = atoi(arg_values[2]);

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

    int operation_result = process_file(input_file, max_changes);
    dlclose(library_handle);

    if (operation_result >= 0) {
        printf("Result code (dynamic): %d\n", operation_result);
    }
    return EXIT_SUCCESS;
}
