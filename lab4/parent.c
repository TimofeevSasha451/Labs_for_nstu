#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define FIFO_NAME "/tmp/file_processor_fifo"

typedef struct {
    char filename[256];
    int max_changes;
} task_t;

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

    if (mkfifo(FIFO_NAME, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_files; i++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            execl("./main_d", "main_d", NULL);
            
            perror("execl failed");
            exit(EXIT_FAILURE);
        }
    }
    sleep(1);
    int fifo_fd = open(FIFO_NAME, O_WRONLY);
    if (fifo_fd == -1) {
        perror("open FIFO for writing failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_files; i++) {
        task_t task;
        strncpy(task.filename, files[i], sizeof(task.filename) - 1);
        task.filename[sizeof(task.filename) - 1] = '\0';
        task.max_changes = max_changes;

        if (write(fifo_fd, &task, sizeof(task)) != sizeof(task)) {
            perror("write to FIFO failed");
            close(fifo_fd);
            exit(EXIT_FAILURE);
        }
        printf("Sent task: %s with max changes: %d\n", task.filename, task.max_changes);
    }

    close(fifo_fd);

    for (int i = 0; i < num_files; i++) {
        int status;
        pid_t pid = wait(&status);
        
        if (WIFEXITED(status)) {
            int replacements = WEXITSTATUS(status);
            if (replacements >= 0) {
                printf("File: %s - Replacements: %d\n", files[i], replacements);
                total_replacements += replacements;
            } else {
                printf("File: %s - Processing error\n", files[i]);
            }
        } else {
            printf("File: %s - Child process terminated abnormally\n", files[i]);
        }
    }

    unlink(FIFO_NAME);

    printf("Total replacements: %d\n", total_replacements);
    return EXIT_SUCCESS;
}
