#ifndef EMPLOYEE_H
#define EMPLOYEE_H

struct Employee {
    int id;
    char name[30];
    bool ismanager;
    // Login Credentials
    char login[30];  // name_id will be loginid
    char password[30];
};

#endif  // EMPLOYEE_H