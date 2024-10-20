#include <fcntl.h>  // For open()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // For strcpy()
#include <unistd.h>  // For write() and close()

#ifndef CUSTOMER_RECORD
#define CUSTOMER_RECORD

struct Customer {
    int id;
    char name[30];
    long int phone;
    int account;     // Account number
    char login[50];  // Login id
    char password[30];
};

#endif

void insertHardcodedCustomer() {
    struct Customer customer;
    int file;

    // Hardcode values for the customer
    customer.id = 0;
    strcpy(customer.name, "shardul");
    customer.phone = 6666699999;
    customer.account = 1001;
    strcpy(customer.login, "shardul_0");
    strcpy(customer.password, "1234");

    // Open the file using the open() system call in write-only and append mode
    file = open("customer_db", O_WRONLY, 0644);
    if (file < 0) {
        perror("Error opening file");
        exit(1);
    }

    // Write the customer record to the file
    ssize_t bytes_written = write(file, &customer, sizeof(struct Customer));
    if (bytes_written < 0) {
        perror("Error writing to file");
    } else {
        printf("Customer record added successfully!\n");
    }

    // Close the file using close() system call
    close(file);
}

int main() {
    insertHardcodedCustomer();
    return 0;
}
