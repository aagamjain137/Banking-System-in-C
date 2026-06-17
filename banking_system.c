#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ==================== STRUCTURES ====================

struct Account {
    char accNo[20];
    char name[50];
    char phone[15];
    char accType[20]; // Savings / Current
    double balance;
    int isActive; // 1 = active, 0 = deleted
};

struct Transaction {
    char accNo[20];
    char type[20];   // DEPOSIT / WITHDRAW / TRANSFER_IN / TRANSFER_OUT / CREATE
    double amount;
    double balanceAfter;
    char datetime[30];
};

// ==================== FUNCTION DECLARATIONS ====================

void createAccount();
void depositMoney();
void withdrawMoney();
void checkBalance();
void transferMoney();
void deleteAccount();
void modifyAccount();
void viewTransactionHistory();
void listAllAccounts();

int  accountExists(const char *accNo);
void logTransaction(const char *accNo, const char *type, double amount, double balanceAfter);
void getCurrentTime(char *buffer);
void clearInputBuffer();
void printHeader(const char *title);

// ==================== UTILITY FUNCTIONS ====================

void clearInputBuffer() {
    while (getchar() != '\n');
}

void getCurrentTime(char *buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", tm_info);
}

void printHeader(const char *title) {
    printf("\n========================================\n");
    printf("       %s\n", title);
    printf("========================================\n");
}

