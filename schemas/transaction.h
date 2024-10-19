#ifndef TRANSACTIONS
#define TRANSACTIONS

#include <time.h>

struct Transaction {
    int transactionID;  // 0, 1, 2, 3 ...
    int senderAccountNumber;
    int receiverAccountNumber;  //-1 if it is a withdraw, -2 if it is a deposit
    int amount;
    time_t transactionTimestamp;
};

#endif