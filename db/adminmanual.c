#include <fcntl.h>  // For open()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // For strcpy()
#include <unistd.h>  // For write() and close()

struct Admin {
    int id;
    char name[30];
    // Login Credentials
    char login[50];  // name_id will be loginid
    char password[30];
};

void insertHardcodedRecord() {
    struct Admin admin;
    int file;

    // Hardcode values
    admin.id = 0;
    strcpy(admin.name, "Saurabh");
    strcpy(admin.login, "Saurabh_0");
    strcpy(admin.password, "12345");

    // Open the file using the open() system call in write-only and append mode
    file = open("admin_db", O_WRONLY);
    if (file < 0) {
        perror("Error opening file");
        exit(1);
    }

    // Write the admin record to the file
    ssize_t bytes_written = write(file, &admin, sizeof(struct Admin));
    if (bytes_written < 0) {
        perror("Error writing to file");
    } else {
        printf("Record added successfully!\n");
    }

    // Close the file using close() system call
    close(file);
}

int main() {
    insertHardcodedRecord();
    return 0;
}
