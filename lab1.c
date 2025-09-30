#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <number_of_spaces_to_remove>\n", argv[0]);
        return -1;
    }

    char* input_filename = argv[1];
    int max_replacements = atoi(argv[2]); 
    
    if (max_replacements <= 0) {
        fprintf(stderr, "Error: Number of replacements must be positive\n");
        return -1;
    }

    
    char output_filename[256];
    strcpy(output_filename, input_filename);
    char* dot = strrchr(output_filename, '.');
    if (dot != NULL) {
        strcpy(dot, ".out");
    } else {
        strcat(output_filename, ".out");
    }

    int fd_input = -1;
    int fd_output = -1;
    int replace_count = 0;

    
    fd_input = open(input_filename, O_RDONLY);
    if (fd_input == -1) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", input_filename);
        return -1;
    }

    
    fd_output = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_output == -1) {
        fprintf(stderr, "Error: Cannot create output file '%s'\n", output_filename);
        close(fd_input);
        return -1;
    }

    
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    char processed_buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    
    while ((bytesRead = read(fd_input, buffer, BUFFER_SIZE)) > 0 && replace_count < max_replacements) {
        
        ssize_t processed_size = 0;

        for (ssize_t i = 0; i < bytesRead; i++) {
            if (buffer[i] == ' ' && replace_count < max_replacements) {
                
                replace_count++;
            } else {
                
                processed_buffer[processed_size++] = buffer[i];
            }
        }

        
        if (processed_size > 0) {
            if (write(fd_output, processed_buffer, processed_size) == -1) {
                fprintf(stderr, "Error: Write to output file failed\n");
                close(fd_input);
                close(fd_output);
                return -1;
            }
        }
    }

    if (bytesRead == -1) {
        fprintf(stderr, "Error: Reading from input file failed\n");
        close(fd_input);
        close(fd_output);
        return -1;
    }

    close(fd_input);
    close(fd_output);

    printf("Successfully processed file. Spaces removed: %d out of %d requested\n", 
           replace_count, max_replacements);
    printf("Input file: %s\n", input_filename);
    printf("Output file: %s\n", output_filename);

    return replace_count;
}
