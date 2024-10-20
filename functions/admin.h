#ifndef ADMIN_FUNCTIONS
#define ADMIN_FUNCTIONS

// Include necessary headers
#include "../include/constants.h"
#include "../schemas/admin.h"
#include "../schemas/customer.h"
#include "../schemas/employee.h"
#include "auxillaries.h"
#include "common.h"
#include "customer.h"
#include "employee.h"

// Function prototypes for admin management
bool login_handler_admin(int connFD, struct Admin *ad);
bool admin_operation_handler(int connFD);
int add_employee(int connFD);
bool get_employee_details(int connFD, int employeeID);
int add_admin(int connFD);
// functions
bool login_handler_admin(int connFD, struct Admin *ad) {
    ssize_t readBytes, writeBytes;             // Number of bytes written to / read from the socket
    char readBuffer[1000], writeBuffer[1000];  // Buffer for reading from / writing to the client
    char tempBuffer[1000];
    struct Admin admin;

    int ID;

    bzero(readBuffer, sizeof(readBuffer));
    bzero(writeBuffer, sizeof(writeBuffer));

    // Get login message for respective user type

    strcpy(writeBuffer, ADMIN_LOGIN_WELCOME);

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

    int AdminFD = open(ADMIN_FILE, O_RDONLY);
    if (AdminFD == -1) {
        perror("Error opening customer_db in RDONLY mode!");
        return false;
    }

    off_t offset = lseek(AdminFD, ID * sizeof(struct Admin), SEEK_SET);
    if (offset >= 0) {
        struct flock lock;
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = ID * sizeof(struct Admin);
        lock.l_len = sizeof(struct Admin);
        lock.l_pid = 0;  // by setting 0 we Let the kernel fill in the correct PID

        int lockingStatus = fcntl(AdminFD, F_SETLKW, &lock);
        if (lockingStatus == -1) {
            perror("Failed to acquire read lock on customer_db record");
            return false;
        }

        readBytes = read(AdminFD, &admin, sizeof(struct Admin));
        if (readBytes == -1) {
            perror("Error reading admin record from customer_db!");
        }

        lock.l_type = F_UNLCK;
        fcntl(AdminFD, F_SETLK, &lock);

        if (strcmp(admin.login, readBuffer) == 0)
            userFound = true;

        close(AdminFD);
    } else {
        writeBytes = write(connFD, ADMIN_DOESNT_EXIST, strlen(ADMIN_DOESNT_EXIST));
    }

    char password[100];

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

    strcpy(password, readBuffer);

    if (userFound) {
        if (strcmp(password, admin.password) == 0) {  // password matching
            *ad = admin;
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

bool admin_operation_handler(int connFD) {
    struct Admin admin;
    struct Employee employee;
    struct Account account;
    struct Customer customer;

    int accontNumber;
    int customerID;

    if (login_handler_admin(connFD, &admin)) {
        ssize_t writeBytes, readBytes;             // Number of bytes read from / written to the client
        char readBuffer[1000], writeBuffer[1000];  // A buffer used for reading & writing to the client
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, ADMIN_LOGIN_SUCCESS);
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
                    get_employee_details(connFD, -1);
                    break;
                case 2:
                    // printf("fsklfewdefub\n");
                    add_employee(connFD);
                    break;
                case 3:
                    // modify_employee(connFD, -1);
                    break;
                case 4:
                    add_admin(connFD);
                    break;
                default:
                    writeBytes = write(connFD, ADMIN_LOGOUT, strlen(ADMIN_LOGOUT));
                    return false;
            }
        }
    } else {
        // ADMIN LOGIN FAILED
        return false;
    }
    return true;
}

bool get_employee_details(int connFD, int empID) {
    ssize_t readBytes, writeBytes;              // Number of bytes read from / written to the socket
    char readBuffer[1000], writeBuffer[10000];  // A buffer for reading from / writing to the socket
    char tempBuffer[1000];

    struct Employee employee;
    int empFD;
    struct flock lock = {F_RDLCK, SEEK_SET, 0, sizeof(struct Account), 0};

    if (empID == -1) {
        writeBytes = write(connFD, GET_EMPLOYEE_ID, strlen(GET_EMPLOYEE_ID));
        if (writeBytes == -1) {
            perror("Error while writing GET_EMPLOYEE_ID message to client!");
            return false;
        }

        bzero(readBuffer, sizeof(readBuffer));
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error getting employee ID from client!");
            ;
            return false;
        }

        empID = atoi(readBuffer);
    }

    empFD = open(EMPLOYEE_FILE, O_RDONLY);
    if (empFD == -1) {
        // Employee File doesn't exist
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing EMPLOYEE_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
        return false;
    }
    int offset = lseek(empFD, empID * sizeof(struct Employee), SEEK_SET);
    if (errno == EINVAL) {
        // Employee record doesn't exist
        // printf("debug at get_employee_details\n %s    %s", empID, offset);
        bzero(writeBuffer, sizeof(writeBuffer));
        strcpy(writeBuffer, CUSTOMER_ID_DOESNT_EXIT);
        strcat(writeBuffer, "^");
        writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing EMPLOYEE_ID_DOESNT_EXIT message to client!");
            return false;
        }
        readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
        return false;
    } else if (offset == -1) {
        perror("Error while seeking to required employee record!");
        return false;
    }
    lock.l_start = offset;

    int lockingStatus = fcntl(empFD, F_SETLKW, &lock);
    if (lockingStatus == -1) {
        perror("Error while obtaining read lock on the Employee file!");
        return false;
    }

    readBytes = read(empFD, &employee, sizeof(struct Employee));
    if (readBytes == -1) {
        perror("Error reading employee record from file!");
        return false;
    }

    lock.l_type = F_UNLCK;
    fcntl(empFD, F_SETLK, &lock);

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "Employee Details - \n\tID : %d\n\tName : %s\n\tisManager : %s\n", employee.id, employee.name, (employee.ismanager ? "YES" : "NO"));

    strcat(writeBuffer, "\n\nYou'll now be redirected to the main menu...^");

    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing employee info to client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
    return true;
}

