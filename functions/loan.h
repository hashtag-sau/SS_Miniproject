#ifndef LOAN_H
#define LOAN_H

// Include necessary headers
#include <stdio.h>
#include <stdlib.h>

// Define constants
#define MAX_LOANS 100
#include "../include/common_includes.h"

// Function prototypes
int apply_loan(int connFD, struct Customer *customer);
int manage_loan(int connFD, int loanID);
int get_loan_details(int connFD, int loanID);
int get_customer_loans(int connFD, int customerID);

#endif  // LOAN_H