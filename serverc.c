#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/syscall.h>

#define SHM_NAME "/connect"
#define SHARED_MEM_SIZE 102
#define MAX_CL_NAME 20
#define MAX_CLIENT 10
#define PROMPT "Open for client requests!"
#define COMM_CHANNEL_PREFIX "/comm_hello"

int count = 0;
typedef struct
{
    char clname[MAX_CL_NAME];
    char clKey[MAX_CL_NAME];
    int svc_cnt;
    int connected;
    // Add a pointer to the shared memory for the client
    void *shm_ptr;
} client_data;

client_data clientsArr[MAX_CLIENT];

typedef struct
{
    char comm_channel[MAX_CL_NAME];
    void *shm_ptr;
    int ct;
    pthread_t thread;
    int localcounter;
} worker_thread;

int running = 1; // Flag to indicate if server is running

// Signal handler function to handle SIGINT signal
void sigint_handler(int signum)
{
    printf("Server stopped.\n");
    running = 0;
    return;
}

// void print_info()
// {
//     pid_t pid = getpid();
//     pid_t tid = gettid();
//     char *source_file = __FILE__;
//     const char *function_name = __func__;
//     int line_number = __LINE__;

//     printf("PID: %d\n", pid);
//     printf("ThreadID: %d\n", tid);
//     printf("SourceFile: %s\n", source_file);
//     printf("Function: %s\n", function_name);
//     printf("Line Number: %d\n", line_number);
// }

void print_info()
{
    pid_t pid = getpid();
    pthread_t tid = pthread_self();
    printf("PID: %d, ThreadID: %ld, SourceFile: %s, Function: %s, Line: %d\n",
           pid, (long)tid, __FILE__, __func__, __LINE__);
}

int lockshmid;
pthread_mutex_t *mutex;
pthread_mutex_t *commMutex[MAX_CLIENT];

void createlock(key_t key, pthread_mutex_t *mtx)
{
    key_t lockKey = key;
    lockshmid = shmget(lockKey, sizeof(pthread_mutex_t), IPC_CREAT | 0666);
    if (lockshmid < 0)
    {
        perror("shmget");
        exit(1);
    }

    mutex = shmat(lockshmid, NULL, 0);
    if (mutex == (pthread_mutex_t *)-1)
    {
        perror("shmat");
        exit(1);
    }
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &attr);
}

double additionOP(int nums[])
{
    if (nums[3] == 1)
    {
        return nums[2] + nums[1];
    }
    else if (nums[3] == 2)
    {
        return nums[2] - nums[1];
    }
    else if (nums[3] == 3)
    {
        return nums[2] * nums[1];
    }
    else if (nums[3] == 4)
    {
        return (double)nums[2] / nums[1];
    }
    else
    {
        double result = 0;
        for (int i = 0; i < 4; i++)
        {
            result += nums[i];
        }
        return result;
    }
}

double EvenOrOdd(int nums[])
{
    if (nums[1] % 2 == 0)
    {
        return 1; // for even
    }
    else
    {
        return 0; // for odd
    }
}

double isPrime(int nums[])
{
    int i, m = 0, flag = 0;
    m = nums[1] / 2;
    for (i = 2; i <= m; i++)
    {
        if (nums[1] % i == 0)
        {
            flag = 1;
            break;
        }
    }
    if (flag == 0)
    {
        return 1; // for prime
    }
    else
    {
        return 0; // for not prime
    }
}

double isNegative(int nums[])
{
    if (nums[1] < 0)
    {
        return 1; // for negative
    }
    else
    {
        return 0; // for positive
    }
}

void *client_worker(void *data)
{
    worker_thread *wt = (worker_thread *)data;
    print_info();
    int nums[4]; // Declare nums outside the loop
    while (running)
    {
        signal(SIGINT, sigint_handler);
        // Read from the shared memory
        char *cl_shm_data = (char *)wt->shm_ptr;
        if (strlen(cl_shm_data) > 0)
        {
            // printf("Data read from shared memory: %s\n", cl_shm_data);
            char *token = strtok(cl_shm_data, " ");
            // take data from shared memory and put it in nums array
            double result = 0;
            int i = 0;
            while (token != NULL)
            {
                nums[i] = atoi(token);
                token = strtok(NULL, " ");
                i++;
                if (result != -1)
                {
                    wt->localcounter++;
                    // printf("service request number: %d \n", wt->localcounter);
                }
            }
            if (nums[0] == 1)
            {
                result = additionOP(nums);
                if (result != -1)
                {
                    wt->localcounter++;
                    // printf("service request number: %d \n", wt->localcounter);
                }
            }
            else if (nums[0] == 2)
            {
                result = EvenOrOdd(nums);
                if (result != -1)
                {
                    wt->localcounter++;
                    // printf("service request number: %d \n", wt->localcounter);
                }
            }
            else if (nums[0] == 3)
            {
                result = isPrime(nums);
                if (result != -1)
                {
                    wt->localcounter++;
                    // printf("service request number: %d \n", wt->localcounter);
                }
            }
            else if (nums[0] == 4)
            {
                result = isNegative(nums);
                {
                    wt->localcounter++;
                    // printf("service request number: %d \n", wt->localcounter);
                }
            }
            else if (strcmp(cl_shm_data, "exit") == 0)
            {
                // delete worker thread wt
                // delete clientsArr[wt->ct] from clientArr array
                // delete wt from workerArr array

                // pthread_cancel(pthread_self());
                // // Close the shared memory object and unlink the name
                // munmap(wt->shm_ptr, SHARED_MEM_SIZE);
                // shm_unlink(wt->shm_ptr);
                // shm_unlink(wt->comm_channel);
                // running = 0;
            }
            else
            {
                // printf("Invalid input\n");
            }
            // cl_shm_data equals to result in string
            sprintf(cl_shm_data, "%f", result);
            // printf("Data written to shared memory: %s\n", cl_shm_data);
            sleep(3);
            // Write to the shared memory
            // memset(cl_shm_data, 0, SHARED_MEM_SIZE);
            // sprintf(cl_shm_data, "Received: %s", cl_shm_data);
            // printf("Data written to shared memory: %s\n", cl_shm_data);
        }
    }
    printf("Worker thread for %s terminated.\n", wt->comm_channel + strlen(COMM_CHANNEL_PREFIX));
    print_info();
    // Close the shared memory object and unlink the name
    munmap(wt->shm_ptr, SHARED_MEM_SIZE);
    shm_unlink(wt->comm_channel);
    return NULL;
}

