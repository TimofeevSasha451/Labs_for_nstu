#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>

#define QUEUE_NAME "/file_processor_queue"

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

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;  
    attr.mq_msgsize = sizeof(task_t);  
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_files; i++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            perror("fork failed");
            mq_close(mq);
            mq_unlink(QUEUE_NAME);
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            execl("./main_d", "main_d", NULL);
            
            perror("execl failed");
            exit(EXIT_FAILURE);
        }
    }

    sleep(1);

    for (int i = 0; i < num_files; i++) {
        task_t task;
        strncpy(task.filename, files[i], sizeof(task.filename) - 1);
        task.filename[sizeof(task.filename) - 1] = '\0';
        task.max_changes = max_changes;

        if (mq_send(mq, (const char*)&task, sizeof(task), 0) == -1) {
            perror("mq_send failed");
        } else {
            printf("Sent task: %s with max changes: %d\n", task.filename, task.max_changes);
        }
    }

    mq_close(mq);

    for (int i = 0; i < num_files; i++) {
        int status;
        pid_t pid = wait(&status);
        
        if (WIFEXITED(status)) {
            int replacements = WEXITSTATUS(status);
            if (replacements >= 0 && replacements != 255) {
                printf("File: %s - Replacements: %d\n", files[i], replacements);
                total_replacements += replacements;
            } else {
                printf("File: %s - Processing error (code: %d)\n", files[i], replacements);
            }
        } else {
            printf("File: %s - Child process terminated abnormally\n", files[i]);
        }
    }

    mq_unlink(QUEUE_NAME);

    printf("Total replacements: %d\n", total_replacements);
    return EXIT_SUCCESS;
}
