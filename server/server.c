#include <errno.h>       // errno variable
#include <fcntl.h>       // fcntl functions
#include <netinet/ip.h>  // constants and structures for Internet Protocol family, IPv4 addresses, sockaddr_in struct
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

void connection_handler(int connectionFD);  // Function to handle communication with the client

int main() {
    int socketFD, connectionFD;
    struct sockaddr_in serverAddress, clientAddress;

    printf("Starting server...\n");

    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1) {
        perror("Error while creating server socket!");
        return 1;
    }

    printf("Server socket created successfully.\n");

    serverAddress.sin_family = AF_INET;                 // for IPv4
    serverAddress.sin_port = htons(8080);               // Server listens to port 8080
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);  // Binds the socket to all interfaces

    if (bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Socket Binding Error!!!");
        close(socketFD);
        return -1;
    }

    printf("Socket successfully bound to port 8080.\n");

    if (listen(socketFD, 50) < 0) {  // Max 50 clients in queue (backlog)
        perror("listen error !!!");
        close(socketFD);
        return -1;
    }

    printf("Server is now listening for incoming connections...\n");

    int clientSize;
    while (1) {
        clientSize = sizeof(clientAddress);
        connectionFD = accept(socketFD, (struct sockaddr *)&clientAddress, &clientSize);
        if (connectionFD == -1) {
            perror("Error in Accepting client connection!");
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork process");
            close(connectionFD);
        } else if (pid == 0) {  // Child process
            close(socketFD);    // Child does not need the listening socket
            connection_handler(connectionFD);
            exit(0);
        } else {                  // Parent process
            close(connectionFD);  // Parent does not need the connection socket
        }
    }

    close(socketFD);  // Close the listening socket
    return 0;
}

void connection_handler(int connectionFD) {
    printf("Client has connected to the server!\n");

    char readBuffer[10000];
    ssize_t readBytes;
    int userChoice;

    // Send initial prompt to the user when they connect
    ssize_t writeBytes = write(connectionFD, INITIAL_PROMPT, strlen(INITIAL_PROMPT));
    if (writeBytes == -1) {
        perror("Error while sending first prompt to the user!");
    } else {
        fd_set readfds;
        struct timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(connectionFD, &readfds);

        int selectResult = select(connectionFD + 1, &readfds, NULL, NULL, &timeout);
        if (selectResult == -1) {
            perror("select() error");
        } else if (selectResult == 0) {
            write(connectionFD, SESSION_TIMEOUT, strlen(SESSION_TIMEOUT));
            close(connectionFD);
            return;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connectionFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1)
            perror("Error while reading from client");
        else if (readBytes == 0)
            printf("No data was sent by the client");
        else {
            userChoice = atoi(readBuffer);
            switch (userChoice) {
                case 1:
                    admin_operation_handler(connectionFD);
                    break;
                case 2:
                    customer_operation_handler(connectionFD);
                    break;
                case 3:
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
    close(connectionFD);
}
