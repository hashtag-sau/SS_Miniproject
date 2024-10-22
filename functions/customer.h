#ifndef CUSTOMER_FUNCTIONS
#define CUSTOMER_FUNCTIONS

#include <ctype.h>  //for isdigit
#include <fcntl.h>  // For file control options like O_CREAT, O_APPEND, O_RDWR
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>  // For file permissions like S_IRWXU
#include <unistd.h>    // For POSIX API like read, write, lseek

#include "../include/constants.h"
#include "../schemas/account.h"
#include "../schemas/customer.h"
#include "../schemas/transaction.h"
#include "common.h"

struct Customer loggedInCustomer;
int loginSemIdentifier;
int operationSemIdentifier;

// Function Prototypes =================================

bool customer_operation_handler(int connFD);
bool deposit(int connFD);
bool withdraw(int connFD);
bool get_balance(int connFD);
bool change_password(int connFD);
int write_transaction_to_file(int accountNumber, int receiverAccount, int amount);
bool send_money(int connFD, int AccountNumber);
bool login_handler_customer(int connFD, struct Customer *cust);
void initialize_semaphores(int accountNumber);
int lock_semaphore(int semIdentifier, int nowait);
void unlock_semaphore(int semIdentifier);

// =====================================================

// Function Definition =================================

bool login_handler_customer(int connFD, struct Customer *cust) {
    ssize_t readBytes, writeBytes;             // Number of bytes written to / read from the socket
    char readBuffer[1000], writeBuffer[1000];  // Buffer for reading from / writing to the client
    char tempBuffer[1000];
    struct Customer customer;

    int ID;

    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Get login message for respective user type

    strcpy(writeBuffer, CUSTOMER_LOGIN_WELCOME);

    // Append the request for LOGIN ID message
    strcat(writeBuffer, "\n");
    strcat(writeBuffer, LOGIN_ID);

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing WELCOME & LOGIN_ID message to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading login ID from client!");
        return false;
    }

    bool userFound = false;

    // Extracting ID from the login ID
    bzero(tempBuffer, sizeof(tempBuffer));
    strcpy(tempBuffer, readBuffer);
    strtok(tempBuffer, "_");
    char *idStr = strtok(NULL, "_");
    // strtok(NULL, "_") continues tokenizing the same string.
    //  By passing NULL asfirst argument, strtok knows to continue from where it left off in the previous call
    if (idStr == NULL || !isdigit(idStr[0])) {
        write(connFD, INVALID_LOGIN, strlen(INVALID_LOGIN));
        return false;
    }
    ID = atoi(idStr);

    int customerFileFD = open(CUSTOMER_FILE, O_RDONLY);
    if (customerFileFD == -1) {
        perror("Error opening customer_db in RDONLY mode!");
        return false;
    }

    off_t offset = lseek(customerFileFD, ID * sizeof(struct Customer), SEEK_SET);
    if (offset >= 0) {
        struct flock lock;
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = ID * sizeof(struct Customer);
        lock.l_len = sizeof(struct Customer);
        lock.l_pid = 0;  // by setting 0 we Let the kernel fill in the correct PID

        int lockingStatus = fcntl(customerFileFD, F_SETLKW, &lock);
        if (lockingStatus == -1) {
            perror("Failed to acquire read lock on customer_db record");
            return false;
        }

        readBytes = read(customerFileFD, &customer, sizeof(struct Customer));
        if (readBytes == -1) {
            perror("Error reading customer record from customer_db!");
        }

        lock.l_type = F_UNLCK;
        fcntl(customerFileFD, F_SETLK, &lock);

        if (strcmp(customer.login, readBuffer) == 0)
            userFound = true;

        close(customerFileFD);
    } else {
        writeBytes = write(connFD, CUSTOMER_LOGIN_ID_DOESNT_EXIT, strlen(CUSTOMER_LOGIN_ID_DOESNT_EXIT));
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    writeBytes = write(connFD, PASSWORD, strlen(PASSWORD));
    if (writeBytes == -1) {
        perror("Error writing PASSWORD message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == 1) {
        perror("Error reading password from the client!");
        return false;
    }

    char password[100];
    strcpy(password, readBuffer);

    if (userFound) {
        if (strcmp(password, customer.password) == 0) {  // password matching
            *cust = customer;
            return true;
        }

        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_PASSWORD, strlen(INVALID_PASSWORD));
    } else {
        bzero(writeBuffer, sizeof(writeBuffer));
        writeBytes = write(connFD, INVALID_LOGIN, strlen(INVALID_LOGIN));
    }

    return false;
}

