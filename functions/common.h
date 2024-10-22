#ifndef COMMON_FUNCTIONS
#define COMMON_FUNCTIONS

#include <errno.h>      // Import for `errno`
#include <fcntl.h>      // Import for `open`
#include <stdbool.h>    // Import for `bool` data type
#include <stdio.h>      // Import for `printf` & `perror`
#include <stdlib.h>     // Import for `atoi`
#include <string.h>     // Import for string functions
#include <sys/stat.h>   // Import for `open`
#include <sys/types.h>  // Import for `open`, `lseek`
#include <unistd.h>     // Import for `read`, `write & `lseek`

#include "../include/constants.h"
#include "../schemas/account.h"
#include "../schemas/admin.h"
#include "../schemas/customer.h"
#include "../schemas/transaction.h"
#include "auxillaries.h"

// Function Prototypes =================================

// bool login_handler(bool isAdmin, int connFD, struct Customer *customerAccou);
bool get_account_details(int connFD, struct Account *customerAccount, int AccountNumber);
bool get_customer_details(int connFD, int customerID);
bool get_transaction_details(int connFD, int accountNumber);
off_t find_account(int accountNumber, int accountFileDescriptor);

// =====================================================

// Function Definition =================================

bool get_account_details(int connFD, struct Account *customerAccount, int AccountNumber) {
    ssize_t readBytes, writeBytes;               // Number of bytes read from / written to the socket
    char readBuffer[10000], writeBuffer[10000];  // A buffer for reading from / writing to the socket
    char tempBuffer[10000];

    int accountNumber;
    struct Account account;
    int accountFileDescriptor;

    accountNumber = AccountNumber;

    if (accountNumber < 0) {
        if (customerAccount == NULL) {
            writeBytes = write(connFD, GET_ACCOUNT_NUMBER, strlen(GET_ACCOUNT_NUMBER));
            if (writeBytes == -1) {
                perror("Error writing GET_ACCOUNT_NUMBER message to client!");
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading account number response from client!");
                return false;
            }

            accountNumber = atoi(readBuffer);
        } else
            accountNumber = customerAccount->accountNumber;
    }

    accountFileDescriptor = open(ACCOUNT_FILE, O_RDONLY);
    if (accountFileDescriptor == -1) {
        perror("Error while opening account file");
        return false;
    }

    off_t filesize = lseek(accountFileDescriptor, 0, SEEK_END);
    if (filesize == -1) {
        perror("Error determining file size");
        return false;
    }

    int offset = find_account(accountNumber, accountFileDescriptor);
    if (offset == -1 || offset >= filesize) {
        // Account record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        perror("Error seeking to account record in get_account_details!\n");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing ACCOUNT_ID_DOESNT_EXIT message to client!\n");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
        return false;
    }

    offset = lseek(accountFileDescriptor, offset, SEEK_SET);

    readBytes = read(accountFileDescriptor, &account, sizeof(struct Account));
    if (readBytes == -1) {
        perror("Error reading employee record from file!");
        return false;
    }
    if (readBytes == 0) {
        perror("record doesn't exist!");
        return false;
    }

    if (customerAccount != NULL) {
        *customerAccount = account;
        return true;
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Account Details - \n\tAccount Number : %d\n\tCustomer ID: %d\n\tAccount Status : %s", account.accountNumber, account.custid, (account.active) ? "Active" : "Deactived");
    if (account.active) {
        sprintf(tempBuffer, "\n\tAccount Balance:₹ %d", account.balance);
        strcat(writeBuffer, tempBuffer);
    }

    strcat(writeBuffer, "\n^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

    return true;
}

bool get_customer_details(int connFD, int customerID) {
    ssize_t readBytes, writeBytes;               // Number of bytes read from / written to the socket
    char readBuffer[10000], writeBuffer[10000];  // A buffer for reading from / writing to the socket
    char tempBuffer[10000];

    struct Customer customer;
    int customerFileDescriptor;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Account), 0};

    if (customerID == -1) {
        writeBytes = write(connFD, GET_CUSTOMER_ID, strlen(GET_CUSTOMER_ID));
        if (writeBytes == -1) {
            perror("Error while writing GET_CUSTOMER_ID message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error getting customer ID from client!");
            return false;
        }

        customerID = atoi(readBuffer);
    }

    customerFileDescriptor = open(CUSTOMER_FILE, O_RDONLY);
    if (customerFileDescriptor == -1) {
        // Customer File doesn't exist
        perror("Error while opening customer file");
        return false;
    }
    // Get the file size
    off_t fileSize = lseek(customerFileDescriptor, 0, SEEK_END);
    if (fileSize == -1) {
        perror("Error determining file size");
        return false;
    }

    int offset = lseek(customerFileDescriptor, customerID * sizeof(struct Customer), SEEK_SET);
    if (offset == -1 && errno == EINVAL || offset >= fileSize) {
        // Customer record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing CUSTOMER_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
        return false;
    } else if (offset == -1) {
        perror("Error while seeking to required customer record!");
        return false;
    }
    lock.l_start = offset;

    int lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1) {
        perror("Error while obtaining read lock on the Customer file!");
        return false;
    }

    readBytes = read(customerFileDescriptor, &customer, sizeof(struct Customer));
    if (readBytes == -1) {
        perror("Error reading customer record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLK, &lock);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Customer Details - \n\tID : %d\n\tName : %s\n\tPhone : %ld\n\tAccount Number : %d\n\tUSERID : %s", customer.id, customer.name, customer.phone, customer.account, customer.login);

    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing customer info to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
    return true;
}