int add_employee(int connFD) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Employee newEmployee, previousEmployee;

    int empFD = open(EMPLOYEE_FILE, O_RDWR, S_IRWXU);
    if (empFD == -1) {
        perror("Error while opening customer file");
        return -1;
    } else {
        int offset = lseek(empFD, -sizeof(struct Employee), SEEK_END);
        if (offset == -1) {
            perror("Error seeking to last Employee record!");
            // offset = 0;
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Employee), getpid()};
        int lockingStatus = fcntl(empFD, F_SETLKW, &lock);
        if (lockingStatus == -1) {
            perror("Error obtaining read lock on Employee record!");
            return false;
        }

        readBytes = read(empFD, &previousEmployee, sizeof(struct Employee));
        if (readBytes == -1) {
            perror("Error while reading Employee record from file!");
            return false;
        }
        newEmployee.id = previousEmployee.id + 1;

        sprintf(writeBuffer, "%s\n%s", ADD_EMPLOYEE, ADD_EMPLOYEE_NAME);

        writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
        if (writeBytes == -1) {
            perror("Error writing ADD_EMP_NAME message to client!");
            return false;
        }

        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading employee name response from client!");
            return false;
        }

        strcpy(newEmployee.name, readBuffer);

        sprintf(writeBuffer, "%s: ", "is Employee a Manager Enter 0 for NO and 1 for YES");

        writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
        if (writeBytes == -1) {
            perror("Error writing isManager message to client!");
            return false;
        }

        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading isManager response from client!");
            return false;
        }

        if (atoi(readBuffer) != 0 && atoi(readBuffer) != 1) {
            sprintf(writeBuffer, "%s: ", "wrong input for is Manager^");

            writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
            if (writeBytes == -1) {
                perror("Error writing isManager message to client!");
                return false;
            }

            readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read
            return false;
        }
        newEmployee.ismanager = atoi(readBuffer);

        snprintf(newEmployee.login, sizeof(newEmployee.login), "%s_%d", newEmployee.name, newEmployee.id);

        // Generate a random 5 - character password
        char password[6];
        srand(time(NULL));  // Seed the random number generator
        generate_random_password(password, 5);
        strcpy(newEmployee.password, password);

        // 3rd parameter specifies file permissions to be set if new file is created
        if (empFD == -1) {
            perror("Error while creating / opening employee file!");
            return false;
        }
        writeBytes = write(empFD, &newEmployee, sizeof(newEmployee));
        if (writeBytes == -1) {
            perror("Error while writing Employee record to file!");
            return false;
        }

        lock.l_type = F_UNLCK;
        fcntl(empFD, F_SETLK, &lock);

        close(empFD);
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    // char firstName[100];
    // sscanf(newEmployee.name, "%s", firstName);
    sprintf(writeBuffer, "%s\n%s_%d\n%s", ADD_EMPLOYEE_LOGIN_PASS, newEmployee.name, newEmployee.id, newEmployee.password);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error sending employee loginID and password to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

    return newEmployee.id;
}

int add_admin(int connFD) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Admin newAdmin, previousAdmin;

    int adminFD = open(ADMIN_FILE, O_RDWR, S_IRWXU);
    if (adminFD == -1) {
        perror("Error while opening customer file");
        return -1;
    } else {
        int offset = lseek(adminFD, -sizeof(struct Admin), SEEK_END);
        if (offset == -1) {
            perror("Error seeking to last Admin record!");
            return false;
        }

        struct flock lock = {F_RDLCK, SEEK_SET, offset, sizeof(struct Admin), getpid()};
        int lockingStatus = fcntl(adminFD, F_SETLKW, &lock);
        if (lockingStatus == -1) {
            perror("Error obtaining read lock on Admin record!");
            return false;
        }

        readBytes = read(adminFD, &previousAdmin, sizeof(struct Admin));
        if (readBytes == -1) {
            perror("Error while reading Admin record from file!");
            return false;
        }
        newAdmin.id = previousAdmin.id + 1;

        sprintf(writeBuffer, "%s\n%s", ADD_ADMIN, ADD_ADMIN_NAME);

        writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
        if (writeBytes == -1) {
            perror("Error writing ADD_ADMIN_NAME message to client!");
            return false;
        }

        readBytes = read(connFD, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error reading admin name response from client!");
            return false;
        }

        strcpy(newAdmin.name, readBuffer);

        // char firstName[100];
        // sscanf(newAdmin.name, "%s", firstName);
        snprintf(newAdmin.login, sizeof(newAdmin.login), "%s_%d", newAdmin.name, newAdmin.id);

        // Generate a random 5 - character password
        char password[6];
        srand(time(NULL));  // Seed the random number generator
        generate_random_password(password, 5);
        strcpy(newAdmin.password, password);

        if (adminFD == -1) {
            perror("Error while creating / opening admin file!");
            return false;
        }
        writeBytes = write(adminFD, &newAdmin, sizeof(newAdmin));
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    char firstName[100];
    sscanf(newAdmin.name, "%s", firstName);
    sprintf(writeBuffer, "%s\n%s_%d\n%s", ADD_ADMIN_LOGIN_PASS, firstName, newAdmin.id, newAdmin.password);
    strcat(writeBuffer, "^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error sending admin loginID and password to the client!");
        return false;
    }
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

    return newAdmin.id;
}

#endif  // ADMIN_H