#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Account {
    int accNo;
    char name[50];
    float balance;
};

void createAccount();
void depositMoney();
void withdrawMoney();
void checkBalance();

int main() {
    int choice;

    while (1) {
        printf("\n===== BANKING SYSTEM =====\n");
        printf("1. Create Account\n");
        printf("2. Deposit Money\n");
        printf("3. Withdraw Money\n");
        printf("4. Check Balance\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: createAccount(); break;
            case 2: depositMoney(); break;
            case 3: withdrawMoney(); break;
            case 4: checkBalance(); break;

            // CHANGE: Exit message added
            case 5:
                printf("Thank you for using the banking system!\n");
                exit(0);

            default: printf("Invalid choice!\n");
        }
    }
}

void createAccount() {

    struct Account acc, temp;
    FILE *fp;

    // CHANGE: .txt replaced with .dat because we are using binary file functions
    fp = fopen("bank.dat", "ab+");

    // CHANGE: file open error check
    if (fp == NULL) {
        printf("Unable to open file!\n");
        return;
    }

    printf("Enter Account Number: ");
    scanf("%d", &acc.accNo);

    // CHANGE: account number minimum digit validation (must be at least 6 digits)
    if (acc.accNo < 100000) {
        printf("Account number must be at least 6 digits!\n");
        fclose(fp);
        return;
    }

    // CHANGE: duplicate account number check
    rewind(fp);
    while (fread(&temp, sizeof(temp), 1, fp)) {
        if (temp.accNo == acc.accNo) {
            printf("Account already exists!\n");
            fclose(fp);
            return;
        }
    }

    printf("Enter Name: ");
    scanf(" %[^\n]", acc.name);

    printf("Enter Initial Balance: ");
    scanf("%f", &acc.balance);

    // CHANGE: minimum initial balance validation (minimum 500 required)
    if (acc.balance < 500) {
        printf("Initial balance must be at least 500!\n");
        fclose(fp);
        return;
    }

    fwrite(&acc, sizeof(acc), 1, fp);
    fclose(fp);

    printf("Account created successfully!\n");
}

void depositMoney() {

    struct Account acc;
    FILE *fp;
    int accNo, found = 0;
    float amount;

    // CHANGE: file extension corrected
    fp = fopen("bank.dat", "rb+");

    // CHANGE: file open check
    if (fp == NULL) {
        printf("No account file found!\n");
        return;
    }

    printf("Enter Account Number: ");
    scanf("%d", &accNo);

    while (fread(&acc, sizeof(acc), 1, fp)) {

        if (acc.accNo == accNo) {

            printf("Enter amount to deposit: ");
            scanf("%f", &amount);

            // CHANGE: negative deposit validation
            if (amount <= 0) {
                printf("Invalid amount!\n");
                fclose(fp);
                return;
            }

            acc.balance += amount;

            fseek(fp, -sizeof(acc), SEEK_CUR);
            fwrite(&acc, sizeof(acc), 1, fp);

            printf("Amount deposited successfully!\n");
            found = 1;
            break;
        }
    }

    if (!found)
        printf("Account not found!\n");

    fclose(fp);
}

void withdrawMoney() {

    struct Account acc;
    FILE *fp;
    int accNo, found = 0;
    float amount;

    // CHANGE: file extension corrected
    fp = fopen("bank.dat", "rb+");

    // CHANGE: file open check
    if (fp == NULL) {
        printf("No account file found!\n");
        return;
    }

    printf("Enter Account Number: ");
    scanf("%d", &accNo);

    while (fread(&acc, sizeof(acc), 1, fp)) {

        if (acc.accNo == accNo) {

            printf("Enter amount to withdraw: ");
            scanf("%f", &amount);

            // CHANGE: negative withdraw validation
            if (amount <= 0) {
                printf("Invalid amount!\n");
                fclose(fp);
                return;
            }

            if (amount > acc.balance) {
                printf("Insufficient balance!\n");
            }
            else {

                acc.balance -= amount;

                fseek(fp, -sizeof(acc), SEEK_CUR);
                fwrite(&acc, sizeof(acc), 1, fp);

                printf("Amount withdrawn successfully!\n");
            }

            found = 1;
            break;
        }
    }

    if (!found)
        printf("Account not found!\n");

    fclose(fp);
}

void checkBalance() {

    struct Account acc;
    FILE *fp;
    int accNo, found = 0;

    // CHANGE: file extension corrected
    fp = fopen("bank.dat", "rb");

    // CHANGE: file open check
    if (fp == NULL) {
        printf("No account file found!\n");
        return;
    }

    printf("Enter Account Number: ");
    scanf("%d", &accNo);

    while (fread(&acc, sizeof(acc), 1, fp)) {

        if (acc.accNo == accNo) {

            printf("\nAccount Holder: %s\n", acc.name);
            printf("Current Balance: %.2f\n", acc.balance);

            found = 1;
            break;
        }
    }

    if (!found)
        printf("Account not found!\n");

    fclose(fp);
}
