#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

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

    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed\n");
        return -1;
    }

    printf("Connected to server\n");

    for (int i = 0; i < num_files; i++) {
        task_t task;
        strncpy(task.filename, files[i], sizeof(task.filename) - 1);
        task.filename[sizeof(task.filename) - 1] = '\0';
        task.max_changes = max_changes;

        if (send(sock, &task, sizeof(task), 0) < 0) {
            perror("send failed");
            close(sock);
            return EXIT_FAILURE;
        }

        printf("Sent task: %s with max changes: %d\n", task.filename, task.max_changes);

        int result;
        if (recv(sock, &result, sizeof(result), 0) <= 0) {
            perror("recv failed");
            close(sock);
            return EXIT_FAILURE;
        }

        printf("File: %s - Replacements: %d\n", files[i], result);
        total_replacements += result;

        if (i < num_files - 1) {
            usleep(100000); 
        }
    }

    close(sock);
    printf("Total replacements: %d\n", total_replacements);

    return EXIT_SUCCESS;
}
