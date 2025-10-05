#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dlfcn.h>
#include <errno.h>

#define PORT 8080
#define MAX_BUFFER 1024

typedef struct {
    char filename[256];
    int max_changes;
} task_t;

typedef int (*file_processor_t)(const char* input_path, int max_modifications);

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    void *library_handle = dlopen("./libdll.so", RTLD_LAZY);
    if (!library_handle) {
        fprintf(stderr, "Library loading error: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    file_processor_t process_file = (file_processor_t)dlsym(library_handle, "execute_file_processing");
    if (!process_file) {
        fprintf(stderr, "Symbol resolution error: %s\n", dlerror());
        dlclose(library_handle);
        return EXIT_FAILURE;
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        dlclose(library_handle);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        dlclose(library_handle);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        dlclose(library_handle);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        dlclose(library_handle);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }

        printf("Client connected\n");

        task_t task;
        int bytes_received = recv(new_socket, &task, sizeof(task), 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected\n");
            } else {
                perror("recv failed");
            }
            close(new_socket);
            continue;
        }

        printf("Received task: %s with max changes: %d\n", task.filename, task.max_changes);

        int result = process_file(task.filename, task.max_changes);

        send(new_socket, &result, sizeof(result), 0);
        printf("Sent result: %d\n", result);

        close(new_socket);
    }

    close(server_fd);
    dlclose(library_handle);
    return 0;
}
