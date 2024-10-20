#ifndef CUSTOMER_RECORD
#define CUSTOMER_RECORD

struct Customer {
    int id;
    char name[30];
    long int phone;
    int account;  // Account number

    // Login Credentials
    char login[50];  // name_id will be loginid
    char password[30];
};

#endif