// Returns 1 if active account exists, 0 otherwise
int accountExists(const char *accNo) {
    struct Account acc;
    FILE *fp = fopen("bank.dat", "rb");
    if (fp == NULL) return 0;
    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (strcmp(acc.accNo, accNo) == 0 && acc.isActive == 1) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void logTransaction(const char *accNo, const char *type, double amount, double balanceAfter) {
    struct Transaction txn;
    strncpy(txn.accNo, accNo, sizeof(txn.accNo) - 1);
    strncpy(txn.type, type, sizeof(txn.type) - 1);
    txn.amount = amount;
    txn.balanceAfter = balanceAfter;
    getCurrentTime(txn.datetime);

    FILE *fp = fopen("transactions.dat", "ab");
    if (fp == NULL) return;
    fwrite(&txn, sizeof(txn), 1, fp);
    fclose(fp);
}

// ==================== CORE FUNCTIONS ====================

void createAccount() {
    printHeader("CREATE NEW ACCOUNT");

    struct Account acc, temp;
    FILE *fp;

    // Account Number
    printf("Enter Account Number (min 6 digits): ");
    scanf("%19s", acc.accNo);
    clearInputBuffer();

    if (strlen(acc.accNo) < 6) {
        printf("[ERROR] Account number must be at least 6 characters!\n");
        return;
    }

    // Check duplicate
    fp = fopen("bank.dat", "rb");
    if (fp != NULL) {
        while (fread(&temp, sizeof(temp), 1, fp)) {
            if (strcmp(temp.accNo, acc.accNo) == 0 && temp.isActive == 1) {
                printf("[ERROR] Account with this number already exists!\n");
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }

    // Name
    printf("Enter Full Name: ");
    fgets(acc.name, sizeof(acc.name), stdin);
    acc.name[strcspn(acc.name, "\n")] = '\0';
    if (strlen(acc.name) == 0) {
        printf("[ERROR] Name cannot be empty!\n");
        return;
    }

    // Phone
    printf("Enter Phone Number: ");
    scanf("%14s", acc.phone);
    clearInputBuffer();

    // Account Type
    printf("Account Type (1. Savings  2. Current): ");
    int typeChoice;
    scanf("%d", &typeChoice);
    clearInputBuffer();
    if (typeChoice == 1)
        strcpy(acc.accType, "Savings");
    else if (typeChoice == 2)
        strcpy(acc.accType, "Current");
    else {
        printf("[ERROR] Invalid account type!\n");
        return;
    }

    // Initial Balance
    printf("Enter Initial Deposit (min Rs. 500): ");
    scanf("%lf", &acc.balance);
    clearInputBuffer();
    if (acc.balance < 500) {
        printf("[ERROR] Minimum initial deposit is Rs. 500!\n");
        return;
    }

    acc.isActive = 1;

    // Write to file
    fp = fopen("bank.dat", "ab");
    if (fp == NULL) {
        printf("[ERROR] Could not open file!\n");
        return;
    }
    fwrite(&acc, sizeof(acc), 1, fp);
    fclose(fp);

    logTransaction(acc.accNo, "CREATE", acc.balance, acc.balance);
    printf("\n[SUCCESS] Account created successfully!\n");
    printf("  Account No : %s\n", acc.accNo);
    printf("  Name       : %s\n", acc.name);
    printf("  Type       : %s\n", acc.accType);
    printf("  Balance    : Rs. %.2lf\n", acc.balance);
}

void depositMoney() {
    printHeader("DEPOSIT MONEY");

    struct Account acc;
    char accNo[20];
    double amount;
    int found = 0;

    printf("Enter Account Number: ");
    scanf("%19s", accNo);
    clearInputBuffer();

    FILE *fp = fopen("bank.dat", "rb+");
    if (fp == NULL) {
        printf("[ERROR] No accounts found!\n");
        return;
    }

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (strcmp(acc.accNo, accNo) == 0 && acc.isActive == 1) {
            printf("Account Holder : %s\n", acc.name);
            printf("Current Balance: Rs. %.2lf\n", acc.balance);
            printf("Enter amount to deposit: Rs. ");
            scanf("%lf", &amount);
            clearInputBuffer();

            if (amount <= 0) {
                printf("[ERROR] Amount must be greater than 0!\n");
                fclose(fp);
                return;
            }

            acc.balance += amount;
            fseek(fp, -(long)sizeof(acc), SEEK_CUR);
            fwrite(&acc, sizeof(acc), 1, fp);

            logTransaction(accNo, "DEPOSIT", amount, acc.balance);
            printf("[SUCCESS] Rs. %.2lf deposited successfully!\n", amount);
            printf("New Balance: Rs. %.2lf\n", acc.balance);
            found = 1;
            break;
        }
    }

    if (!found) printf("[ERROR] Account not found!\n");
    fclose(fp);
}

void withdrawMoney() {
    printHeader("WITHDRAW MONEY");

    struct Account acc;
    char accNo[20];
    double amount;
    int found = 0;

    printf("Enter Account Number: ");
    scanf("%19s", accNo);
    clearInputBuffer();

    FILE *fp = fopen("bank.dat", "rb+");
    if (fp == NULL) {
        printf("[ERROR] No accounts found!\n");
        return;
    }

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (strcmp(acc.accNo, accNo) == 0 && acc.isActive == 1) {
            printf("Account Holder : %s\n", acc.name);
            printf("Current Balance: Rs. %.2lf\n", acc.balance);
            printf("Enter amount to withdraw: Rs. ");
            scanf("%lf", &amount);
            clearInputBuffer();

            if (amount <= 0) {
                printf("[ERROR] Amount must be greater than 0!\n");
                fclose(fp);
                return;
            }

            if (amount > acc.balance - 500) {
                printf("[ERROR] Insufficient balance! (Minimum balance Rs. 500 required)\n");
                printf("Max withdrawable: Rs. %.2lf\n", acc.balance - 500);
            } else {
                acc.balance -= amount;
                fseek(fp, -(long)sizeof(acc), SEEK_CUR);
                fwrite(&acc, sizeof(acc), 1, fp);
                logTransaction(accNo, "WITHDRAW", amount, acc.balance);
                printf("[SUCCESS] Rs. %.2lf withdrawn successfully!\n", amount);
                printf("Remaining Balance: Rs. %.2lf\n", acc.balance);
            }

            found = 1;
            break;
        }
    }

    if (!found) printf("[ERROR] Account not found!\n");
    fclose(fp);
}

void checkBalance() {
    printHeader("CHECK BALANCE");

    struct Account acc;
    char accNo[20];
    int found = 0;

    printf("Enter Account Number: ");
    scanf("%19s", accNo);
    clearInputBuffer();

    FILE *fp = fopen("bank.dat", "rb");
    if (fp == NULL) {
        printf("[ERROR] No accounts found!\n");
        return;
    }

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (strcmp(acc.accNo, accNo) == 0 && acc.isActive == 1) {
            printf("\n-------- ACCOUNT DETAILS --------\n");
            printf("  Account No  : %s\n", acc.accNo);
            printf("  Name        : %s\n", acc.name);
            printf("  Phone       : %s\n", acc.phone);
            printf("  Type        : %s\n", acc.accType);
            printf("  Balance     : Rs. %.2lf\n", acc.balance);
            printf("---------------------------------\n");
            found = 1;
            break;
        }
    }

    if (!found) printf("[ERROR] Account not found!\n");
    fclose(fp);
}

