#include <errno.h>       // errno variable
#include <fcntl.h>       // fcntl functions
#include <netinet/ip.h>  // constants and structures for Internet Protocol family, IPv4 addresses, sockaddr_in struct
#include <pthread.h>     // `pthread` functions
#include <sched.h>
#include <signal.h>  // For signal handling (SIGINT)
#include <stdbool.h>
#include <stdio.h>   // printf,perror functions
#include <stdlib.h>  // atoi function
#include <string.h>
#include <sys/ipc.h>  // For System V IPC
#include <sys/select.h>
#include <sys/shm.h>     // For System V shared memory
#include <sys/socket.h>  // socket, bind, listen, accept functions
#include <sys/types.h>   // socket, bind, listen, accept functions
#include <unistd.h>      // read, write, close functions

#include "../functions/admin.h"
#include "../functions/customer.h"
#include "../functions/employee.h"
#include "../include/constants.h"

// Shared memory key and size
#define SHM_KEY 1234
#define SHM_SIZE sizeof(SharedData)

// Shared structure in shared memory
typedef struct {
    long int active_clients;
} SharedData;

SharedData *shared_data;  // pointer to shared memory segment
long int client_count = 0;

void *connection_handler(void *connFD_ptr);  // Thread function to handle communication with the client

SharedData *shared_data;  // Pointer to shared memory segment
int shm_id;               // Shared memory identifier
int socketFD;             // Socket file descriptor

void handle_sigint(int sig);

int main() {
    printf("starting server.....\n");
    // Register signal handler for SIGINT
    signal(SIGINT, handle_sigint);

    // ======= Adding shared memory segment for monitoring active user=======
    // Create System V shared memory segment
    // Create System V shared memory segment
    shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attach to the shared memory segment
    shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // Initialize shared memory values
    shared_data->active_clients = 0;

    // ====server code starts from here =====
    int socketFD, connectionFD;
    struct sockaddr_in serverAddress, clientAddress;

    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1) {
        perror("Error while creating server socket!");
        return 1;
    }

    serverAddress.sin_family = AF_INET;                 // for IPv4
    serverAddress.sin_port = htons(8080);               // Server listen to port 8080, htons = host to network short
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);  // Binds the socket to all interfaces, htonl = host to network long
    // INADDR_ANY will mean it will also bind to loopback i.e 127.0.0.1.
    // therefor when we run client it wont require any specific ip address to connect to server.

    if (bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Socket Binding Error!!!");
        close(socketFD);
        return -1;
    }

    if (listen(socketFD, 50) < 0)  // max 50 people can be in queue(backlog)
    {
        perror("listen error !!!");
        close(socketFD);
        return -1;
    }

    printf("🟢 Server is listening on port 8080...\n");
    int clientSize;
    while (1) {
        clientSize = sizeof(clientAddress);
        connectionFD = accept(socketFD, (struct sockaddr *)&clientAddress, &clientSize);
        if (connectionFD == -1) {
            perror("Error in Accepting client connection!");
            continue;  // Continue to the next connection
        }
        pthread_t thread_id;
        // Creating new thread to handle the connections
        if (pthread_create(&thread_id, NULL, connection_handler, (void *)&connectionFD) != 0) {
            perror("Failed to create thread");
            close(connectionFD);
            continue;  // Continue to the next connection
        }

        // Detach the thread so that resources are released when it finishes
        pthread_detach(thread_id);
    }

    close(socketFD);  // Close the listening socket
    return 0;
}

void *connection_handler(void *connFD_ptr) {
    // Update active clients in shared memory
    shared_data->active_clients++;

    printf("+ A Client has connected to the server!\n");

    int connectionFD = *(int *)connFD_ptr;

    char readBuffer[10000];
    ssize_t readBytes;
    int userChoice;

    // Send initial prompt to the user when it connects
    ssize_t writeBytes = write(connectionFD, INITIAL_PROMPT, strlen(INITIAL_PROMPT));

    if (writeBytes == -1)
        perror("Error while sending first prompt to the user!");
    else {
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connectionFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
            perror("Error while reading from client");
        else if (readBytes == 0)
            printf("No data was sent by the client");
        else {
            userChoice = atoi(readBuffer);

            // Get the current thread ID
            pthread_t current_thread = pthread_self();
            struct sched_param param;
            int policy = SCHED_FIFO;

            switch (userChoice) {
                case 1:
                    // Admin
                    param.sched_priority = sched_get_priority_max(policy);
                    if (pthread_setschedparam(current_thread, policy, &param) != 0) {  // increasing priority for admin
                        perror("Failed to set thread priority");
                    }
                    admin_operation_handler(connectionFD);
                    break;
                case 2:
                    // Customer
                    customer_operation_handler(connectionFD);

                    break;
                case 3:
                    // employee
                    employee_operation_handler(connectionFD);
                    break;
                case 4:
                    printf("Exiting...\n");
                    break;
                default:
                    printf("Invalid choice.\n");
                    break;
            }
        }
    }
    // Decrease active clients when client disconnects
    shared_data->active_clients--;

    printf("- Terminating connection to client!\n");
    close(connectionFD);  // Close the connection
    return NULL;          // Exit the thread
}

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    printf("\nServer shutting down...\n");

    // Detach from the shared memory
    if (shmdt(shared_data) == -1) {
        perror("shmdt failed");
    }

    // Mark the shared memory segment for deletion
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
    }

    // Close the server socket
    if (close(socketFD) == -1) {
        perror("Failed to close socket");
    }

    exit(0);  // Exit the program cleanly
}