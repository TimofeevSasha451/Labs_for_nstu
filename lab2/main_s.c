#include <stdio.h>
#include <stdlib.h>
#include "dll.h"

int main(int arg_count, char *arg_values[]) 
{
    if (arg_count != 3) 
    {
        fprintf(stderr, "Usage: %s <input_file> <max_changes>\n", arg_values[0]);
        return EXIT_FAILURE;
    }

    const char *input_file = arg_values[1];
    int max_changes = atoi(arg_values[2]);

    int operation_result = execute_file_processing(input_file, max_changes);
    if (operation_result >= 0) {
        printf("Result code (static): %d\n", operation_result);
    }
    return EXIT_SUCCESS;
}
