#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mysql.h>

#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASSWORD "aagam"
#define DB_NAME "bank_system_v2"

static MYSQL *conn = NULL;

struct Account
{
    char accNo[20];
    char name[100];
    char phone[15];
    char accType[20];
    double balance;
    int isActive;
};

static void clearInputBuffer(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
    }
}

static void printHeader(const char *title)
{
    printf("\n========================================\n");
    printf("       %s\n", title);
    printf("========================================\n");
}

static int runQuery(const char *sql)
{
    if (mysql_query(conn, sql) != 0)
    {
        fprintf(stderr, "[DATABASE ERROR] %s\n", mysql_error(conn));
        return 0;
    }
    return 1;
}

static void escapeString(const char *input, char *output, size_t outputSize)
{
    unsigned long needed = mysql_real_escape_string(conn, output, input, (unsigned long)strlen(input));
    if (needed >= outputSize)
    {
        output[0] = '\0';
    }
    else
    {
        output[needed] = '\0';
    }
}

static int connectDatabase(void)
{
    char sql[256];

    conn = mysql_init(NULL);
    if (conn == NULL)
    {
        fprintf(stderr, "MySQL initialization failed.\n");
        return 0;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD,
                           NULL, 3306, NULL, 0) == NULL)
    {
        fprintf(stderr, "MySQL connection failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        conn = NULL;
        return 0;
    }

    snprintf(sql, sizeof(sql),
             "CREATE DATABASE IF NOT EXISTS %s CHARACTER SET utf8mb4 "
             "COLLATE utf8mb4_unicode_ci",
             DB_NAME);
    if (!runQuery(sql) || mysql_select_db(conn, DB_NAME) != 0)
    {
        fprintf(stderr, "Could not select database: %s\n", mysql_error(conn));
        mysql_close(conn);
        conn = NULL;
        return 0;
    }

    const char *accountsSql =
        "CREATE TABLE IF NOT EXISTS accounts ("
        "acc_no VARCHAR(19) PRIMARY KEY,"
        "name VARCHAR(99) NOT NULL,"
        "phone VARCHAR(14) NOT NULL,"
        "acc_type VARCHAR(20) NOT NULL,"
        "balance DECIMAL(12,2) NOT NULL DEFAULT 0.00,"
        "is_active TINYINT NOT NULL DEFAULT 1)";
    const char *transactionsSql =
        "CREATE TABLE IF NOT EXISTS transactions ("
        "transaction_id BIGINT AUTO_INCREMENT PRIMARY KEY,"
        "acc_no VARCHAR(19) NOT NULL,"
        "type VARCHAR(20) NOT NULL,"
        "amount DECIMAL(12,2) NOT NULL,"
        "balance_after DECIMAL(12,2) NOT NULL,"
        "datetime TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "INDEX idx_transactions_account (acc_no))";

    if (!runQuery(accountsSql) || !runQuery(transactionsSql))
    {
        mysql_close(conn);
        conn = NULL;
        return 0;
    }

    return 1;
}

static int accountExists(const char *accNo)
{
    char escaped[64], sql[256];
    MYSQL_RES *result;
    MYSQL_ROW row;

    escapeString(accNo, escaped, sizeof(escaped));
    snprintf(sql, sizeof(sql),
             "SELECT 1 FROM accounts WHERE acc_no='%s' AND is_active=1 LIMIT 1",
             escaped);

    if (!runQuery(sql))
        return 0;
    result = mysql_store_result(conn);
    if (result == NULL)
    {
        fprintf(stderr, "[DATABASE ERROR] %s\n", mysql_error(conn));
        return 0;
    }

    row = mysql_fetch_row(result);
    mysql_free_result(result);
    return row != NULL;
}

static void logTransaction(const char *accNo, const char *type,
                           double amount, double balanceAfter)
{
    char escapedAcc[64], escapedType[64], sql[512];

    escapeString(accNo, escapedAcc, sizeof(escapedAcc));
    escapeString(type, escapedType, sizeof(escapedType));
    snprintf(sql, sizeof(sql),
             "INSERT INTO transactions (acc_no,type,amount,balance_after) "
             "VALUES ('%s','%s',%.2f,%.2f)",
             escapedAcc, escapedType, amount, balanceAfter);
    (void)runQuery(sql);
}

