#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Account {
    char accNo[20];
    char name[50];
    double balance;
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

        if (scanf("%d", &choice) != 1) {

            printf("Invalid input!\n");

            while (getchar() != '\n');

            continue;
        }

        switch (choice) {

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
                printf("Thank you for using the banking system!\n");
                exit(0);

            default:
                printf("Invalid choice!\n");
        }
    }

    return 0;
}

void createAccount() {

    struct Account acc, temp;

    FILE *fp;

    fp = fopen("bank.dat", "ab+");

    if (fp == NULL) {

        printf("Unable to open file!\n");
        return;
    }

    printf("Enter Account Number: ");
    scanf("%19s", acc.accNo);

    if (strlen(acc.accNo) < 6) {

        printf("Account number must be at least 6 digits!\n");

        fclose(fp);
        return;
    }

    rewind(fp);

    while (fread(&temp, sizeof(temp), 1, fp)) {

        if (strcmp(temp.accNo, acc.accNo) == 0) {

            printf("Account already exists!\n");

            fclose(fp);
            return;
        }
    }

    printf("Enter Name: ");
    scanf(" %49[^\n]", acc.name);

    printf("Enter Initial Balance: ");
    scanf("%lf", &acc.balance);

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

    char accNo[20];

    double amount;

    int found = 0;

    fp = fopen("bank.dat", "rb+");

    if (fp == NULL) {

        printf("No account file found!\n");
        return;
    }

    printf("Enter Account Number: ");
    scanf("%19s", accNo);

    while (fread(&acc, sizeof(acc), 1, fp)) {

        if (strcmp(acc.accNo, accNo) == 0) {

            printf("Enter amount to deposit: ");
            scanf("%lf", &amount);

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

    if (!found) {

        printf("Account not found!\n");
    }

    fclose(fp);
}

void withdrawMoney() {

    struct Account acc;

    FILE *fp;

    char accNo[20];

    double amount;

    int found = 0;

    fp = fopen("bank.dat", "rb+");

    if (fp == NULL) {

        printf("No account file found!\n");
        return;
    }

    printf("Enter Account Number: ");
    scanf("%19s", accNo);

    while (fread(&acc, sizeof(acc), 1, fp)) {

        if (strcmp(acc.accNo, accNo) == 0) {

            printf("Enter amount to withdraw: ");
            scanf("%lf", &amount);

            if (amount <= 0) {

                printf("Invalid amount!\n");

                fclose(fp);
                return;
            }

            if (amount > acc.balance - 500) {

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

    if (!found) {

        printf("Account not found!\n");
    }

    fclose(fp);
}

void checkBalance() {

    struct Account acc;

    FILE *fp;

    char accNo[20];

    int found = 0;

    fp = fopen("bank.dat", "rb");

    if (fp == NULL) {

        printf("No account file found!\n");
        return;
    }

    printf("Enter Account Number: ");
    scanf("%19s", accNo);

    while (fread(&acc, sizeof(acc), 1, fp)) {

        if (strcmp(acc.accNo, accNo) == 0) {

            printf("\n===== ACCOUNT DETAILS =====\n");

            printf("Account Number : %s\n", acc.accNo);
            printf("Account Holder : %s\n", acc.name);
            printf("Current Balance: %.2lf\n", acc.balance);

            found = 1;

            break;
        }
    }

    if (!found) {

        printf("Account not found!\n");
    }

    fclose(fp);
}

//update banking system code
