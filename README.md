# C Banking System with MySQL

A console-based Banking Management System developed in **C** with **MySQL database integration**. The project demonstrates database connectivity, account management, transaction processing, and structured programming in C.

## Features

- Create New Bank Account
- Deposit Money
- Withdraw Money
- Check Account Balance
- Transfer Money Between Accounts
- Modify Account Details
- Delete Bank Account
- View Transaction History
- List All Accounts
- MySQL Database Integration
- Input Validation and Error Handling

## Technologies Used

- **Programming Language:** C
- **Database:** MySQL
- **Database Connector:** MariaDB Connector/C
- **Compiler:** GCC
- **Development Environment:** Visual Studio Code

## Concepts Used

- Structures and Functions
- Modular Programming
- MySQL Database Connectivity
- SQL Queries and Database Operations
- Account and Transaction Management
- Input Validation and Error Handling

## Prerequisites

Before running the project, make sure you have:

- GCC compiler installed
- MySQL Server installed and running
- MariaDB Connector/C installed
- A configured database and the required tables

## Database Configuration

The application connects to the MySQL database named `bank_system_v2`.

Before running the program:

1. Start your MySQL server.
2. Create the required database and tables.
3. Configure the database connection details in the source code.
4. Make sure the configured MySQL user has the necessary database permissions.

> **Note:** Do not publish real database passwords or other credentials in a public repository.

## How to Compile

The following example uses GCC and MariaDB Connector/C on Windows with MSYS2 UCRT64.

Open the terminal in the project directory and run:

```bash
gcc banking_system_mysql.c -IC:/msys64/ucrt64/include/mariadb -LC:/msys64/ucrt64/lib -lmariadb -o banking_system_mysql.exe
```

Update the include and library paths if your connector is installed in a different location.

## How to Run

On Windows, run:

```powershell
.\banking_system_mysql.exe
```

Make sure the MySQL server is running and the database connection is configured correctly.

## Main Menu

The application provides the following operations:

1. Create Account
2. Deposit Money
3. Withdraw Money
4. Check Balance
5. Transfer Money
6. Delete Account
7. Modify Account Details
8. Transaction History
9. List All Accounts
10. Exit

## Project Structure

```text
Banking-System-in-C/
├── banking_system_mysql.c
├── README.md
└── .gitignore
```

## Learning Outcomes

This project provides practical experience with:

- Connecting a C application to a MySQL database
- Executing database operations from C
- Managing bank accounts and transactions
- Organizing code using functions and structures
- Handling user input and database errors
- Compiling and running C applications with external libraries

## Author

**Aagam Jain**

GitHub: [aagamjain137](https://github.com/aagamjain137)
