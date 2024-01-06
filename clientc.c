#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

#define SHM_NAME "/connect"
#define SHM_SIZE 1024

int main(int argc, char *argv[]) {
    printf("my name is %s\n", argv[1]);
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
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

    // Read from the shared memory with read lock
    pthread_rwlock_rdlock(&rwlock);
    printf("Data read from shared memory: %s\n", (char *)shm_ptr);
    pthread_rwlock_unlock(&rwlock);

    if (munmap(shm_ptr, SHM_SIZE) == -1) {
        perror("munmap");
        exit(1);
    }

    if (close(shm_fd) == -1) {
        perror("close");
        exit(1);
    }

    return 0;
}
