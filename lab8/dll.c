#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define DATA_BLOCK_SIZE 8192

char* generate_modified_path(const char* input_path) 
{
    const char* extension_marker = strrchr(input_path, '.');
    size_t base_name_len = extension_marker ? (size_t)(extension_marker - input_path) : strlen(input_path);

    char* modified_path = malloc(base_name_len + 8);
    if (!modified_path) return NULL;

    memcpy(modified_path, input_path, base_name_len);
    modified_path[base_name_len] = '\0';
    strcat(modified_path, "_edited");
    return modified_path;
}

int execute_file_processing(const char* input_path, int max_modifications) 
{
    if (max_modifications < 0) 
    {
        fprintf(stderr, "Error: Modification count cannot be negative.\n");
        return -1;
    }

    char *output_path = generate_modified_path(input_path);
    if (!output_path) 
    {
        fprintf(stderr, "Error: Memory allocation failed for output path.\n");
        return -1;
    }

    int input_fd = open(input_path, O_RDONLY);
    if (input_fd < 0) 
    {
        fprintf(stderr, "Error opening input file: %s\n", strerror(errno));
        free(output_path);
        return -1;
    }

    int output_fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd < 0) 
    {
        fprintf(stderr, "Error creating output file: %s\n", strerror(errno));
        close(input_fd);
        free(output_path);
        return -1;
    }

    unsigned char read_buffer[DATA_BLOCK_SIZE / 2];
    unsigned char write_buffer[DATA_BLOCK_SIZE];
    ssize_t bytes_processed;
    int modifications_count = 0;
    size_t write_position = 0;

    while ((bytes_processed = read(input_fd, read_buffer, sizeof(read_buffer))) > 0) 
    {
        for (ssize_t i = 0; i < bytes_processed; i++) 
        {
            write_buffer[write_position++] = read_buffer[i];
            
            if (read_buffer[i] == 0x20 && modifications_count < max_modifications) 
            {
                write_buffer[write_position++] = 0x2E;
                modifications_count++;
            }

            if (write_position >= (DATA_BLOCK_SIZE - 1)) 
            {
                if (write(output_fd, write_buffer, write_position) != write_position) 
                {
                    fprintf(stderr, "Error writing data: %s\n", strerror(errno));
                    close(input_fd);
                    close(output_fd);
                    free(output_path);
                    return -1;
                }
                write_position = 0;
            }
        }
    }

    if (write_position > 0) 
    {
        if (write(output_fd, write_buffer, write_position) != write_position) 
        {
            fprintf(stderr, "Error during final write: %s\n", strerror(errno));
            close(input_fd);
            close(output_fd);
            free(output_path);
            return -1;
        }
    }

    if (bytes_processed < 0) 
    {
        fprintf(stderr, "Error reading data: %s\n", strerror(errno));
        close(input_fd);
        close(output_fd);
        free(output_path);
        return -1;
    }

    close(input_fd);
    close(output_fd);
    free(output_path);
    return modifications_count;
}