bool get_transaction_details(int connFD, int accountNumber) {
    ssize_t readBytes, writeBytes;                                // Number of bytes read from / written to the socket
    char readBuffer[1000], writeBuffer[10000], tempBuffer[1000];  // A buffer for reading from / writing to the socket

    struct Account account;

    if (accountNumber == -1) {
        // Get the accountNumber
        writeBytes = write(connFD, GET_ACCOUNT_NUMBER, strlen(GET_ACCOUNT_NUMBER));
        if (writeBytes == -1) {
            perror("Error writing GET_ACCOUNT_NUMBER message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading account number response from client!");
            return false;
        }

        account.accountNumber = atoi(readBuffer);
    } else
        account.accountNumber = accountNumber;

    if (get_account_details(connFD, &account, accountNumber)) {
        int iter;

        struct Transaction transaction;
        struct tm transactionTime;

        bzero(writeBuffer, sizeof(readBuffer));

        int transactionFileDescriptor = open(TRANSACTION_FILE, O_RDONLY);
        if (transactionFileDescriptor == -1) {
            perror("Error while opening transaction file!");
            write(connFD, TRANSACTIONS_NOT_FOUND, strlen(TRANSACTIONS_NOT_FOUND));
            read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
            return false;
        }

        while (read(transactionFileDescriptor, &transaction, sizeof(struct Transaction)) > 0) {
            if (transaction.senderAccountNumber == account.accountNumber) {
                transactionTime = *localtime(&(transaction.transactionTimestamp));

                char receiverAccountStr[20];  // Buffer to hold the receiver account number as a string
                if (transaction.receiverAccountNumber == -1)
                    strcpy(receiverAccountStr, "Withdraw");
                else if (transaction.receiverAccountNumber == -2)
                    strcpy(receiverAccountStr, "Deposit");
                else
                    snprintf(receiverAccountStr, sizeof(receiverAccountStr), "Receiver Account: %d", transaction.receiverAccountNumber);

                bzero(tempBuffer, sizeof(tempBuffer));
                sprintf(tempBuffer, "Details of transaction - \n\t Date : %d:%d %d/%d/%d \n\t Sender Account Number : %d \n\t %s \n\t Amount : %d \n",
                        transactionTime.tm_hour, transactionTime.tm_min, transactionTime.tm_mday,
                        (transactionTime.tm_mon + 1), (transactionTime.tm_year + 1900),
                        transaction.senderAccountNumber, receiverAccountStr, transaction.amount);

                if (strlen(writeBuffer) == 0)
                    strcpy(writeBuffer, tempBuffer);
                else
                    strcat(writeBuffer, tempBuffer);
            }
        }

        close(transactionFileDescriptor);

        if (strlen(writeBuffer) == 0) {
            write(connFD, TRANSACTIONS_NOT_FOUND, strlen(TRANSACTIONS_NOT_FOUND));
            read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
            return false;
        } else {
            strcat(writeBuffer, "^");
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
        }
    }
}

off_t find_account(int accountNumber, int accountFileDescriptor) {
    struct Account tempAccount;
    off_t offset = 0;

    // Set the read pointer at the start of the file
    if (lseek(accountFileDescriptor, 0, SEEK_SET) == -1) {
        perror("Error seeking to the start of the account file");
        return -1;
    }

    // Get the file size using lseek with SEEK_END
    off_t fileSize = lseek(accountFileDescriptor, 0, SEEK_END);
    if (fileSize == -1) {
        perror("Error determining file size");
        return -1;
    }

    // Set the read pointer back to the start of the file
    if (lseek(accountFileDescriptor, 0, SEEK_SET) == -1) {
        perror("Error seeking to the start of the account file");
        return -1;
    }

    // Iterate through all account records to find the matching account number
    while (offset < fileSize) {
        // Try to lock the record
        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Account), 0};
        int lockingStatus = fcntl(accountFileDescriptor, F_SETLK, &lock);
        if (lockingStatus == -1) {
            printf("debug at findaccoutn 1\n");
            // If the record is locked, skip to the next record
            offset += sizeof(struct Account);
            if (lseek(accountFileDescriptor, offset, SEEK_SET) == -1) {
                perror("Error seeking to next record");
                break;
            }

            continue;
        }

        ssize_t readBytes = read(accountFileDescriptor, &tempAccount, sizeof(struct Account));
        if (readBytes == -1) {
            perror("Error reading Account record");
            break;
        }
        if (readBytes == 0) {
            // Reached EOF
            perror("Reached EOF at find_account");
            break;
        }

        printf("debug at findaccount3\n");

        // Check if the account number matches
        printf("actual acc %d\n", accountNumber);
        printf("temp acc %d\n", tempAccount.accountNumber);
        if (tempAccount.accountNumber == accountNumber) {
            printf("%d kjhkjhui\n", tempAccount.accountNumber);
            lock.l_type = F_UNLCK;
            fcntl(accountFileDescriptor, F_SETLK, &lock);
            return offset;
        }

        // Unlock the record and move to the next one
        lock.l_type = F_UNLCK;
        fcntl(accountFileDescriptor, F_SETLK, &lock);
        offset += sizeof(struct Account);
    }

    return -1;
}
// =====================================================

#endif