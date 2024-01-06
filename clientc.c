#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <signal.h>

#define CONNECT_NAME "/connect"
#define SHM_SIZE 1024

int running = 1;
char *key = NULL;
void sigint_handler(int signum)
{
    printf("Client Deregistered.\n");
    running = 0;
}
int lockshmid;
pthread_mutex_t *mutex;
pthread_mutex_t *commMutex;

void createlock(key_t kkey, pthread_mutex_t *mtx)
{
    key_t lockKey = kkey;
    lockshmid = shmget(lockKey, sizeof(pthread_mutex_t), IPC_CREAT | 0666);
    if (lockshmid < 0)
    {
        perror("shmget");
        exit(1);
    }

    *mtx = *(pthread_mutex_t *)shmat(lockshmid, NULL, 0);
    if (mtx == (pthread_mutex_t *)-1)
    {
        perror("shmat");
        exit(1);
    }
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mtx, &attr);
}
int comm_shm_fd;
void *comm_shm_ptr = NULL;
void newSHMusingKey()
{
    comm_shm_fd = shm_open(key, O_RDWR, 0666);
    if (comm_shm_fd == -1)
    {
        printf("ERROR! CLIENT WITH SAME NAME EXISTS\n");
        perror("shm_open");
        exit(1);
    }

    comm_shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, comm_shm_fd, 0);
    if (comm_shm_ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    // int numct = atoi(key + 8);
    key_t clientKey = 1244;
    // key_t clientKey = 1244 + numct;
    createlock(clientKey, commMutex);
    // Initialize the read-write lock
    pthread_mutex_lock(commMutex);
    memset(comm_shm_ptr, 0, SHM_SIZE);
    memcpy(comm_shm_ptr, "Hello", strlen("Hello"));
    pthread_mutex_unlock(commMutex);
}

int main(int argc, char *argv[])
{
    commMutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    signal(SIGINT, sigint_handler);
    printf("my name is %s\n", argv[1]);
    key_t startKey = 1233;
    createlock(startKey, mutex);
    int shm_fd = shm_open(CONNECT_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        printf("Server is not running\n");
        perror("shm_open");
        exit(1);
    }

    void *shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    // Initialize the read-write lock
    pthread_mutex_lock(mutex);
    memset(shm_ptr, 0, SHM_SIZE);
    memcpy(shm_ptr, argv[1], strlen(argv[1]));
    sleep(2);
    pthread_mutex_unlock(mutex);
    sleep(1);
    pthread_mutex_lock(mutex);
    key = (char *)shm_ptr;
    printf("Client Registration Key is: %s\n", (char *)shm_ptr);
    pthread_mutex_unlock(mutex);
    newSHMusingKey();

    while (running)
    {
        if (running == 0)
        {
            break;
        }
        // Print menu options
        printf("Menu:\n");
        printf("1. Arithmetic\n");
        printf("2. Even or Odd\n");
        printf("3. Is Prime\n");
        printf("4. Is Negative\n");
        printf("5. Disconnect\n");
        printf("6. Unregisterzz\n");
        printf("Choose an option: ");

        // Read user input
        int option;
        scanf("%d", &option);
        // Perform the selected action
        if (option == 1)
        {
            printf("Enter the first number: ");
            int num1;
            scanf("%d", &num1);
            printf("Enter the second number: ");
            int num2;
            scanf("%d", &num2);
            printf("Enter the operation: 1. + ,2. -, 3. *, 4. / \n");
            int op;
            scanf("%d", &op);
            pthread_mutex_lock(commMutex);
            memset(comm_shm_ptr, 0, SHM_SIZE);
            sprintf(comm_shm_ptr, "%d %d %d %d", option, num1, num2, op);
            pthread_mutex_unlock(commMutex);
            // read and print data in shm
            sleep(3);
            printf("Result %s\n", (char *)comm_shm_ptr);
        }
        else if (option == 2)
        {
            printf("Enter the number: ");
            int num1;
            scanf("%d", &num1);
            pthread_mutex_lock(commMutex);
            memset(comm_shm_ptr, 0, SHM_SIZE);
            sprintf(comm_shm_ptr, "%d %d", option, num1);
            pthread_mutex_unlock(commMutex);
            sleep(3);
            printf("0 for odd, 1 for even %s\n", (char *)comm_shm_ptr);
        }
        else if (option == 3)
        {
            printf("Enter the number: ");
            int num1;
            scanf("%d", &num1);
            pthread_mutex_lock(commMutex);
            memset(comm_shm_ptr, 0, SHM_SIZE);
            sprintf(comm_shm_ptr, "%d %d", option, num1);
            pthread_mutex_unlock(commMutex);
            sleep(3);
            printf("0 for non prime, 1 for prime %s\n", (char *)comm_shm_ptr);
        }
        else if (option == 4)
        {
            printf("Enter the number: ");
            int num1;
            scanf("%d", &num1);
            pthread_mutex_lock(commMutex);
            memset(comm_shm_ptr, 0, SHM_SIZE);
            sprintf(comm_shm_ptr, "%d %d", option, num1);
            pthread_mutex_unlock(commMutex);
            sleep(3);
            printf("1 for negative, 0 for positive %s\n", (char *)comm_shm_ptr);
        }
        else if (option == 5)
        {
            printf("Your registration is active, press 1 to continue:\n");
            int runner;
            scanf("%d", &runner);
            if (runner == 1)
            {
                continue;
            }
            else
            {
                printf("Invalid option.\n");
            }
        }
        else if (option == 6)
        {
            printf("Unregistering...\n");
            memset(comm_shm_ptr, 0, SHM_SIZE);
            memcpy(comm_shm_ptr, "exit", strlen("exit"));
            printf("Client Deregistered.\n");
            running = 0;
        }
        else
        {
            printf("Invalid option.\n");
        }
    }

    if (munmap(shm_ptr, SHM_SIZE) == -1)
    {
        perror("munmap");
        exit(1);
    }

    if (close(shm_fd) == -1)
    {
        perror("close");
        exit(1);
    }
    shmdt(mutex);
    shmctl(shm_fd, IPC_RMID, NULL);
    shmdt(commMutex);
    shmctl(lockshmid, IPC_RMID, NULL);
    return 0;
}
