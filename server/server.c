#include <errno.h>       // errno variable
#include <fcntl.h>       // fcntl functions
#include <netinet/ip.h>  // constants and structures for Internet Protocol family, IPv4 addresses, sockaddr_in struct
#include <pthread.h>     // `pthread` functions
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>   // printf,perror functions
#include <stdlib.h>  // atoi function
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>  // socket, bind, listen, accept functions
#include <sys/types.h>   // socket, bind, listen, accept functions
#include <unistd.h>      // read, write, close functions

#include "../functions/admin.h"
#include "../functions/customer.h"
#include "../functions/employee.h"
#include "../include/constants.h"

void *connection_handler(void *connFD_ptr);  // Thread function to handle communication with the client

int main() {
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
    int connectionFD = *(int *)connFD_ptr;
    printf("Client has connected to the server!\n");

    char readBuffer[10000];
    ssize_t readBytes;
    int userChoice;

    // Send initial prompt to the user when it connects
    ssize_t writeBytes = write(connectionFD, INITIAL_PROMPT, strlen(INITIAL_PROMPT));

    if (writeBytes == -1)
        perror("Error while sending first prompt to the user!");
    else {
        fd_set readfds;
        struct timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 0;

        FD_ZERO(&readfds);               // Clear the set of monitored file descriptors
        FD_SET(connectionFD, &readfds);  // Add the client socket to the set

        int selectResult = select(connectionFD + 1, &readfds, NULL, NULL, &timeout);
        // using select to monitor activity on socket and timeout if no activity is detected
        // for scenario where client is direclty closing the app without logging out
        if (selectResult == -1) {
            // Error occurred during select()
            perror("select() error");

        } else if (selectResult == 0) {
            // Timeout occurred, no data received
            write(connectionFD, SESSION_TIMEOUT, strlen(SESSION_TIMEOUT));
            close(connectionFD);
            return NULL;
        }

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
    printf("Terminating connection to client!\n");
    close(connectionFD);  // Close the connection
    return NULL;          // Exit the thread
}