bool customer_operation_handler(int connFD) {
    if (login_handler_customer(connFD, &loggedInCustomer)) {
        ssize_t writeBytes, readBytes;  // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[10000];

        initialize_semaphores(loggedInCustomer.account);
        int session = lock_semaphore(loginSemIdentifier, 1);
        if (session == -1) {
            bzero(writeBuffer, sizeof(writeBuffer));
            strcpy(writeBuffer, CUSTOMER_ALREADY_LOGGED_IN);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            return false;
        }
        // A buffer used for reading & writing to the client

        // union semun {
        //     int val;  // Value of the semaphore
        // } semSet;

        // int semctlStatus;
        // semIdentifier = semget(semKey, 1, 0);  // Get the semaphore if it exists
        // if (semIdentifier == -1) {
        //     // semaphore doesn't exist already creating new
        //     semIdentifier = semget(semKey, 1, IPC_CREAT | 0700);  // Create a new semaphore
        //     if (semIdentifier == -1) {
        //         perror("Error while creating semaphore!");
        //         _exit(1);
        //     }

        //     semSet.val = 1;  // Initialize the semaphore value to 1
        //     semctlStatus = semctl(semIdentifier, 0, SETVAL, semSet);
        //     if (semctlStatus == -1) {
        //         perror("Error while initializing a binary semaphore!");
        //         _exit(1);
        //     }
        // }

        // // Perform a semaphore operation to check if the user is already logged in
        // struct sembuf semOp;
        // semOp.sem_num = 0;
        // semOp.sem_op = -1;           // Decrement the semaphore value
        // semOp.sem_flg = IPC_NOWAIT;  // Non-blocking operation

        // if (semop(semIdentifier, &semOp, 1) == -1) {
        //     bzero(writeBuffer, sizeof(writeBuffer));
        //     strcpy(writeBuffer, CUSTOMER_ALREADY_LOGGED_IN);
        //     writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        //     return false;lock_sem
        // }

        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_LOGIN_SUCCESS);
        while (1) {
            clear_screen(connFD);  // Clear the screen
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, CUSTOMER_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1) {
                perror("Error while writing CUSTOMER_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error while reading client's choice for CUSTOMER_MENU");
                return false;
            }

            // printf("READ BUFFER : %s\n", readBuffer);
            int choice = atoi(readBuffer);
            // printf("CHOICE : %d\n", choice);
            switch (choice) {
                case 1:
                    get_customer_details(connFD, loggedInCustomer.id);
                    break;
                case 2:
                    deposit(connFD);
                    break;
                case 3:
                    withdraw(connFD);
                    break;
                case 4:
                    send_money(connFD, -1);
                    break;
                case 5:
                    get_balance(connFD);
                    break;
                case 6:
                    get_passbook(connFD, loggedInCustomer.account);
                    break;
                case 7:
                    change_password(connFD);
                    break;
                default:
                    unlock_semaphore(loginSemIdentifier);
                    writeBytes = write(connFD, CUSTOMER_LOGOUT, strlen(CUSTOMER_LOGOUT));
                    return false;
            }
            hold_screen(connFD);
        }
    } else {
        // CUSTOMER LOGIN FAILED
        return false;
    }
    return true;
}

