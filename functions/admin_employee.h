#ifndef ADMIN_EMPLOYEE_H
#define ADMIN_EMPLOYEE_H

#include <errno.h>   // For errno
#include <fcntl.h>   // For open(), O_RDONLY, lseek(), fcntl(), F_RDLCK
#include <stdio.h>   // For perror(), sprintf(), bzero()
#include <stdlib.h>  // For atoi()
#include <string.h>  // For strcpy(), strcat(), strlen()
#include <unistd.h>  // For ssize_t, read(), write()

#include "../include/common_includes.h"

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

    strcat(writeBuffer, "^");

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

int modify_employee(int connFD) {
    ssize_t readBytes, writeBytes;
    char readBuffer[1000], writeBuffer[1000];

    struct Employee newEmployee;

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s\n", MODIFY_EMPLOYEE);
    writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
    if (writeBytes == -1) {
        perror("Error writing MODIFY_EMPLOYEE message to client!");
        return false;
    }
    bzero(readBuffer, sizeof(readBuffer));
    readBytes = read(connFD, readBuffer, sizeof(readBuffer));
    if (readBytes == -1) {
        perror("Error reading employee ID from client!");
        return false;
    }

    int empID = atoi(readBuffer);

    // checking whether the employee exists or not
    if (get_employee_details(connFD, empID)) {
        int empFD = open(EMPLOYEE_FILE, O_RDWR, S_IRWXU);
        if (empFD == -1) {
            perror("Error while opening customer file");
            return -1;
        } else {
            int offset = lseek(empFD, empID * sizeof(struct Employee), SEEK_SET);
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

            readBytes = read(empFD, &newEmployee, sizeof(struct Employee));
            if (readBytes == -1) {
                perror("Error while reading Employee record from file!");
                return false;
            }

            lock.l_type = F_UNLCK;
            fcntl(empFD, F_SETLK, &lock);

            bzero(writeBuffer, sizeof(writeBuffer));
            sprintf(writeBuffer, "%s\n", MODIFY_EMPLOYEE_MENU);
            writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
            if (writeBytes == -1) {
                perror("Error writing MODIFY_EMPLOYEE_MENU message to client!");
                return false;
            }

            bzero(readBuffer, sizeof(readBuffer));
            readBytes = read(connFD, readBuffer, sizeof(readBuffer));
            if (readBytes == -1) {
                perror("Error reading employee modification choice from client!");
                return false;
            }

            int choice = atoi(readBuffer);

            if (choice == 1) {
                bzero(writeBuffer, sizeof(writeBuffer));
                sprintf(writeBuffer, "%s\n", MODIFY_EMPLOYEE_NEW_NAME);
                writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
                if (writeBytes == -1) {
                    perror("Error writing MODIFY_EMPLOYEE_NEW_NAME message to client!");
                    return false;
                }

                bzero(readBuffer, sizeof(readBuffer));
                readBytes = read(connFD, readBuffer, sizeof(readBuffer));
                if (readBytes == -1) {
                    perror("Error reading new employee name from client!");
                    return false;
                }

                strcpy(newEmployee.name, readBuffer);
            } else if (choice == 2) {
                bzero(writeBuffer, sizeof(writeBuffer));
                sprintf(writeBuffer, "%s\n", MODIFY_EMPLOYEE_NEW_ISMANAGER);
                writeBytes = write(connFD, writeBuffer, sizeof(writeBuffer));
                if (writeBytes == -1) {
                    perror("Error writing MODIFY_EMPLOYEE_NEW_ISMANAGER message to client!");
                    return false;
                }

                bzero(readBuffer, sizeof(readBuffer));
                readBytes = read(connFD, readBuffer, sizeof(readBuffer));
                if (readBytes == -1) {
                    perror("Error reading new employee isManager from client!");
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
            } else {
                return false;
            }

            offset = lseek(empFD, empID * sizeof(struct Employee), SEEK_SET);
            if (offset == -1) {
                perror("Error seeking to last Employee record!");
                // offset = 0;
                return false;
            }

            struct flock lock2 = {F_WRLCK, SEEK_SET, offset, sizeof(struct Employee), getpid()};
            lockingStatus = fcntl(empFD, F_SETLKW, &lock2);
            if (lockingStatus == -1) {
                perror("Error obtaining read lock on Employee record!");
                return false;
            }

            writeBytes = write(empFD, &newEmployee, sizeof(struct Employee));
            if (writeBytes == -1) {
                perror("Error while writing Employee record to file!");
                return false;
            }

            lock.l_type = F_UNLCK;
            fcntl(empFD, F_SETLK, &lock2);

            close(empFD);
        }
    }

    bzero(writeBuffer, sizeof(writeBuffer));
    sprintf(writeBuffer, "%s\n", "Successfully modified employee details^");
    writeBytes = write(connFD, writeBuffer, strlen(writeBuffer));
    if (writeBytes == -1) {
        perror("Error sending employee loginID and password to the client!");
        return false;
    }

    readBytes = read(connFD, readBuffer, sizeof(readBuffer));  // Dummy read

    return newEmployee.id;
}

#endif  // ADMIN_EMPLOYEE_H