#ifndef ADMIN_FUNCTIONS
#define ADMIN_FUNCTIONS

#include <ctype.h>  //for isdigit

#include "../include/constants.h"
#include "../schemas/account.h"
#include "../schemas/customer.h"
#include "../schemas/employee.h"
#include "../schemas/transaction.h"
#include "./common.h"

// Function Prototypes =================================

bool employee_operation_handler(int connFD);
bool add_account(int connFD);
int add_customer(int connFD);
bool delete_account(int connFD);
bool modify_customer_info(int connFD);
bool login_handler_employee(int connFD, struct Employee *emp);
bool is_valid_phone_number(const char *phone);
void generate_random_password(char *password, size_t length);
bool is_pure_number(const char *str);

// =====================================================

// Function Definition =================================

// =====================================================

bool login_handler_employee(int connFD, struct Employee *emp) {
    ssize_t readBytes, writeBytes;             // Number of bytes written to / read from the socket
    char readBuffer[1000], writeBuffer[1000];  // Buffer for reading from / writing to the client
    char tempBuffer[1000];
    struct Employee employee;

    int ID;

    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Get login message for respective user type

    strcpy(writeBuffer, EMPLOYEE_LOGIN_WELCOME);

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

    int EmployeeFD = open(EMPLOYEE_FILE, O_RDONLY);
    if (EmployeeFD == -1) {
        perror("Error opening customer_db in RDONLY mode!");
        return false;
    }

    off_t offset = lseek(EmployeeFD, ID * sizeof(struct Employee), SEEK_SET);
    if (offset >= 0) {
        struct flock lock;
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = ID * sizeof(struct Employee);
        lock.l_len = sizeof(struct Employee);
        lock.l_pid = 0;  // by setting 0 we Let the kernel fill in the correct PID

        int lockingStatus = fcntl(EmployeeFD, F_SETLKW, &lock);
        if (lockingStatus == -1) {
            perror("Failed to acquire read lock on customer_db record");
            return false;
        }

        readBytes = read(EmployeeFD, &employee, sizeof(struct Employee));
        if (readBytes == -1) {
            perror("Error reading employee record from customer_db!");
        }

        lock.l_type = F_UNLCK;
        fcntl(EmployeeFD, F_SETLK, &lock);

        if (strcmp(employee.login, readBuffer) == 0)
            userFound = true;

        close(EmployeeFD);
    } else {
        writeBytes = write(connFD, EMPLOYEE_LOGIN_ID_DOESNT_EXIT, strlen(EMPLOYEE_LOGIN_ID_DOESNT_EXIT));
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

    if (userFound) {
        char password[100];

        if (strcmp(password, employee.password) == 0) {  // password matching
            *emp = employee;
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

bool employee_operation_handler(int connFD) {
    struct Employee loggedInEmployee;
    struct Account account;
    struct Customer customer;

    int accontNumber;
    int customerID;

    if (login_handler_employee(connFD, &loggedInEmployee)) {
        ssize_t writeBytes, readBytes;             // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000];  // A buffer used for reading & writing to the client
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, EMPLOYEE_LOGIN_SUCCESS);
        while (1) {
            strcat(writeBuffer, "\n");
            strcat(writeBuffer, ADMIN_MENU);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1) {
                perror("Error while writing ADMIN_MENU to client!");
                return false;
            }
            bzero(writeBuffer, sizeof(writeBuffer));

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error while reading client's choice for ADMIN_MENU");
                return false;
            }

            int choice = atoi(readBuffer);
            switch (choice) {
                case 1:
                    get_customer_details(connFD, -1);
                    break;
                case 2:
                    get_account_details(connFD, &account, -1);
                    break;
                case 3:
                    get_transaction_details(connFD, -1);
                    break;
                case 4:
                    add_account(connFD);
                    break;
                case 5:
                    delete_account(connFD);
                    break;
                case 6:
                    modify_customer_info(connFD);
                    break;
                default:
                    writeBytes = write(connFD, EMPLOYEE_LOGOUT, strlen(EMPLOYEE_LOGOUT));
                    return false;
            }
        }
    } else {
        // ADMIN LOGIN FAILED
        return false;
    }
    return true;
}

bool add_account(int connFD) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Account newAccount, prevAccount;

    writeBytes = write(connFD, "Initiating Account Creation...\n^", strlen("Initiating Account Creation...\n^"));
    if (writeBytes == -1) {
        perror("Error writing Create account message to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, &readBuffer, sizeof(readBuffer));  // Dummy read
    if (readBytes == -1) {
        perror("Error reading response from client!");
        return false;
    }

    newAccount.custid = add_customer(connFD);

    newAccount.active = true;
    newAccount.balance = 0;

    int accountFileDescriptor = open(ACCOUNT_FILE, O_RDONLY);
    if (accountFileDescriptor == -1 && errno == ENOENT) {
        // Account file was never created
        newAccount.accountNumber = 0;
    } else if (accountFileDescriptor == -1) {
        perror("Error while opening account file");
        return false;
    } else {
        int offset = lseek(accountFileDescriptor, -sizeof(struct Account), SEEK_END);
        if (offset == -1) {
            perror("Error seeking to last Account record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Account), 0};
        int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1) {
            perror("Error obtaining read lock on Account record!");
            return false;
        }

        readBytes = read(accountFileDescriptor, &prevAccount, sizeof(struct Account));
        if (readBytes == -1) {
            perror("Error while reading Account record from file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(accountFileDescriptor, F_SETLK, &lock);

        close(accountFileDescriptor);

        newAccount.accountNumber = prevAccount.accountNumber + 1;
    }

    accountFileDescriptor = open(ACCOUNT_FILE, O_RDONLY);

    writeBytes = write(accountFileDescriptor, &newAccount, sizeof(struct Account));
    if (writeBytes == -1) {
        perror("Error while writing Account record to file!");
        return false;
    }

    close(accountFileDescriptor);

    int customerFileDescriptor = open(CUSTOMER_FILE, O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
    if (customerFileDescriptor == -1) {
        perror("Error while opening customer file");
        return -1;
    } else {
        struct Customer cust;
        int offset = lseek(customerFileDescriptor, newAccount.custid * sizeof(struct Customer), SEEK_SET);
        readBytes = read(customerFileDescriptor, &cust, sizeof(struct Customer));
        cust.account = newAccount.accountNumber;
        writeBytes = write(customerFileDescriptor, &cust, sizeof(struct Customer));
    }
    close(customerFileDescriptor);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s%d", NEW_ACCOUNT_NUMBER, newAccount.accountNumber);
    strcat(writeBuffer, "\nRedirecting you to the main menu ...^");
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    readBytes = read(connFD, readBuffer, sizeof(read));  // Dummy read
    return true;
}

int add_customer(int connFD) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Customer newCustomer, previousCustomer;

    int customerFileDescriptor = open(CUSTOMER_FILE, O_RDWR | O_CREAT | O_EXCL, S_IRWXU);
    if (customerFileDescriptor == -1) {
        perror("Error while opening customer file");
        return -1;
    } else {
        int offset = lseek(customerFileDescriptor, -sizeof(struct Customer), SEEK_END);
        if (offset == -1) {
            perror("Error seeking to last Customer record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Customer), getpid()};
        int lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1) {
            perror("Error obtaining read lock on Customer record!");
            return false;
        }

        readBytes = read(customerFileDescriptor, &previousCustomer, sizeof(struct Customer));
        if (readBytes == -1) {
            perror("Error while reading Customer record from file!");
            return false;
        }
        newCustomer.id = previousCustomer.id + 1;

        sprintf(writeBuffer, "%s%s", ADD_CUSTOMER, ADD_CUSTOMER_NAME);

        writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
        if (writeBytes == -1) {
            perror("Error writing ADD_CUSTOMER_NAME message to client!");
            return false;
        }

        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading customer name response from client!");
            return false;
        }

        strcpy(newCustomer.name, readBuffer);

        writeBytes = write(connFD, ADD_CUSTOMER_PHONE, strlen(ADD_CUSTOMER_PHONE));
        if (writeBytes == -1) {
            perror("Error writing ADD_CUSTOMER phone message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading customer phone response from client!");
            return false;
        }

        // Check if the phone number is a 10-digit number
        if (!is_valid_phone_number(readBuffer)) {
            writeBytes = write(connFD, ADD_CUSTOMER_WRONG_PHONE, strlen(ADD_CUSTOMER_WRONG_PHONE));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
            return false;
        }

        newCustomer.phone = atoi(readBuffer);

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading customer age response from client!");
            return false;
        }

        newCustomer.account = -700;

        snprintf(newCustomer.login, sizeof(newCustomer.login), "%s_%d", newCustomer.name, newCustomer.id);

        // Generate a random 5 - character password
        char password[6];
        srand(time(NULL));  // Seed the random number generator
        generate_random_password(password, 5);
        strcpy(newCustomer.password, password);

        // 3rd parameter specifies file permissions to be set if new file is created
        if (customerFileDescriptor == -1) {
            perror("Error while creating / opening customer file!");
            return false;
        }
        writeBytes = write(customerFileDescriptor, &newCustomer, sizeof(newCustomer));
        if (writeBytes == -1) {
            perror("Error while writing Customer record to file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(customerFileDescriptor, F_SETLK, &lock);

        close(customerFileDescriptor);
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    char firstName[100];
    sscanf(newCustomer.name, "%s", firstName);
    sprintf(writeBuffer, "%s\n%s_%d\n%s", ADD_CUSTOMER_LOGIN_PASS, firstName, newCustomer.id, newCustomer.password);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error sending customer loginID and password to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

    return newCustomer.id;
}

bool delete_account(int connFD) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Account account;

    writeBytes = write(connFD, ADMIN_DEL_ACCOUNT_NO, strlen(ADMIN_DEL_ACCOUNT_NO));
    if (writeBytes == -1) {
        perror("Error writing ADMIN_DEL_ACCOUNT_NO to client!");
        return false;
    }

    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading account number response from the client!");
        return false;
    }

    int accountNumber = atoi(readBuffer);

    int accountFileDescriptor = open(ACCOUNT_FILE, O_RDONLY);
    if (accountFileDescriptor == -1) {
        // Account record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing ACCOUNT_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
        return false;
    }

    int offset = lseek(accountFileDescriptor, accountNumber * sizeof(struct Account), SEEK_SET);
    if (errno == EINVAL) {
        // Customer record doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ACCOUNT_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing ACCOUNT_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
        return false;
    } else if (offset == -1) {
        perror("Error while seeking to required account record!");
        return false;
    }

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Account), getpid()};
    int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1) {
        perror("Error obtaining read lock on Account record!");
        return false;
    }

    readBytes = read(accountFileDescriptor, &account, sizeof(struct Account));
    if (readBytes == -1) {
        perror("Error while reading Account record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(accountFileDescriptor, F_SETLK, &lock);

    close(accountFileDescriptor);

    bzero(writeBuffer, sizeof(writeBuffer));
    if (account.balance == 0) {
        // No money, hence can close account
        account.active = false;
        accountFileDescriptor = open(ACCOUNT_FILE, O_WRONLY);
        if (accountFileDescriptor == -1) {
            perror("Error opening Account file in write mode!");
            return false;
        }

        offset = lseek(accountFileDescriptor, accountNumber * sizeof(struct Account), SEEK_SET);
        if (offset == -1) {
            perror("Error seeking to the Account!");
            return false;
        }

        lock.l_type = F_WRLCK;
        lock.l_start = offset;

        int lockingStatus = fcntl(accountFileDescriptor, F_SETLKW, &lock);
        if (lockingStatus == -1) {
            perror("Error obtaining write lock on the Account file!");
            return false;
        }

        writeBytes = write(accountFileDescriptor, &account, sizeof(struct Account));
        if (writeBytes == -1) {
            perror("Error deleting account record!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(accountFileDescriptor, F_SETLK, &lock);

        strcpy(writeBuffer, ADMIN_DEL_ACCOUNT_SUCCESS);
    } else
        // Account has some money ask customer to withdraw it
        strcpy(writeBuffer, ADMIN_DEL_ACCOUNT_FAILURE);
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error while writing final DEL message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

    return true;
}

bool modify_customer_info(int connFD) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Customer customer;

    int customerID;

    off_t offset;
    int lockingStatus;

    writeBytes = write(connFD, MODIFY_CUSTOMER_ID, strlen(MODIFY_CUSTOMER_ID));
    if (writeBytes == -1) {
        perror("Error while writing ADMIN_MOD_CUSTOMER_ID message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error while reading customer ID from client!");
        return false;
    }

    // Check if the string in readBuffer is an integer
    if (!is_pure_number(readBuffer)) {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
        return false;
    }

    customerID = atoi(readBuffer);

    int customerFileDescriptor = open(CUSTOMER_FILE, O_RDONLY);
    if (customerFileDescriptor == -1) {
        // Customer File doesn't exist
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
    }

    offset = lseek(customerFileDescriptor, customerID * sizeof(struct Customer), SEEK_SET);
    if (errno == EINVAL) {
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

    struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Customer), 0};

    // Lock the record to be read
    lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1) {
        perror("Couldn't obtain lock on customer record!");
        return false;
    }

    readBytes = read(customerFileDescriptor, &customer, sizeof(struct Customer));
    if (readBytes == -1) {
        perror("Error while reading customer record from the file!");
        return false;
    }

    // Unlock the record
    lock.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLK, &lock);

    close(customerFileDescriptor);

    writeBytes = write(connFD, MODIFY_CUSTOMER_MENU, strlen(MODIFY_CUSTOMER_MENU));
    if (writeBytes == -1) {
        perror("Error while writing ADMIN_MOD_CUSTOMER_MENU message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error while getting customer modification menu choice from client!");
        return false;
    }

    // Check if the string in readBuffer is an integer since we are expecting a menu choice which is an integer
    if (!is_pure_number(readBuffer)) {
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
        return false;
    }

    int choice = atoi(readBuffer);

    bzero(readBuffer, sizeof(readBuffer));
    switch (choice) {
        case 1:
            writeBytes = write(connFD, MODIFY_CUSTOMER_NEW_NAME, strlen(MODIFY_CUSTOMER_NEW_NAME));
            if (writeBytes == -1) {
                perror("Error while writing MODIFY_CUSTOMER_NEW_NAME message to client!");
                return false;
            }
            readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error while getting response for customer's new name from client!");
                return false;
            }
            strcpy(customer.name, readBuffer);
            break;
        case 2:
            writeBytes = write(connFD, MODIFY_CUSTOMER_NEW_PHONE, strlen(MODIFY_CUSTOMER_NEW_PHONE));
            if (writeBytes == -1) {
                perror("Error while writing MODIFY_CUSTOMER_NEW_PHONE message to client!");
                return false;
            }
            readBytes = read(connFD, &readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error while getting response for customer's new age from client!");
                return false;
            }

            if (!is_valid_phone_number(readBuffer)) {
                bzero(writeBuffer, sizeof(writeBuffer));
                strcpy(writeBuffer, ERRON_INPUT_FOR_NUMBER);
                writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
                if (writeBytes == -1) {
                    perror("Error while writing ERRON_INPUT_FOR_NUMBER message to client!");
                    return false;
                }
                readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
                return false;
            }
            int updatedPhone = atoi(readBuffer);
            customer.phone = updatedPhone;
            break;

        default:
            bzero(writeBuffer, sizeof(writeBuffer));
            strcpy(writeBuffer, INVALID_MENU_CHOICE);
            writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
            if (writeBytes == -1) {
                perror("Error while writing INVALID_MENU_CHOICE message to client!");
                return false;
            }
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
            return false;
    }

    customerFileDescriptor = open(CUSTOMER_FILE, O_WRONLY);
    if (customerFileDescriptor == -1) {
        perror("Error while opening customer file");
        return false;
    }
    offset = lseek(customerFileDescriptor, customerID * sizeof(struct Customer), SEEK_SET);
    if (offset == -1) {
        perror("Error while seeking to required customer record!");
        return false;
    }

    lock.l_type = F_WRLCK;
    lock.l_start = offset;
    lockingStatus = fcntl(customerFileDescriptor, F_SETLKW, &lock);
    if (lockingStatus == -1) {
        perror("Error while obtaining write lock on customer record!");
        return false;
    }

    writeBytes = write(customerFileDescriptor, &customer, sizeof(struct Customer));
    if (writeBytes == -1) {
        perror("Error while writing update customer info into file");
    }

    lock.l_type = F_UNLCK;
    fcntl(customerFileDescriptor, F_SETLKW, &lock);

    close(customerFileDescriptor);

    writeBytes = write(connFD, MODIFY_CUSTOMER_SUCCESS, strlen(MODIFY_CUSTOMER_SUCCESS));
    if (writeBytes == -1) {
        perror("Error while writing MODIFY_CUSTOMER_SUCCESS message to client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

    return true;
}

// helper functions
//  Function to check if a string is a 10-digit number
bool is_valid_phone_number(const char *phone) {
    if (strlen(phone) != 10) {
        return false;
    }
    for (int i = 0; i < 10; i++) {
        if (!isdigit(phone[i])) {
            return false;
        }
    }
    return true;
}

// Function to generate a random 5-character password
void generate_random_password(char *password, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < length; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        password[i] = charset[key];
    }
    password[length] = '\0';  // Null-terminate the string
}

// Function to check if a string is purely a number
bool is_pure_number(const char *str) {
    for (size_t i = 0; i < strlen(str); i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

#endif