bool deposit(int connFD) {
    char readBuffer[10000], writeBuffer[10000];
    ssize_t readBytes, writeBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;
    // printf("Account Number : %d\n", account.accountNumber);

    int depositAmount = 0;

    // Lock the critical section
    lock_semaphore(operationSemIdentifier, 0);

    if (get_account_details(connFD, &account, loggedInCustomer.account)) {
        if (account.active) {
            writeBytes = write(connFD, DEPOSIT_AMOUNT, strlen(DEPOSIT_AMOUNT));
            if (writeBytes == -1) {
                perror("Error writing DEPOSIT_AMOUNT to client!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading deposit money from client!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            depositAmount = atoi(readBuffer);
            if (depositAmount < 0) {
                write(connFD, DEPOSIT_AMOUNT_INVALID, strlen(DEPOSIT_AMOUNT_INVALID));
                read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
                unlock_semaphore(operationSemIdentifier);
                return false;
            }
            if (depositAmount == 0) {
                write(connFD, "nice bro depositing 0 rupee^", strlen("nice bro depositing 0 rupee^"));
                read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
                unlock_semaphore(operationSemIdentifier);
                return false;
            }
            if (depositAmount != 0) {
                printf("debug deposit amount : %d\n", depositAmount);
                int newTransactionID = write_transaction_to_file(account.accountNumber, -2, depositAmount);

                account.balance += depositAmount;

                int accountFileDescriptor = open(ACCOUNT_FILE, O_RDWR);
                if (accountFileDescriptor == -1) {
                    perror("Error opening account file in write mode!");
                    unlock_semaphore(operationSemIdentifier);
                    return false;
                }

                bool accountFound = false;

                off_t offset = find_account(account.accountNumber, accountFileDescriptor);

                if (offset < 0) {
                    perror("Account record not found or all records are locked!");
                    unlock_semaphore(operationSemIdentifier);
                    return false;
                }

                lseek(accountFileDescriptor, offset, SEEK_SET);

                struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Account), 0};
                int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
                if (lockingStatus == -1) {
                    perror("Error obtaining write lock on account file!");
                    unlock_semaphore(operationSemIdentifier);
                    return false;
                }

                writeBytes = write(accountFileDescriptor, &account, sizeof(struct Account));
                if (writeBytes == -1) {
                    perror("Error storing updated deposit money in account record!");
                    unlock_semaphore(operationSemIdentifier);
                    return false;
                }

                lock.l_type = F_UNLCK;
                fcntl(accountFileDescriptor, F_SETLK, &lock);

                write(connFD, DEPOSIT_AMOUNT_SUCCESS, strlen(DEPOSIT_AMOUNT_SUCCESS));
                read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

                get_balance(connFD);

                unlock_semaphore(operationSemIdentifier);

                return true;
            } else
                writeBytes = write(connFD, DEPOSIT_AMOUNT_INVALID, strlen(DEPOSIT_AMOUNT_INVALID));
        } else
            write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

        unlock_semaphore(operationSemIdentifier);
    } else {
        // FAIL
        unlock_semaphore(operationSemIdentifier);
        return false;
    }
    return false;
}

bool withdraw(int connFD) {
    char readBuffer[10000], writeBuffer[10000];
    ssize_t readBytes, writeBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;

    long int withdrawAmount = 0;

    // Lock the critical section
    lock_semaphore(operationSemIdentifier, 0);

    if (get_account_details(connFD, &account, loggedInCustomer.account)) {
        if (account.active) {
            writeBytes = write(connFD, WITHDRAW_AMOUNT, strlen(WITHDRAW_AMOUNT));
            if (writeBytes == -1) {
                perror("Error writing WITHDRAW_AMOUNT message to client!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading withdraw amount from client!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            withdrawAmount = atol(readBuffer);

            if (withdrawAmount != 0 && account.balance - withdrawAmount >= 0) {
                write_transaction_to_file(account.accountNumber, -1, withdrawAmount);

                account.balance -= withdrawAmount;

                int accountFileDescriptor = open(ACCOUNT_FILE, O_WRONLY);
                off_t offset = find_account(account.accountNumber, accountFileDescriptor);

                struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Account), getpid()};
                int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
                if (lockingStatus == -1) {
                    perror("Error obtaining write lock on account record!");
                    unlock_semaphore(operationSemIdentifier);
                    return false;
                }

                writeBytes = write(accountFileDescriptor, &account, sizeof(struct Account));
                if (writeBytes == -1) {
                    perror("Error writing updated balance into account file!");
                    unlock_semaphore(operationSemIdentifier);
                    return false;
                }

                lock.l_type = F_UNLCK;
                fcntl(accountFileDescriptor, F_SETLK, &lock);

                write(connFD, WITHDRAW_AMOUNT_SUCCESS, strlen(WITHDRAW_AMOUNT_SUCCESS));
                read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

                get_balance(connFD);

                unlock_semaphore(operationSemIdentifier);

                return true;
            } else
                writeBytes = write(connFD, WITHDRAW_AMOUNT_INVALID, strlen(WITHDRAW_AMOUNT_INVALID));
        } else
            write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

        unlock_semaphore(operationSemIdentifier);
    } else {
        // FAILURE while getting account information
        unlock_semaphore(operationSemIdentifier);
        return false;
    }
}

