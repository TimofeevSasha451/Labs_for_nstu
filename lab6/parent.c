#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>

#define SHM_NAME "/file_processor_shm"
#define SEM_NAME "/file_processor_sem"

typedef struct {
    int total_files;
    int current_index;
    int max_changes;
    char filenames[0]; 
} shared_data_t;

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

    size_t total_path_len = 0;
    for (int i = 0; i < num_files; i++) {
        total_path_len += strlen(files[i]) + 1; // +1 для нулевого байта
    }
    
    size_t shm_size = sizeof(shared_data_t) + total_path_len;

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate failed");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    shared_data_t *shared_data = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap failed");
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    shared_data->total_files = num_files;
    shared_data->current_index = 0;
    shared_data->max_changes = max_changes;

    char *ptr = shared_data->filenames;
    for (int i = 0; i < num_files; i++) {
        strcpy(ptr, files[i]);
        ptr += strlen(files[i]) + 1;
    }

    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        munmap(shared_data, shm_size);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_files; i++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            perror("fork failed");
            sem_close(sem);
            sem_unlink(SEM_NAME);
            munmap(shared_data, shm_size);
            shm_unlink(SHM_NAME);
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            execl("./main_d", "main_d", NULL);
            
            perror("execl failed");
            exit(EXIT_FAILURE);
        }
    }

    sleep(1);

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

    sem_close(sem);
    sem_unlink(SEM_NAME);
    munmap(shared_data, shm_size);
    shm_unlink(SHM_NAME);

    printf("Total replacements: %d\n", total_replacements);
    return EXIT_SUCCESS;
}
