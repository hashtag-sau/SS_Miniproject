#ifndef SERVER_CONSTANTS
#define SERVER_CONSTANTS
#define INITIAL_PROMPT                                                                                                                                                                                                    \
    "\n"                                                                                                                                                                                                                  \
    "\033[1;36m╔══════════════════════════════════════════════════════════════╗\033[0m\n" \
    "\033[1;36m║                                                              ║\033[0m\n"                                                                                                                             \
    "\033[1;36m║       \033[1;33m🏦  WELCOME TO  ONLYBANKS OF INDIA! 🏦   \033[1;36m              ║\033[0m\n"                                                                                                     \
    "\033[1;36m║                                                              ║\033[0m\n"                                                                                                                             \
    "\033[1;36m║              Please identify yourself:                       ║\033[0m\n"                                                                                                                             \
    "\033[1;36m║              1. 👑 Admin                                     ║\033[0m\n"                                                                                                                           \
    "\033[1;36m║              2. 👤 Customer                                  ║\033[0m\n"                                                                                                                           \
    "\033[1;36m║              3. 🧑‍💼 Employee                                ║\033[0m\n"                                                                                                                      \
    "\033[1;36m║                                                              ║\033[0m\n"                                                                                                                             \
    "\033[1;36m║              Enter any other number to exit.                 ║\033[0m\n"                                                                                                                             \
    "\033[1;36m║                                                              ║\033[0m\n"                                                                                                                             \
    "\033[1;36m╚══════════════════════════════════════════════════════════════╝\033[0m\n" \
    "Enter your choice: "

#define INITIAL_PROMPT2 "Enter your choice: 1. admin 2. customer 3. employee 4. exit\n"
// ========== COMMON TEXT =============================

// LOGIN
#define LOGIN_ID "\033[1;34m🔐 Enter your login ID: \033[0m"
#define PASSWORD "\033[1;33m🔑 Enter your password: \033[0m\n#"
#define INVALID_LOGIN "\033[1;31m🚫 The login ID specified doesn't exist! \033[0m$"
#define INVALID_PASSWORD "\033[1;31m❌ The password specified doesn't match! \033[0m$"

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
#define ADMIN_LOGIN_WELCOME \
    "\
\033[1;34m🚀 Welcome aboard, Admin Commander! 🚀\033[0m\n\
\033[1;33m🔑 Please enter your credentials to gain access.\033[0m\n"

#define ADMIN_LOGIN_SUCCESS \
    "\
\033[1;32m✅ Access Granted! Hello, Admin! ✅\033[0m\n"

// ADMIN MENU
#define ADMIN_MENU \
    "\
\033[36m╔════════════════════════════════════════════════════════════╗\033[0m\n\
\033[36m║ \033[35m            🛠️  \033[1;32mAdmin Menu \033[0;35m🛠️              \033[36m║\033[0m\n\
\033[36m╠════════════════════════════════════════════════════════════╣\033[0m\n\
\033[36m║ \033[33m1. Get Employee Details    \033[32m👤                      \033[36m║\033[0m\n\
\033[36m║ \033[33m2. Add Employee            \033[32m➕👨‍💼                   \033[36m║\033[0m\n\
\033[36m║ \033[33m3. Modify Employee         \033[32m✏️👨‍💼                  \033[36m║\033[0m\n\
\033[36m║ \033[33m4. List All Employees      \033[32m📋👥                   \033[36m║\033[0m\n\
\033[36m║ \033[33m5. Add Admin               \033[32m➕🛡️                    \033[36m║\033[0m\n\
\033[36m╠════════════════════════════════════════════════════════════╣\033[0m\n\
\033[36m║ \033[31mPress any other key to Logout   \033[32m🚪                   \033[36m║\033[0m\n\
\033[36m╚════════════════════════════════════════════════════════════╝\033[0m\n"
#define ADMIN_MENUold "1. Get Employee Details\n2. Add Employee\n3. modify employee\n4. list all employees \n5. Add Admin\nPress any other key to logout"

// DELETE ACCOUNT
#define ADMIN_DEL_ACCOUNT_NO "What is the account number of the account you want to delete?"
#define ADMIN_DEL_ACCOUNT_SUCCESS "This account has been successfully deleted\nRedirecting you to the main menu ...^"
#define ADMIN_DEL_ACCOUNT_FAILURE "This account cannot be deleted since it still has some money\nRedirecting you to the main menu ...^"

#define ADMIN_MOD_CUSTOMER_SUCCESS "The required modification was successfully made!\nYou'll now be redirected to the main menu!^"

#define ADMIN_LOGOUT "Logging you out now superman! Good bye!$"
#define ADMIN_DOESNT_EXIST "this account doesn't exist"