bool get_balance(int connFD) {
    char buffer[10000];
    struct Account account;
    account.accountNumber = loggedInCustomer.account;
    printf("debug at get_balance 1 \n");
    if (get_account_details(connFD, &account, loggedInCustomer.account)) {
        bzero(buffer, sizeof(buffer));
        printf("debug at get_balance : %d\n", account.balance);
        if (account.active) {
            sprintf(buffer, "You have ₹ %d imaginary money in our bank!^", account.balance);
            write(connFD, buffer, strlen(buffer));
        } else {
            write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        }
        read(connFD, buffer, sizeof(buffer));  // Dummy read
        return true;
    } else {
        printf("debug at get_balance 2 \n");
        return false;
    }
    return false;
}

bool send_money(int connFD, int AccountNumber) {
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    struct Account account;
    account.accountNumber = loggedInCustomer.account;

    int amount = 0;

    // Lock the critical section
    lock_semaphore(operationSemIdentifier, 0);

    if (get_account_details(connFD, &account, loggedInCustomer.account)) {
        if (account.active) {
            bzero(writeBuffer, sizeof(writeBuffer));
            writeBytes = write(connFD, SEND_ACCOUNT, strlen(SEND_ACCOUNT));
            if (writeBytes == -1) {
                perror("Error writing Send_Account message to client!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading send_account number from client!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            int receiverAccountNumber = atoi(readBuffer);

            if (receiverAccountNumber == account.accountNumber) {
                write(connFD, "You can't send money to yourself!^", strlen("You can't send money to yourself!^"));
                read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            int accountFileDescriptor = open(ACCOUNT_FILE, O_RDWR);

            off_t offset = find_account(receiverAccountNumber, accountFileDescriptor);
            if (offset == -1) {
                write(connFD, "Receiver account doesn't exist!^", strlen("Receiver account doesn't exist!^"));
                read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            struct Account receiverAccount;
            lseek(accountFileDescriptor, offset, SEEK_SET);
            read(accountFileDescriptor, &receiverAccount, sizeof(struct Account));

            if (receiverAccount.active == 0) {
                write(connFD, "Receiver account is deactivated!^", strlen("Receiver account is deactivated!^"));
                read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            bzero(writeBuffer, sizeof(writeBuffer));
            writeBytes = write(connFD, SEND_AMOUNT, strlen(SEND_AMOUNT));
            if (writeBytes == -1) {
                perror("Error writing Send_AMOUNT message to client!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading send amount from client!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            amount = atoi(readBuffer);

            if (amount != 0 && account.balance - amount >= 0) {
                account.balance -= amount;
                receiverAccount.balance += amount;

                // updating in sender account
                off_t offset = find_account(account.accountNumber, accountFileDescriptor);

                lseek(accountFileDescriptor, offset, SEEK_SET);

                struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Account), 0};
                int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
                if (lockingStatus == -1) {
                    perror("Error obtaining write lock on account record!");
                    unlock_semaphore(operationSemIdentifier);
                    return false;
                }

                writeBytes = write(accountFileDescriptor, &account, sizeof(struct Account));
                if (writeBytes == -1) {
                    perror("Error writing updated balance into account file!");
                    unlock_semaphore(operationSemIdentifier);
                    return false;
                }

                lock.l_type = F_UNLCK;
                fcntl(accountFileDescriptor, F_SETLK, &lock);

                // updating in receiver account
                offset = find_account(receiverAccount.accountNumber, accountFileDescriptor);

                lseek(accountFileDescriptor, offset, SEEK_SET);

                lock.l_type = F_WRLCK;
                lock.l_whence = SEEK_SET;
                lock.l_start = offset;
                lock.l_len = sizeof(struct Account);
                lock.l_pid = 0;
                lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
                if (lockingStatus == -1) {
                    perror("Error obtaining write lock on account record!");
                    unlock_semaphore(operationSemIdentifier);
                    return false;
                }

                writeBytes = write(accountFileDescriptor, &receiverAccount, sizeof(struct Account));
                if (writeBytes == -1) {
                    perror("Error writing updated balance into account file!");
                    unlock_semaphore(operationSemIdentifier);
                    return false;
                }

                lock.l_type = F_UNLCK;
                fcntl(accountFileDescriptor, F_SETLK, &lock);

                write_transaction_to_file(account.accountNumber, receiverAccount.accountNumber, amount);

                write(connFD, SEND_AMOUNT_SUCCESS, strlen(SEND_AMOUNT_SUCCESS));
                read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

                get_balance(connFD);

                unlock_semaphore(operationSemIdentifier);

                return true;
            } else
                writeBytes = write(connFD, WITHDRAW_AMOUNT_INVALID, strlen(WITHDRAW_AMOUNT_INVALID));
        } else
            write(connFD, ACCOUNT_DEACTIVATED, strlen(ACCOUNT_DEACTIVATED));
        read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

        unlock_semaphore(operationSemIdentifier);
    } else {
        // FAILURE while getting account information
        unlock_semaphore(operationSemIdentifier);
        return false;
    }
}

bool change_password(int connFD) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    char newPassword[1000];

    // Lock the critical section
    lock_semaphore(operationSemIdentifier, 0);

    writeBytes = write(connFD, PASSWORD_CHANGE_OLD_PASS, strlen(PASSWORD_CHANGE_OLD_PASS));
    if (writeBytes == -1) {
        perror("Error writing PASSWORD_CHANGE_OLD_PASS message to client!");
        unlock_semaphore(operationSemIdentifier);
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading old password response from client");
        unlock_semaphore(operationSemIdentifier);
        return false;
    }

    if (strcmp(readBuffer, loggedInCustomer.password) == 0) {
        // Password matches with old password
        writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS, strlen(PASSWORD_CHANGE_NEW_PASS));
        if (writeBytes == -1) {
            perror("Error writing PASSWORD_CHANGE_NEW_PASS message to client!");
            unlock_semaphore(operationSemIdentifier);
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading new password response from client");
            unlock_semaphore(operationSemIdentifier);
            return false;
        }

        strcpy(newPassword, readBuffer);

        writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS_RE, strlen(PASSWORD_CHANGE_NEW_PASS_RE));
        if (writeBytes == -1) {
            perror("Error writing PASSWORD_CHANGE_NEW_PASS_RE message to client!");
            unlock_semaphore(operationSemIdentifier);
            return false;
        }
        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading new password reenter response from client");
            unlock_semaphore(operationSemIdentifier);
            return false;
        }

        if (strcmp(readBuffer, newPassword) == 0) {
            // New & reentered passwords match

            strcpy(loggedInCustomer.password, newPassword);

            int customerFileDescriptor = open(CUSTOMER_FILE, O_WRONLY);
            if (customerFileDescriptor == -1) {
                perror("Error opening customer file!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            off_t offset = lseek(customerFileDescriptor, loggedInCustomer.id * sizeof(struct Customer), SEEK_SET);
            if (offset == -1) {
                perror("Error seeking to the customer record!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            struct flock lock = {F_WRLCK, SEEK_SET, offset, sizeof(struct Customer), getpid()};
            int lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
            if (lockingStatus == -1) {
                perror("Error obtaining write lock on customer record!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            writeBytes = write(customerFileDescriptor, &loggedInCustomer, sizeof(struct Customer));
            if (writeBytes == -1) {
                perror("Error storing updated customer password into customer record!");
                unlock_semaphore(operationSemIdentifier);
                return false;
            }

            lock.l_type = F_UNLCK;
            lockingStatus = fcntl(customerFileDescriptor, F_SETLK, &lock);

            close(customerFileDescriptor);

            writeBytes = write(connFD, PASSWORD_CHANGE_SUCCESS, strlen(PASSWORD_CHANGE_SUCCESS));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

            unlock_semaphore(operationSemIdentifier);

            return true;
        } else {
            // New & reentered passwords don't match
            writeBytes = write(connFD, PASSWORD_CHANGE_NEW_PASS_INVALID, strlen(PASSWORD_CHANGE_NEW_PASS_INVALID));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
        }
    } else {
        // Password doesn't match with old password
        writeBytes = write(connFD, PASSWORD_CHANGE_OLD_PASS_INVALID, strlen(PASSWORD_CHANGE_OLD_PASS_INVALID));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
    }

    unlock_semaphore(operationSemIdentifier);

    return false;
}

int write_transaction_to_file(int accountNumber, int receiverAccount, int amount) {
    struct Transaction newTransaction;
    newTransaction.senderAccountNumber = accountNumber;
    newTransaction.receiverAccountNumber = receiverAccount;
    newTransaction.amount = amount;
    newTransaction.transactionTimestamp = time(NULL);

    ssize_t readBytes, writeBytes;

    int transactionFileDescriptor = open(TRANSACTION_FILE, O_CREAT | O_APPEND | O_RDWR, S_IRWXU);
    if (transactionFileDescriptor == -1) {
        perror("Error opening transaction file!");
        return -1;
    }

    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_END;
    lock.l_start = 0;
    lock.l_len = 0;  // Lock the entire file
    lock.l_pid = 0;

    if (fcntl(transactionFileDescriptor, F_SETLKW, &lock) == -1) {
        perror("Error obtaining write lock on transaction file!");
        close(transactionFileDescriptor);
        return -1;
    }

    // Determine the next transaction ID
    off_t offset = lseek(transactionFileDescriptor, -sizeof(struct Transaction), SEEK_END);
    if (offset >= 0) {
        // There exists a last transaction record
        struct Transaction prevTransaction;
        readBytes = read(transactionFileDescriptor, &prevTransaction, sizeof(struct Transaction));
        if (readBytes == -1) {
            perror("Error reading last transaction record!");
            lock.l_type = F_UNLCK;
            fcntl(transactionFileDescriptor, F_SETLK, &lock);
            close(transactionFileDescriptor);
            return -1;
        }
        newTransaction.transactionID = prevTransaction.transactionID + 1;
    } else {
        // No transaction records exist therefore the new transaction ID is 0
        newTransaction.transactionID = 0;
    }

    writeBytes = write(transactionFileDescriptor, &newTransaction, sizeof(struct Transaction));
    if (writeBytes == -1) {
        perror("Error writing new transaction record!");
        lock.l_type = F_UNLCK;
        fcntl(transactionFileDescriptor, F_SETLK, &lock);
        close(transactionFileDescriptor);
        return -1;
    }

    lock.l_type = F_UNLCK;
    if (fcntl(transactionFileDescriptor, F_SETLK, &lock) == -1) {
        perror("Error releasing write lock on transaction file!");
        close(transactionFileDescriptor);
        return -1;
    }

    close(transactionFileDescriptor);
    return newTransaction.transactionID;
}

void initialize_semaphores(int accountNumber) {
    key_t loginSemKey = ftok(CUSTOMER_FILE, accountNumber);
    key_t operationSemKey = ftok(ACCOUNT_FILE, accountNumber);

    if (loginSemKey == -1 || operationSemKey == -1) {
        perror("Error generating semaphore keys");
        exit(1);
    }

    union semun {
        int val;
    } semSet;

    // Create login semaphore if it does not exist
    loginSemIdentifier = semget(loginSemKey, 1, IPC_CREAT | IPC_EXCL | 0700);
    if (loginSemIdentifier == -1) {
        if (errno == EEXIST) {
            // Semaphore already exists, get the existing semaphore
            loginSemIdentifier = semget(loginSemKey, 1, 0);
            if (loginSemIdentifier == -1) {
                perror("Error getting existing login semaphore");
                exit(1);
            }
        } else {
            perror("Error creating login semaphore");
            exit(1);
        }
    } else {
        semSet.val = 1;
        if (semctl(loginSemIdentifier, 0, SETVAL, semSet) == -1) {
            perror("Error initializing login semaphore");
            exit(1);
        }
    }

    // Create operation semaphore if it does not exist
    operationSemIdentifier = semget(operationSemKey, 1, IPC_CREAT | IPC_EXCL | 0700);
    if (operationSemIdentifier == -1) {
        if (errno == EEXIST) {
            // Semaphore already exists, get the existing semaphore
            operationSemIdentifier = semget(operationSemKey, 1, 0);
            if (operationSemIdentifier == -1) {
                perror("Error getting existing operation semaphore");
                exit(1);
            }
        } else {
            perror("Error creating operation semaphore");
            exit(1);
        }
    } else {
        // Initialize the operation semaphore
        semSet.val = 1;
        if (semctl(operationSemIdentifier, 0, SETVAL, semSet) == -1) {
            perror("Error initializing operation semaphore");
            exit(1);
        }
    }
}

int lock_semaphore(int semIdentifier, int nowait) {
    struct sembuf semOp;
    semOp.sem_num = 0;
    semOp.sem_op = -1;  // Decrement the semaphore value
    semOp.sem_flg = nowait ? IPC_NOWAIT : 0;

    if (semop(semIdentifier, &semOp, 1) == -1) {
        perror("Error locking semaphore");
        return -1;
    }
    return 1;
}

void unlock_semaphore(int semIdentifier) {
    struct sembuf semOp;
    semOp.sem_num = 0;
    semOp.sem_op = 1;   // Increment the semaphore value
    semOp.sem_flg = 0;  // Blocking operation
    printf("debug at unlock_semaphore\n");
    if (semop(semIdentifier, &semOp, 1) == -1) {
        perror("Error unlocking semaphore");
        exit(1);
    }
}

// =====================================================

#endif