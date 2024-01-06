#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>

#define SHM_NAME "/connect"
#define SHM_SIZE 1024

int running = 1; // Flag to indicate if server is running

// Signal handler function to handle SIGINT signal
void sigint_handler(int signum) {
    printf("Server stopped.\n");
    running = 0;
}

int main() {
    signal(SIGINT, sigint_handler); // Register SIGINT signal handler

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    void *shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Initialize the read-write lock
    pthread_rwlock_t rwlock;
    pthread_rwlock_init(&rwlock, NULL);

    // Server loop
    while (running) {
        // Check for any requests from clients
        // ...
        // Write to the shared memory with write lock
        pthread_rwlock_wrlock(&rwlock);
        const char *data = "Hello, world!";
        memcpy(shm_ptr, data, strlen(data));
        printf("Data written to shared memory: %s\n", (char *)shm_ptr);
        pthread_rwlock_unlock(&rwlock);

        sleep(1); // Wait for some time before next iteration
    }

    if (munmap(shm_ptr, SHM_SIZE) == -1) {
        perror("munmap");
        exit(1);
    }

    if (close(shm_fd) == -1) {
        perror("close");
        exit(1);
    }

    // Clean up
    pthread_rwlock_destroy(&rwlock);

    return 0;
}

