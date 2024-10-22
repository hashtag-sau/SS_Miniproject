#ifndef SERVER_CONSTANTS
#define SERVER_CONSTANTS
#define INITIAL_PROMPT \
    "\
╔════════════════════════════════════════════════════════════════╗\n\
║                                                                ║\n\
║   🌟 Welcome to OnlyBanks of India! 🌟                         ║\n\
║                                                                ║\n\
║   Please identify yourself:                                    ║\n\
║   1. 👑 Admin                                                  ║\n\
║   2. 👤 Customer                                               ║\n\
║   3. 🧑‍💼 Employee                                             ║\n\
║                                                                ║\n\
║   Enter any other number to exit.                              ║\n\
║                                                                ║\n\
╚════════════════════════════════════════════════════════════════╝\n\
Enter your choice: "

#define INITIAL_PROMPT2 "Enter your choice: 1. admin 2. customer 3. employee 4. exit\n"
// ========== COMMON TEXT =============================

// LOGIN
#define LOGIN_ID "Enter your login ID"
#define PASSWORD "Enter your password \n# "
#define INVALID_LOGIN "The login ID specified doesn't exist!$"
#define INVALID_PASSWORD "The password specified doesn't match!$"

// GET ACCOUNT DETAILS
#define GET_ACCOUNT_NUMBER "Enter the account number of the account you're searching for"
#define GET_CUSTOMER_ID "Enter the customer ID of the customer you're searching for"

#define ERRON_INPUT_FOR_NUMBER "Ya its robust(somewhat) (⌐■_■) \nIt seems you have passed a sequence of alphabets when a number was expected or you have entered an invalid number!\nYou'll now be redirected to the main menu!^"

#define INVALID_MENU_CHOICE "It seems you've made an invalid menu choice\nYou'll now be redirected to the main menu!^"

#define CUSTOMER_ID_DOESNT_EXIT "No customer could be found for the given ID"
#define CUSTOMER_LOGIN_ID_DOESNT_EXIT "No customer could be found for the given login ID$"

#define ACCOUNT_ID_DOESNT_EXIT "No account could be found for the given account number"

#define TRANSACTIONS_NOT_FOUND "No transactions were performed on this account by the customer!^"

// ====================================================

// ========== ADMIN SPECIFIC TEXT======================

// LOGIN WELCOME
#define ADMIN_LOGIN_WELCOME "Welcome dear admin! With great power comes great responsibility!\nEnter your credentials to unlock this power!"
#define ADMIN_LOGIN_SUCCESS "Welcome superman!"

// ADMIN MENU
#define ADMIN_MENU "1. Get Employee Details\n2. Add Employee\n3. modify employee\n4. Add Admin \n5. \nPress any other key to logout"

// DELETE ACCOUNT
#define ADMIN_DEL_ACCOUNT_NO "What is the account number of the account you want to delete?"
#define ADMIN_DEL_ACCOUNT_SUCCESS "This account has been successfully deleted\nRedirecting you to the main menu ...^"
#define ADMIN_DEL_ACCOUNT_FAILURE "This account cannot be deleted since it still has some money\nRedirecting you to the main menu ...^"

#define ADMIN_MOD_CUSTOMER_SUCCESS "The required modification was successfully made!\nYou'll now be redirected to the main menu!^"

#define ADMIN_LOGOUT "Logging you out now superman! Good bye!$"
#define ADMIN_DOESNT_EXIST "this account doesn't exist"

// admin manages employee
#define GET_EMPLOYEE_ID "Enter Employee ID you are looking for: "
#define ADD_EMPLOYEE "__Enter Employee details__"
#define ADD_EMPLOYEE_NAME "Enter name: "
#define ADD_EMPLOYEE_LOGIN_PASS "The login ID and password for the employee is : "
#define MODIFY_EMPLOYEE "Enter the ID of the employee who's information you want to edit"
#define MODIFY_EMPLOYEE_MENU "Which information would you like to modify?\n1. Name 2. ManagerRole\nPress any other key to cancel"
#define MODIFY_EMPLOYEE_NEW_NAME "What's the updated name? "
#define MODIFY_EMPLOYEE_NEW_ISMANAGER "Is the employee a manager? (1/0) "

// admin admin
#define ADD_ADMIN "__Enter Admin details__"
#define ADD_ADMIN_NAME "Enter name: "
#define ADD_ADMIN_LOGIN_PASS "The login ID and password for the admin is : "
#define ADD_ADMIN_SUCCESS "The admin has been successfully added!^"
// ====================================================

