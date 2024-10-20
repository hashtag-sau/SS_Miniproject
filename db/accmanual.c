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

    // Hardcode values for the account
    account.accountNumber = 1001;
    account.custid = 0;      // Linking to customer with ID 101
    account.active = true;   // Active account
    account.balance = 5000;  // Initial balance

    // Open the file using the open() system call in write-only and append mode
    file = open("account_db", O_WRONLY, 0644);
    if (file < 0) {
        perror("Error opening file");
        exit(1);
    }

    // Write the account record to the file
    ssize_t bytes_written = write(file, &account, sizeof(struct Account));
    if (bytes_written < 0) {
        perror("Error writing to file");
    } else {
        printf("Account record added successfully!\n");
    }

    // Close the file using close() system call
    close(file);
}

int main() {
    insertHardcodedAccount();
    return 0;
}
