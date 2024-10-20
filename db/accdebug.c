#include <fcntl.h>    // For open()
#include <stdbool.h>  // For bool type
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // For strcpy()
#include <unistd.h>  // For write() and close()

struct Account {
    int accountNumber;  // Account number: 0, 1, 2, etc.
    int custid;         // Customer ID
    bool active;        // 1 -> Active, 0 -> Deactivated (Deleted)
    int balance;        // Account balance
};

void insertHardcodedAccount() {
    struct Account account;
    int file;

    // Open the file using the open() system call in write-only and append mode
    file = open("account_db", O_RDWR, 0644);
    if (file < 0) {
        perror("Error opening file");
        exit(1);
    }

    // read first record from file and print it
    ssize_t bytes_read = read(file, &account, sizeof(struct Account));
    if (bytes_read < 0) {
        perror("Error reading from file");
    } else {
        printf("Account record read successfully!\n");
        printf("Account Number: %d\n", account.accountNumber);
        printf("Customer ID: %d\n", account.custid);
        printf("Active: %d\n", account.active);
        printf("Balance: %d\n", account.balance);
    }

    // Close the file using close() system call
    close(file);
}

int main() {
    insertHardcodedAccount();
    return 0;
}
