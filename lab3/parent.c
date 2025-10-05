#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <file1> [file2 ...] <max_changes>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int max_changes = atoi(argv[argc - 1]);
    int num_files = argc - 2;
    char *files[num_files];
    int total_replacements = 0;

    for (int i = 0; i < num_files; i++) {
        files[i] = argv[i + 1];
    }

    for (int i = 0; i < num_files; i++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            char max_changes_str[20];
            snprintf(max_changes_str, sizeof(max_changes_str), "%d", max_changes);
            
            execl("./main_d", "main_d", files[i], max_changes_str, NULL);
            
            perror("execl failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_files; i++) {
        int status;
        pid_t pid = wait(&status);
        
        if (WIFEXITED(status)) {
            int replacements = WEXITSTATUS(status);
            if (replacements != 255) { 
                printf("File: %s - Replacements: %d\n", files[i], replacements);
                total_replacements += replacements;
            } else {
                printf("File: %s - Processing error\n", files[i]);
            }
        } else {
            printf("File: %s - Child process terminated abnormally\n", files[i]);
        }
    }

    printf("Total replacements: %d\n", total_replacements);
    return EXIT_SUCCESS;
}