static int getAccount(const char *accNo, struct Account *acc)
{
    char escaped[64], sql[256];
    MYSQL_RES *result;
    MYSQL_ROW row;

    escapeString(accNo, escaped, sizeof(escaped));
    snprintf(sql, sizeof(sql),
             "SELECT acc_no,name,phone,acc_type,balance,is_active "
             "FROM accounts WHERE acc_no='%s' LIMIT 1",
             escaped);

    if (!runQuery(sql))
        return 0;
    result = mysql_store_result(conn);
    if (result == NULL)
        return 0;

    row = mysql_fetch_row(result);
    if (row == NULL)
    {
        mysql_free_result(result);
        return 0;
    }

    snprintf(acc->accNo, sizeof(acc->accNo), "%s", row[0] ? row[0] : "");
    snprintf(acc->name, sizeof(acc->name), "%s", row[1] ? row[1] : "");
    snprintf(acc->phone, sizeof(acc->phone), "%s", row[2] ? row[2] : "");
    snprintf(acc->accType, sizeof(acc->accType), "%s", row[3] ? row[3] : "");
    acc->balance = row[4] ? atof(row[4]) : 0.0;
    acc->isActive = row[5] ? atoi(row[5]) : 0;
    mysql_free_result(result);
    return 1;
}

static int validAccountNumber(const char *accNo)
{
    size_t i, len = strlen(accNo);
    if (len < 6 || len > 19)
        return 0;
    for (i = 0; i < len; i++)
    {
        if (!isdigit((unsigned char)accNo[i]))
            return 0;
    }
    return 1;
}