void transferMoney() {
    printHeader("TRANSFER MONEY");

    char fromAcc[20], toAcc[20];
    double amount;
    struct Account acc;
    int fromFound = 0, toFound = 0;

    printf("Enter YOUR Account Number: ");
    scanf("%19s", fromAcc);
    clearInputBuffer();

    printf("Enter RECEIVER'S Account Number: ");
    scanf("%19s", toAcc);
    clearInputBuffer();

    if (strcmp(fromAcc, toAcc) == 0) {
        printf("[ERROR] Cannot transfer to the same account!\n");
        return;
    }

    if (!accountExists(fromAcc)) {
        printf("[ERROR] Your account not found!\n");
        return;
    }
    if (!accountExists(toAcc)) {
        printf("[ERROR] Receiver's account not found!\n");
        return;
    }

    printf("Enter amount to transfer: Rs. ");
    scanf("%lf", &amount);
    clearInputBuffer();

    if (amount <= 0) {
        printf("[ERROR] Amount must be greater than 0!\n");
        return;
    }

    FILE *fp = fopen("bank.dat", "rb+");
    if (fp == NULL) {
        printf("[ERROR] Could not open file!\n");
        return;
    }

    // Deduct from sender
    rewind(fp);
    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (strcmp(acc.accNo, fromAcc) == 0 && acc.isActive == 1) {
            if (amount > acc.balance - 500) {
                printf("[ERROR] Insufficient balance!\n");
                fclose(fp);
                return;
            }
            acc.balance -= amount;
            fseek(fp, -(long)sizeof(acc), SEEK_CUR);
            fwrite(&acc, sizeof(acc), 1, fp);
            logTransaction(fromAcc, "TRANSFER_OUT", amount, acc.balance);
            fromFound = 1;
            break;
        }
    }

    // Add to receiver
    rewind(fp);
    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (strcmp(acc.accNo, toAcc) == 0 && acc.isActive == 1) {
            acc.balance += amount;
            fseek(fp, -(long)sizeof(acc), SEEK_CUR);
            fwrite(&acc, sizeof(acc), 1, fp);
            logTransaction(toAcc, "TRANSFER_IN", amount, acc.balance);
            toFound = 1;
            break;
        }
    }

    fclose(fp);

    if (fromFound && toFound)
        printf("[SUCCESS] Rs. %.2lf transferred successfully from %s to %s!\n", amount, fromAcc, toAcc);
    else
        printf("[ERROR] Transfer failed!\n");
}

void deleteAccount() {
    printHeader("DELETE ACCOUNT");

    struct Account acc;
    char accNo[20];
    char confirm;
    int found = 0;

    printf("Enter Account Number to delete: ");
    scanf("%19s", accNo);
    clearInputBuffer();

    FILE *fp = fopen("bank.dat", "rb+");
    if (fp == NULL) {
        printf("[ERROR] No accounts found!\n");
        return;
    }

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (strcmp(acc.accNo, accNo) == 0 && acc.isActive == 1) {
            printf("\nAccount Found:\n");
            printf("  Name   : %s\n", acc.name);
            printf("  Balance: Rs. %.2lf\n", acc.balance);
            printf("\nAre you sure you want to delete? (y/n): ");
            scanf(" %c", &confirm);
            clearInputBuffer();

            if (confirm == 'y' || confirm == 'Y') {
                acc.isActive = 0; // Soft delete
                fseek(fp, -(long)sizeof(acc), SEEK_CUR);
                fwrite(&acc, sizeof(acc), 1, fp);
                logTransaction(accNo, "DELETED", 0, acc.balance);
                printf("[SUCCESS] Account deleted successfully!\n");
            } else {
                printf("Deletion cancelled.\n");
            }

            found = 1;
            break;
        }
    }

    if (!found) printf("[ERROR] Account not found!\n");
    fclose(fp);
}

