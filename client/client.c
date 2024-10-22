#include <errno.h>       // Import for errno variable
#include <fcntl.h>       // Import for fcntl functions
#include <netinet/ip.h>  // Import for sockaddr_in structure
#include <stdio.h>       // Import for printf & perror functions
#include <stdlib.h>
#include <string.h>      // Import for string functions
#include <sys/socket.h>  // Import for socket, bind, listen, connect functions
#include <sys/types.h>   // Import for socket, bind, listen, connect, fork, lseek functions
#include <unistd.h>      // Import for fork, fcntl, read, write, lseek, _exit functions
void connection_handler(int sockFD);

void main() {
    int socketFileDescriptor, connectStatus;
    struct sockaddr_in serverAddress;
    struct sockaddr server;

    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor == -1) {
        perror("Error while creating server socket!");
        _exit(0);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    connectStatus = connect(socketFileDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectStatus == -1) {
        perror("Error while connecting to server!");
        close(socketFileDescriptor);
        _exit(0);
    }

    connection_handler(socketFileDescriptor);

    close(socketFileDescriptor);
}
// Handles the read & write operations w the server
void connection_handler(int sockFD) {
    // creating buffers for reading and writing
    char readBuffer[10000], writeBuffer[10000];  // Buffers for reading from and writing to the server
    ssize_t readBytes, writeBytes;               // Number of bytes read from or written to the socket
    // ssize_t is standard return type for read() and write()

    char tempBuffer[10000];

    do {
        // emptying the  buffers to ensure it doesn't contains residual from previous iterations
        bzero(readBuffer, sizeof(readBuffer));
        bzero(tempBuffer, sizeof(tempBuffer));

        readBytes = read(sockFD, readBuffer, sizeof(readBuffer));
        // reading the socket pointed by sockFD and storing the data in readBuffer

        if (readBytes == -1)
            perror("Failed to read from client socket!");

        else if (readBytes == 0)  // read returns 0, indicates that end of the file (EOF) has been reached.
            printf("Server closed the connection! Terminating now.\n");
        // An end-of-file condition usually indicates that the other side of the socket has closed the connection.

        // now we using 3 custom indicators like ^, $, #
        else if (strchr(readBuffer, '^') != NULL) {
            // Skip read from client
            strncpy(tempBuffer, readBuffer, strlen(readBuffer) - 1);
            printf("%s\n", tempBuffer);
            writeBytes = write(sockFD, "^", strlen("^"));
            if (writeBytes == -1) {
                perror("Failed to write to client socket!");
                break;
            }
        }

        else if (strchr(readBuffer, '$') != NULL) {
            // Server sent an error message and is now closing its end of the connection
            strncpy(tempBuffer, readBuffer, strlen(readBuffer) - 2);
            printf("%s\n", tempBuffer);
            printf("Server is closing the connection now.\n");
            break;
        }

        else if (strchr(readBuffer, '~') != NULL) {
            // Clear the terminal screen when receiving '~'
            system("clear");
            writeBytes = write(sockFD, "^", strlen("^"));
            if (writeBytes == -1) {
                perror("Failed to write to client socket!");
                break;
            }
        }

        else {
            bzero(writeBuffer, sizeof(writeBuffer));  // Empty the write buffer

            if (strchr(readBuffer, '#') != NULL)
                strcpy(writeBuffer, getpass(readBuffer));
            else {
                printf("%s", readBuffer);
                scanf("%[^\n]%*c", writeBuffer);  // Take user input!
            }

            writeBytes = write(sockFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1) {
                perror("Error while writing to client socket!");
                printf("Closing the connection to the server now!\n");
                break;
            }
        }
    } while (readBytes > 0);

    close(sockFD);
}