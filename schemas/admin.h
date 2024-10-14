#ifndef ADMIN_RECORD
#define ADMIN_RECORD

struct Admin {
    int id;
    char name[25];
    int phone;

    // Login Credentials
    char login[30];  // name_id will be loginid
    char password[30];
};

#endif