void createAccount(void)
{
    struct Account acc;
    char eNo[64], eName[160], ePhone[64], eType[64], sql[512];
    int typeChoice;

    printHeader("CREATE NEW ACCOUNT");
    printf("Enter Account Number (6-19 digits): ");
    if (scanf("%19s", acc.accNo) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (!validAccountNumber(acc.accNo))
    {
        printf("[ERROR] Account number must contain 6-19 digits.\n");
        return;
    }
    if (accountExists(acc.accNo))
    {
        printf("[ERROR] Active account with this number already exists.\n");
        return;
    }

    if (getAccount(acc.accNo, &acc))
    {
        printf("[ERROR] This account number already exists in the database.\n");
        return;
    }

    printf("Enter Full Name: ");
    if (fgets(acc.name, sizeof(acc.name), stdin) == NULL)
        return;
    acc.name[strcspn(acc.name, "\n")] = '\0';
    if (strlen(acc.name) == 0)
    {
        printf("[ERROR] Name cannot be empty.\n");
        return;
    }

    printf("Enter Phone Number: ");
    if (scanf("%14s", acc.phone) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    printf("Account Type (1. Savings  2. Current): ");
    if (scanf("%d", &typeChoice) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    if (typeChoice == 1)
        strcpy(acc.accType, "Savings");
    else if (typeChoice == 2)
        strcpy(acc.accType, "Current");
    else
    {
        printf("[ERROR] Invalid account type.\n");
        return;
    }

    printf("Enter Initial Deposit (minimum Rs. 500): ");
    if (scanf("%lf", &acc.balance) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    if (acc.balance < 500)
    {
        printf("[ERROR] Minimum initial deposit is Rs. 500.\n");
        return;
    }

    escapeString(acc.accNo, eNo, sizeof(eNo));
    escapeString(acc.name, eName, sizeof(eName));
    escapeString(acc.phone, ePhone, sizeof(ePhone));
    escapeString(acc.accType, eType, sizeof(eType));

    snprintf(sql, sizeof(sql),
             "INSERT INTO accounts (acc_no,name,phone,acc_type,balance,is_active) "
             "VALUES ('%s','%s','%s','%s',%.2f,1)",
             eNo, eName, ePhone, eType, acc.balance);
    if (!runQuery(sql))
        return;

    logTransaction(acc.accNo, "CREATE", acc.balance, acc.balance);
    printf("\n[SUCCESS] Account created successfully!\n");
    printf("  Account No : %s\n  Name       : %s\n  Type       : %s\n  Balance    : Rs. %.2f\n",
           acc.accNo, acc.name, acc.accType, acc.balance);
}

void depositMoney(void)
{
    char accNo[20], eNo[64], sql[512];
    double amount;
    struct Account acc;

    printHeader("DEPOSIT MONEY");
    printf("Enter Account Number: ");
    if (scanf("%19s", accNo) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (!getAccount(accNo, &acc) || !acc.isActive)
    {
        printf("[ERROR] Active account not found.\n");
        return;
    }
    printf("Account Holder : %s\nCurrent Balance: Rs. %.2f\n", acc.name, acc.balance);
    printf("Enter amount to deposit: Rs. ");
    if (scanf("%lf", &amount) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    if (amount <= 0)
    {
        printf("[ERROR] Amount must be greater than 0.\n");
        return;
    }

    escapeString(accNo, eNo, sizeof(eNo));
    snprintf(sql, sizeof(sql),
             "UPDATE accounts SET balance=balance+%.2f WHERE acc_no='%s' AND is_active=1",
             amount, eNo);
    if (!runQuery(sql))
        return;
    if (!getAccount(accNo, &acc))
    {
        printf("[ERROR] Could not reload account.\n");
        return;
    }
    logTransaction(accNo, "DEPOSIT", amount, acc.balance);
    printf("[SUCCESS] Rs. %.2f deposited. New balance: Rs. %.2f\n", amount, acc.balance);
}

void withdrawMoney(void)
{
    char accNo[20], eNo[64], sql[512];
    double amount;
    struct Account acc;

    printHeader("WITHDRAW MONEY");
    printf("Enter Account Number: ");
    if (scanf("%19s", accNo) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (!getAccount(accNo, &acc) || !acc.isActive)
    {
        printf("[ERROR] Active account not found.\n");
        return;
    }
    printf("Account Holder : %s\nCurrent Balance: Rs. %.2f\n", acc.name, acc.balance);
    printf("Enter amount to withdraw: Rs. ");
    if (scanf("%lf", &amount) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    if (amount <= 0)
    {
        printf("[ERROR] Amount must be greater than 0.\n");
        return;
    }
    if (amount > acc.balance - 500)
    {
        printf("[ERROR] Insufficient balance! Minimum balance Rs. 500 required.\n");
        printf("Max withdrawable: Rs. %.2f\n", acc.balance - 500);
        return;
    }

    escapeString(accNo, eNo, sizeof(eNo));
    snprintf(sql, sizeof(sql),
             "UPDATE accounts SET balance=balance-%.2f "
             "WHERE acc_no='%s' AND is_active=1 AND balance-%.2f>=500",
             amount, eNo, amount);
    if (!runQuery(sql) || mysql_affected_rows(conn) != 1)
    {
        printf("[ERROR] Withdrawal could not be completed.\n");
        return;
    }
    if (!getAccount(accNo, &acc))
        return;
    logTransaction(accNo, "WITHDRAW", amount, acc.balance);
    printf("[SUCCESS] Rs. %.2f withdrawn. Remaining balance: Rs. %.2f\n",
           amount, acc.balance);
}

void checkBalance(void)
{
    char accNo[20];
    struct Account acc;

    printHeader("CHECK BALANCE");
    printf("Enter Account Number: ");
    if (scanf("%19s", accNo) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (!getAccount(accNo, &acc) || !acc.isActive)
    {
        printf("[ERROR] Account not found.\n");
        return;
    }
    printf("\n-------- ACCOUNT DETAILS --------\n");
    printf("  Account No  : %s\n  Name        : %s\n  Phone       : %s\n  Type        : %s\n  Balance     : Rs. %.2f\n",
           acc.accNo, acc.name, acc.phone, acc.accType, acc.balance);
    printf("---------------------------------\n");
}

void transferMoney(void)
{
    char fromAcc[20], toAcc[20], eFrom[64], eTo[64], sql[512];
    struct Account from, to;
    double amount;
    int ok = 0;

    printHeader("TRANSFER MONEY");
    printf("Enter YOUR Account Number: ");
    if (scanf("%19s", fromAcc) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    printf("Enter RECEIVER'S Account Number: ");
    if (scanf("%19s", toAcc) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    if (strcmp(fromAcc, toAcc) == 0)
    {
        printf("[ERROR] Cannot transfer to the same account.\n");
        return;
    }
    if (!getAccount(fromAcc, &from) || !from.isActive)
    {
        printf("[ERROR] Sender account not found.\n");
        return;
    }
    if (!getAccount(toAcc, &to) || !to.isActive)
    {
        printf("[ERROR] Receiver account not found.\n");
        return;
    }

    printf("Enter amount to transfer: Rs. ");
    if (scanf("%lf", &amount) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    if (amount <= 0)
    {
        printf("[ERROR] Amount must be greater than 0.\n");
        return;
    }
    if (amount > from.balance - 500)
    {
        printf("[ERROR] Insufficient balance! Minimum balance Rs. 500 required.\n");
        return;
    }

    escapeString(fromAcc, eFrom, sizeof(eFrom));
    escapeString(toAcc, eTo, sizeof(eTo));
    if (!runQuery("START TRANSACTION"))
        return;

    snprintf(sql, sizeof(sql),
             "UPDATE accounts SET balance=balance-%.2f "
             "WHERE acc_no='%s' AND is_active=1 AND balance-%.2f>=500",
             amount, eFrom, amount);
    if (!runQuery(sql) || mysql_affected_rows(conn) != 1)
        goto rollback;

    snprintf(sql, sizeof(sql),
             "UPDATE accounts SET balance=balance+%.2f WHERE acc_no='%s' AND is_active=1",
             amount, eTo);
    if (!runQuery(sql) || mysql_affected_rows(conn) != 1)
        goto rollback;

    if (!getAccount(fromAcc, &from) || !getAccount(toAcc, &to))
        goto rollback;

    snprintf(sql, sizeof(sql),
             "INSERT INTO transactions (acc_no,type,amount,balance_after) "
             "VALUES ('%s','TRANSFER_OUT',%.2f,%.2f)",
             eFrom, amount, from.balance);
    if (!runQuery(sql))
        goto rollback;
    snprintf(sql, sizeof(sql),
             "INSERT INTO transactions (acc_no,type,amount,balance_after) "
             "VALUES ('%s','TRANSFER_IN',%.2f,%.2f)",
             eTo, amount, to.balance);
    if (!runQuery(sql))
        goto rollback;

    if (!runQuery("COMMIT"))
        goto rollback;
    ok = 1;

rollback:
    if (!ok)
    {
        (void)mysql_query(conn, "ROLLBACK");
        printf("[ERROR] Transfer failed; transaction rolled back.\n");
        return;
    }
    printf("[SUCCESS] Rs. %.2f transferred from %s to %s.\n", amount, fromAcc, toAcc);
    printf("Sender balance: Rs. %.2f | Receiver balance: Rs. %.2f\n",
           from.balance, to.balance);
}

void deleteAccount(void)
{
    char accNo[20], eNo[64], sql[256], confirm;
    struct Account acc;

    printHeader("DELETE ACCOUNT");
    printf("Enter Account Number to delete: ");
    if (scanf("%19s", accNo) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    if (!getAccount(accNo, &acc) || !acc.isActive)
    {
        printf("[ERROR] Account not found.\n");
        return;
    }
    printf("Name: %s\nBalance: Rs. %.2f\nAre you sure? (y/n): ", acc.name, acc.balance);
    if (scanf(" %c", &confirm) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    if (confirm != 'y' && confirm != 'Y')
    {
        printf("Deletion cancelled.\n");
        return;
    }

    escapeString(accNo, eNo, sizeof(eNo));
    snprintf(sql, sizeof(sql),
             "UPDATE accounts SET is_active=0 WHERE acc_no='%s' AND is_active=1", eNo);
    if (!runQuery(sql) || mysql_affected_rows(conn) != 1)
    {
        printf("[ERROR] Account could not be deleted.\n");
        return;
    }
    logTransaction(accNo, "DELETED", 0, acc.balance);
    printf("[SUCCESS] Account deactivated successfully.\n");
}

void modifyAccount(void)
{
    char accNo[20], newName[50], newPhone[15], eNo[64], eName[160], ePhone[64], sql[512];
    struct Account acc;

    printHeader("MODIFY ACCOUNT DETAILS");
    printf("Enter Account Number to modify: ");
    if (scanf("%19s", accNo) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    if (!getAccount(accNo, &acc) || !acc.isActive)
    {
        printf("[ERROR] Account not found.\n");
        return;
    }

    printf("Current name : %s\nCurrent phone: %s\n", acc.name, acc.phone);
    printf("Enter new name (Enter to keep current): ");
    if (fgets(newName, sizeof(newName), stdin) == NULL)
        return;
    newName[strcspn(newName, "\n")] = '\0';
    if (strlen(newName) == 0)
        snprintf(newName, sizeof(newName), "%s", acc.name);

    printf("Enter new phone (Enter to keep current): ");
    if (fgets(newPhone, sizeof(newPhone), stdin) == NULL)
        return;
    newPhone[strcspn(newPhone, "\n")] = '\0';
    if (strlen(newPhone) == 0)
        snprintf(newPhone, sizeof(newPhone), "%s", acc.phone);

    escapeString(accNo, eNo, sizeof(eNo));
    escapeString(newName, eName, sizeof(eName));
    escapeString(newPhone, ePhone, sizeof(ePhone));
    snprintf(sql, sizeof(sql),
             "UPDATE accounts SET name='%s',phone='%s' WHERE acc_no='%s' AND is_active=1",
             eName, ePhone, eNo);
    if (runQuery(sql))
        printf("[SUCCESS] Account details updated.\n");
}

void viewTransactionHistory(void)
{
    char accNo[20], eNo[64], sql[256];
    MYSQL_RES *result;
    MYSQL_ROW row;

    printHeader("TRANSACTION HISTORY");
    printf("Enter Account Number: ");
    if (scanf("%19s", accNo) != 1)
    {
        clearInputBuffer();
        return;
    }
    clearInputBuffer();
    if (!accountExists(accNo))
    {
        printf("[ERROR] Active account not found.\n");
        return;
    }

    escapeString(accNo, eNo, sizeof(eNo));
    snprintf(sql, sizeof(sql),
             "SELECT datetime,type,amount,balance_after FROM transactions "
             "WHERE acc_no='%s' ORDER BY transaction_id",
             eNo);
    if (!runQuery(sql))
        return;
    result = mysql_store_result(conn);
    if (result == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return;
    }

    printf("\n%-20s %-15s %-12s %-12s\n", "Date & Time", "Type", "Amount", "Balance");
    printf("----------------------------------------------------------------\n");
    if (mysql_num_rows(result) == 0)
    {
        printf("No transactions found for this account.\n");
    }
    else
    {
        while ((row = mysql_fetch_row(result)) != NULL)
        {
            printf("%-20s %-15s Rs.%-9.2f Rs.%-9.2f\n",
                   row[0] ? row[0] : "", row[1] ? row[1] : "",
                   row[2] ? atof(row[2]) : 0.0, row[3] ? atof(row[3]) : 0.0);
        }
    }
    mysql_free_result(result);
}

void listAllAccounts(void)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int count = 0;

    printHeader("ALL ACTIVE ACCOUNTS");
    if (!runQuery("SELECT acc_no,name,phone,acc_type,balance FROM accounts "
                  "WHERE is_active=1 ORDER BY acc_no"))
        return;
    result = mysql_store_result(conn);
    if (result == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return;
    }

    printf("\n%-15s %-25s %-12s %-10s %-12s\n",
           "Acc No", "Name", "Phone", "Type", "Balance");
    printf("------------------------------------------------------------------------\n");
    while ((row = mysql_fetch_row(result)) != NULL)
    {
        printf("%-15s %-25s %-12s %-10s Rs.%.2f\n",
               row[0] ? row[0] : "", row[1] ? row[1] : "",
               row[2] ? row[2] : "", row[3] ? row[3] : "",
               row[4] ? atof(row[4]) : 0.0);
        count++;
    }
    printf("------------------------------------------------------------------------\nTotal Active Accounts: %d\n", count);
    mysql_free_result(result);
}

int main(void)
{
    int choice;

    printf("\n========================================\n");
    printf("       C BANKING SYSTEM \n");
    printf("========================================\n");

    if (!connectDatabase())
    {
        fprintf(stderr, "\nCheck MySQL Server is running and DB_PASSWORD in the source is correct.\n");
        return 1;
    }
    printf(" Database connected successfully.\n");

    while (1)
    {
        printf("\n===== MAIN MENU =====\n");
        printf(" 1. Create Account\n");
        printf(" 2. Deposit Money\n");
        printf(" 3. Withdraw Money\n");
        printf(" 4. Check Balance\n");
        printf(" 5. Transfer Money\n");
        printf(" 6. Delete Account\n");
        printf(" 7. Modify Account Details\n");
        printf(" 8. Transaction History\n");
        printf(" 9. List All Accounts\n");
        printf("10. Exit\n");
        printf("=====================\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("[ERROR] Invalid input. Enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice)
        {
        case 1:
            createAccount();
            break;
        case 2:
            depositMoney();
            break;
        case 3:
            withdrawMoney();
            break;
        case 4:
            checkBalance();
            break;
        case 5:
            transferMoney();
            break;
        case 6:
            deleteAccount();
            break;
        case 7:
            modifyAccount();
            break;
        case 8:
            viewTransactionHistory();
            break;
        case 9:
            listAllAccounts();
            break;
        case 10:
            printf("\nThank you for using C Banking System. Goodbye!\n");
            mysql_close(conn);
            return 0;
        default:
            printf("[ERROR] Invalid choice. Enter 1-10.\n");
        }
    }
}
