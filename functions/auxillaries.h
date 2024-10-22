#ifndef AUXILLARIES_H
#define AUXILLARIES_H
#include <ctype.h>   // for isdigit
#include <stddef.h>  // for size_t
#include <stdlib.h>  // for rand
#include <string.h>  // for strlen

// Function to check if a string is purely a number
bool is_pure_number(const char *str) {
    for (size_t i = 0; i < strlen(str); i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

// Function to generate a random 5-character password
void generate_random_password(char *password, size_t length) {
    // const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const char charset[] = "1";
    for (size_t i = 0; i < length; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        password[i] = charset[key];
    }
    password[length] = '\0';  // Null-terminate the string
}

void clear_screen(int connFD) {
    char writeBuffer[1000];
    ssize_t writeBytes;

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s", "~");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error sending clear screen command to the client!");
    }
    read(connFD, writeBuffer, sizeof(writeBuffer));  // Dummy read
}

void hold_screen(int connFD) {
    char writeBuffer[1000];
    ssize_t writeBytes;

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s", "🕹️Press any key to continue... ▶️");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error sending clear screen command to the client!");
    }
    read(connFD, writeBuffer, sizeof(writeBuffer));  // Dummy read
}

#endif  // AUXILLARIES_H