// ========== EMPLOYEE SPECIFIC TEXT======================
#define EMPLOYEE_LOGIN_WELCOME "Welcome dear employee! Enter your cerdentials"
#define EMPLOYEE_LOGIN_SUCCESS "Sucessfully logged in. Now start working, and remember not to take rest"

#define EMPLOYEE_LOGOUT "Logging you out now dear employee!!$"
#define EMPLOYEE_MENU "1. Get Customer Details\n2. Get Account Details\n3. Get Transaction details\n4. Add Account\n5. Delete Account\n6. Modify Customer Information\nPress any other key to logout"

#define EMPLOYEE_ID_DOESNT_EXIT "No employee could be found for the given ID"
#define EMPLOYEE_LOGIN_ID_DOESNT_EXIT "No employee could be found for the given login ID$"

// employee creating account for customer
#define NEW_ACCOUNT_NUMBER "The newly created account's number is :"
#define ADD_CUSTOMER "__Enter Customer Details__\n"
#define ADD_CUSTOMER_NAME "Enter customer's name: "
#define ADD_CUSTOMER_PHONE "Enter customer's phone number: "
#define ADD_CUSTOMER_WRONG_PHONE "It seems you've entered an invalid phone number!^"
#define ADD_CUSTOMER_LOGIN_PASS "The login ID and password for the customer is : "

// modify customer info
#define MODIFY_CUSTOMER_ID "Enter the ID of the customer who's information you want to edit"
#define MODIFY_CUSTOMER_MENU "Which information would you like to modify?\n1. Name 2. Phone\nPress any other key to cancel"
#define MODIFY_CUSTOMER_NEW_NAME "What's the updated name? "
#define MODIFY_CUSTOMER_NEW_PHONE "What's the updated phone number? "
#define MODIFY_CUSTOMER_SUCCESS "The required modification was successfully made!\nYou'll now be redirected to the main menu!^"

// ========== CUSTOMER SPECIFIC TEXT===================

// LOGIN WELCOME
#define CUSTOMER_LOGIN_WELCOME "Welcome dear customer! Enter your cerdentials"
#define CUSTOMER_LOGIN_SUCCESS "Sucessfully logged in. Now go to counter 5"

#define CUSTOMER_ALREADY_LOGGED_IN "You're already logged in!$"

#define CUSTOMER_LOGOUT "Logging you out now dear customer! Good bye!$"
#define SEND_AMOUNT "How much is it that you want to send?"
#define SEND_ACCOUNT "Enter the account number of the account you want to send money to"
#define SEND_AMOUNT_SUCCESS "The specified amount has been successfully sent to the specified account!^"

// cusotmer MENU
#define CUSTOMER_MENU "1. Get Customer Details\n2. Deposit Money\n3. Withdraw Money\n4. Send Money \n5. Get Balance\n6. Get Transaction information\n7. Change Password\nPress any other key to logout"

#define ACCOUNT_DEACTIVATED "It seems your account has been deactivated!^"

#define DEPOSIT_AMOUNT "How much is it that you want to add into your bank?"
#define DEPOSIT_AMOUNT_INVALID "You seem to have passed an invalid amount!^"
#define DEPOSIT_AMOUNT_SUCCESS "The specified amount has been successfully added to your bank account!^"

#define WITHDRAW_AMOUNT "How much is it that you want to withdraw from your bank?"
#define WITHDRAW_AMOUNT_INVALID "You seem to have either passed an invalid amount or you don't have enough money in your bank to withdraw the specified amount^"
#define WITHDRAW_AMOUNT_SUCCESS "The specified amount has been successfully withdrawn from your bank account!^"

#define PASSWORD_CHANGE_OLD_PASS "Enter your old password"
#define PASSWORD_CHANGE_OLD_PASS_INVALID "The entered password doesn't seem to match with the old password"
#define PASSWORD_CHANGE_NEW_PASS "Enter the new password"
#define PASSWORD_CHANGE_NEW_PASS_RE "Reenter the new password"
#define PASSWORD_CHANGE_NEW_PASS_INVALID "The new password and the reentered passwords don't seem to pass!^"
#define PASSWORD_CHANGE_SUCCESS "Password successfully changed!^"

// ====================================================

#define SESSION_TIMEOUT "No data received within 60 seconds.\n closing connection bye bye\n"

#define ACCOUNT_FILE "../db/account_db"
#define CUSTOMER_FILE "../db/customer_db"
#define TRANSACTION_FILE "../db/transaction_db"
#define ADMIN_FILE "../db/admin_db"
#define EMPLOYEE_FILE "../db/employee_db"

#endif