void modifyAccount() {
    printHeader("MODIFY ACCOUNT DETAILS");

    struct Account acc;
    char accNo[20];
    int found = 0;

    printf("Enter Account Number to modify: ");
    scanf("%19s", accNo);
    clearInputBuffer();

    FILE *fp = fopen("bank.dat", "rb+");
    if (fp == NULL) {
        printf("[ERROR] No accounts found!\n");
        return;
    }

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (strcmp(acc.accNo, accNo) == 0 && acc.isActive == 1) {
            printf("\nCurrent Details:\n");
            printf("  Name : %s\n", acc.name);
            printf("  Phone: %s\n", acc.phone);

            printf("\nEnter New Name (press Enter to keep current): ");
            char newName[50];
            fgets(newName, sizeof(newName), stdin);
            newName[strcspn(newName, "\n")] = '\0';
            if (strlen(newName) > 0)
                strncpy(acc.name, newName, sizeof(acc.name) - 1);

            printf("Enter New Phone (press Enter to keep current): ");
            char newPhone[15];
            fgets(newPhone, sizeof(newPhone), stdin);
            newPhone[strcspn(newPhone, "\n")] = '\0';
            if (strlen(newPhone) > 0)
                strncpy(acc.phone, newPhone, sizeof(acc.phone) - 1);

            fseek(fp, -(long)sizeof(acc), SEEK_CUR);
            fwrite(&acc, sizeof(acc), 1, fp);
            printf("[SUCCESS] Account details updated successfully!\n");
            found = 1;
            break;
        }
    }

    if (!found) printf("[ERROR] Account not found!\n");
    fclose(fp);
}

void viewTransactionHistory() {
    printHeader("TRANSACTION HISTORY");

    char accNo[20];
    struct Transaction txn;
    int found = 0;

    printf("Enter Account Number: ");
    scanf("%19s", accNo);
    clearInputBuffer();

    if (!accountExists(accNo)) {
        printf("[ERROR] Account not found!\n");
        return;
    }

    FILE *fp = fopen("transactions.dat", "rb");
    if (fp == NULL) {
        printf("No transaction history found!\n");
        return;
    }

    printf("\n%-20s %-15s %-12s %-12s\n", "Date & Time", "Type", "Amount", "Balance");
    printf("--------------------------------------------------------------\n");

    while (fread(&txn, sizeof(txn), 1, fp)) {
        if (strcmp(txn.accNo, accNo) == 0) {
            printf("%-20s %-15s Rs.%-9.2lf Rs.%-9.2lf\n",
                txn.datetime, txn.type, txn.amount, txn.balanceAfter);
            found = 1;
        }
    }

    if (!found) printf("No transactions found for this account.\n");
    fclose(fp);
}

void listAllAccounts() {
    printHeader("ALL ACTIVE ACCOUNTS");

    struct Account acc;
    int count = 0;

    FILE *fp = fopen("bank.dat", "rb");
    if (fp == NULL) {
        printf("No accounts found!\n");
        return;
    }

    printf("\n%-15s %-25s %-12s %-10s %-12s\n", "Acc No", "Name", "Phone", "Type", "Balance");
    printf("------------------------------------------------------------------------\n");

    while (fread(&acc, sizeof(acc), 1, fp)) {
        if (acc.isActive == 1) {
            printf("%-15s %-25s %-12s %-10s Rs.%.2lf\n",
                acc.accNo, acc.name, acc.phone, acc.accType, acc.balance);
            count++;
        }
    }

    printf("------------------------------------------------------------------------\n");
    printf("Total Active Accounts: %d\n", count);
    fclose(fp);
}

// ==================== MAIN MENU ====================

int main() {
    int choice;

    printf("\n========================================\n");
    printf("    WELCOME TO C BANKING SYSTEM\n");
    printf("========================================\n");

    while (1) {
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

        if (scanf("%d", &choice) != 1) {
            printf("[ERROR] Invalid input! Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {
            case 1:  createAccount();          break;
            case 2:  depositMoney();            break;
            case 3:  withdrawMoney();           break;
            case 4:  checkBalance();            break;
            case 5:  transferMoney();           break;
            case 6:  deleteAccount();           break;
            case 7:  modifyAccount();           break;
            case 8:  viewTransactionHistory();  break;
            case 9:  listAllAccounts();         break;
            case 10:
                printf("\nThank you for using C Banking System. Goodbye!\n");
                exit(0);
            default:
                printf("[ERROR] Invalid choice! Please enter 1-10.\n");
        }
    }

    return 0;
}