// admin manages employee
#define GET_EMPLOYEE_ID "🔍 Please enter the Employee ID you are searching for: "
#define ADD_EMPLOYEE "📝 __Provide the New Employee's Details__"
#define ADD_EMPLOYEE_NAME "👤 Enter the employee's name: "
#define ADD_EMPLOYEE_LOGIN_PASS "🔑 The login ID and password for the employee are: "
#define MODIFY_EMPLOYEE "✏️ Enter the ID of the employee whose information you'd like to update: "
#define MODIFY_EMPLOYEE_MENU "⚙️ What information would you like to modify?\n1. Name 2. Manager Role\nPress any other key to cancel."
#define MODIFY_EMPLOYEE_NEW_NAME "✏️ Please enter the updated name: "
#define MODIFY_EMPLOYEE_NEW_ISMANAGER "👔 Is this employee a manager? (1 for Yes / 0 for No): "
#define ADD_EMPLOYEE_SUCCESS "🎉 The employee has been successfully added!^"
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
#define CUSTOMER_LOGIN_WELCOME \
    "\
\033[1;34m👋 Welcome, valued customer! 🎉\033[0m\n\
\033[1;33mPlease enter your credentials to continue.\033[0m\n"

#define CUSTOMER_LOGIN_SUCCESS \
    "\
\033[1;32m🎊 Successfully logged in! We hope you have a great experience. 🎊\033[0m\n"

#define CUSTOMER_ALREADY_LOGGED_IN "You're already logged in!$"

#define CUSTOMER_LOGOUT "🔒 Logging you out, valued customer! See you again soon!$"
#define SEND_AMOUNT "💸 How much would you like to transfer today?"
#define SEND_ACCOUNT "🏦 Please enter the account number to which you'd like to send money."
#define SEND_AMOUNT_SUCCESS "✅ Transfer successful! The amount has been sent to the specified account!^"

// cusotmer MENU
#define CUSTOMER_MENUold "1. Get Customer Details\n2. Deposit Money\n3. Withdraw Money\n4. Send Money \n5. Get Balance\n6. View Passbook\n7. Change Password\nPress any other key to logout"
#define RESET_COLOR "\033[0m"
#define HEADER_COLOR "\033[1;34m"  // Bright Blue
#define OPTION_COLOR "\033[1;32m"  // Bright Green
#define LOGOUT_COLOR "\033[1;31m"  // Bright Red

#define CUSTOMER_MENU                                                                                                                                                                                                \
    "╔══════════════════════════════════════════════════════════════════╗\n" \
    "║" HEADER_COLOR "                       🏦 Customer Menu                           " RESET_COLOR                                                                                                                 \
    "║\n"                                                                                                                                                                                                          \
    "╠══════════════════════════════════════════════════════════════════╣\n" \
    "║" OPTION_COLOR " 🌟 1. Get Customer Details                                      " RESET_COLOR                                                                                                                  \
    "║\n"                                                                                                                                                                                                          \
    "║" OPTION_COLOR " 💰 2. Deposit Money                                             " RESET_COLOR                                                                                                                  \
    "║\n"                                                                                                                                                                                                          \
    "║" OPTION_COLOR " 💸 3. Withdraw Money                                            " RESET_COLOR                                                                                                                  \
    "║\n"                                                                                                                                                                                                          \
    "║" OPTION_COLOR " 📤  4. Send Money                                               " RESET_COLOR                                                                                                                  \
    "║\n"                                                                                                                                                                                                          \
    "║" OPTION_COLOR " 📊 5. Get Balance                                               " RESET_COLOR                                                                                                                  \
    "║\n"                                                                                                                                                                                                          \
    "║" OPTION_COLOR " 📖 6. View Passbook                                             " RESET_COLOR                                                                                                                  \
    "║\n"                                                                                                                                                                                                          \
    "║" OPTION_COLOR " 🔒 7. Change Password                                           " RESET_COLOR                                                                                                                  \
    "║\n"                                                                                                                                                                                                          \
    "║" LOGOUT_COLOR " 🔙 Press any other key to logout                                " RESET_COLOR                                                                                                                  \
    "║\n"                                                                                                                                                                                                          \
    "╚══════════════════════════════════════════════════════════════════╝\n"

#define ACCOUNT_DEACTIVATED "It seems your account has been deactivated!^"

#define DEPOSIT_AMOUNT "💰 How much would you like to deposit into your account?\n"
#define DEPOSIT_AMOUNT_INVALID "⚠️ Oops! That doesn't seem like a valid amount! Please try again.^"
#define DEPOSIT_AMOUNT_SUCCESS "✅ Success! The amount has been added to your account!^"

#define WITHDRAW_AMOUNT "🏧 How much would you like to withdraw from your account?\n"
#define WITHDRAW_AMOUNT_INVALID "⚠️ Insufficient funds or invalid amount! Please check your balance and try again.^"
#define WITHDRAW_AMOUNT_SUCCESS "✅ Success! The amount has been withdrawn from your account!^"

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