#ifndef TRANSACTIONS
#define TRANSACTIONS

#include <time.h>

struct Transaction {
    int transactionID;  // 0, 1, 2, 3 ...
    int senderAccountNumber;
    int receiverAccountNumber;  //-1 if it is a withdraw, -2 if it is a deposit
    bool operation;             // 0 -> Withdraw, 1 -> Deposit
    time_t transactionTime;
};

#endif