int client_data_init(client_data *clientsArr, int count, char *clientName)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(clientsArr[i].clname, clientName) == 0)
        {
            printf("Client already exists!\n");
            return 1;
        }
    }
    char clientname[MAX_CL_NAME];
    strcpy(clientname, clientName);
    strcpy(clientsArr[count].clname, clientname);
    clientsArr[count].svc_cnt = count;
    char ch = (char)clientsArr[count].svc_cnt;
    char clientNewKey[30] = "/keyfor_";
    sprintf(clientNewKey + 8, "%c", ch);
    printf("%s\n", clientNewKey);
    strcat(clientNewKey, clientname);
    strcpy(clientsArr[count].clKey, clientNewKey);

    int shm_client = shm_open(clientsArr[count].clKey, O_CREAT | O_RDWR, 0666);
    if (shm_client == -1)
    {
        perror("shm_open");
        exit(1);
    }
    ftruncate(shm_client, SHARED_MEM_SIZE);
    void *shm_client_ptr = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_client, 0);
    if (shm_client_ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    key_t clientKey = 1244 + count;
    commMutex[count] = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    createlock(clientKey, commMutex[count]);
    // Store the pointer to the shared memory for the client
    clientsArr[count].shm_ptr = shm_client_ptr;
    return 0;
}

int main()
{
    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    signal(SIGINT, sigint_handler); // Register SIGINT signal handler
    key_t startKey = 1233;
    createlock(startKey, mutex);
    // Create a shared memory object
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(1);
    }
    ftruncate(shm_fd, SHARED_MEM_SIZE);

    void *shm_ptr = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    // Initialize the read-write lock
    memcpy(shm_ptr, PROMPT, strlen(PROMPT));
    // Create a worker thread for each client
    worker_thread worker_threads[MAX_CLIENT];
    int num_threads = 0;
    // Server loop
    while (running)
    {
        char *data = (char *)shm_ptr;
        if (strlen(PROMPT) > strlen(data))
        {
            count++;
            if (client_data_init(clientsArr, count, data) == 1)
            {
                memcpy(shm_ptr, PROMPT, strlen(PROMPT));
                continue;
            }
            pthread_mutex_lock(mutex);
            memcpy(shm_ptr, clientsArr[count].clKey, strlen(clientsArr[count].clKey));
            pthread_mutex_unlock(mutex);
            printf("Registration done: %s\n", (char *)shm_ptr);
            // Start a new worker thread for the client
            worker_thread *wt = &worker_threads[num_threads];
            snprintf(wt->comm_channel, sizeof(wt->comm_channel), "%s%s", COMM_CHANNEL_PREFIX, clientsArr[count].clname);
            wt->shm_ptr = clientsArr[count].shm_ptr;
            wt->ct = clientsArr[count].svc_cnt;
            wt->localcounter = 0;
            if (pthread_create(&wt->thread, NULL, client_worker, wt) != 0)
            {
                perror("pthread_create");
                exit(1);
            }
            num_threads++;
            sleep(5);
        }
        else
        {
            printf("Server says: %s\n", (char *)shm_ptr);
        }
        pthread_mutex_lock(mutex);
        memset(shm_ptr, 0, SHARED_MEM_SIZE);
        memcpy(shm_ptr, PROMPT, strlen(PROMPT));
        pthread_mutex_unlock(mutex);
        sleep(2); // Wait for some time before next iteration
    }
    // Wait for all worker threads to terminate
    for (int i = 0; i < num_threads; i++)
    {
        worker_thread *wt = &worker_threads[i];
        pthread_join(wt->thread, NULL);

        // Close the shared memory object and unlink the name
        munmap(wt->shm_ptr, SHARED_MEM_SIZE);
        shm_unlink(wt->comm_channel);
    }
    munmap(shm_ptr, SHARED_MEM_SIZE);
    shm_unlink(SHM_NAME);
    if (munmap(shm_ptr, SHARED_MEM_SIZE) == -1)
    {
        perror("munmap");
        exit(1);
    }
    if (close(shm_fd) == -1)
    {
        perror("close");
        exit(1);
    }
    // Clean up
    shmdt(mutex);
    shmctl(lockshmid, IPC_RMID, NULL);

    return 0;
}
