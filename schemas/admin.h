#ifndef ADMIN_RECORD
#define ADMIN_RECORD

struct Admin {
    int id;
    char name[30];
    // Login Credentials
    char login[50];  // name_id will be loginid
    char password[30];
};

#endif