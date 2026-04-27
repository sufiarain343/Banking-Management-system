#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Structures
typedef struct {
    int day;
    int month;
    int year;
} Date;

typedef struct {
    int accountNo;
    char name[100];
    char phone[15];
    char email[50];
    float balance;
    Date openingDate;
    char accountType[20];
    char status[10];
} Account;

Account accounts[1000];
int totalAccounts = 0;
char filename[] = "data/accounts.txt";

// Function to get current date
void getCurrentDate(Date *date) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    date->day = tm->tm_mday;
    date->month = tm->tm_mon + 1;
    date->year = tm->tm_year + 1900;
}

// Save to file
void saveToFile() {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error saving data!\n");
        return;
    }
    fprintf(file, "%d\n", totalAccounts);
    for (int i = 0; i < totalAccounts; i++) {
        fprintf(file, "%d|%s|%s|%s|%.2f|%d|%d|%d|%s|%s\n",
                accounts[i].accountNo,
                accounts[i].name,
                accounts[i].phone,
                accounts[i].email,
                accounts[i].balance,
                accounts[i].openingDate.day,
                accounts[i].openingDate.month,
                accounts[i].openingDate.year,
                accounts[i].accountType,
                accounts[i].status);
    }
    fclose(file);
}

// Load from file
void loadFromFile() {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        totalAccounts = 0;
        return;
    }
    fscanf(file, "%d\n", &totalAccounts);
    for (int i = 0; i < totalAccounts; i++) {
        fscanf(file, "%d|%[^|]|%[^|]|%[^|]|%f|%d|%d|%d|%[^|]|%[^\n]\n",
               &accounts[i].accountNo,
               accounts[i].name,
               accounts[i].phone,
               accounts[i].email,
               &accounts[i].balance,
               &accounts[i].openingDate.day,
               &accounts[i].openingDate.month,
               &accounts[i].openingDate.year,
               accounts[i].accountType,
               accounts[i].status);
    }
    fclose(file);
}

// Generate account number
int generateAccountNo() {
    if (totalAccounts == 0) return 1001;
    int max = accounts[0].accountNo;
    for (int i = 1; i < totalAccounts; i++) {
        if (accounts[i].accountNo > max) max = accounts[i].accountNo;
    }
    return max + 1;
}

// Find account by number
int findAccount(int accNo) {
    for (int i = 0; i < totalAccounts; i++) {
        if (accounts[i].accountNo == accNo) return i;
    }
    return -1;
}

// Clear screen
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Press any key
void pressAnyKey() {
    printf("\nPress Enter to continue...");
    getchar();
    getchar();
}

// Create account
void createAccount() {
    clearScreen();
    printf("\n========== CREATE NEW ACCOUNT ==========\n\n");
    
    Account newAcc;
    newAcc.accountNo = generateAccountNo();
    newAcc.balance = 0;
    strcpy(newAcc.status, "Active");
    getCurrentDate(&newAcc.openingDate);
    
    printf("Account Number will be: %d\n\n", newAcc.accountNo);
    
    printf("Enter Name: ");
    fgets(newAcc.name, 100, stdin);
    newAcc.name[strcspn(newAcc.name, "\n")] = 0;
    
    printf("Enter Phone: ");
    fgets(newAcc.phone, 15, stdin);
    newAcc.phone[strcspn(newAcc.phone, "\n")] = 0;
    
    printf("Enter Email: ");
    fgets(newAcc.email, 50, stdin);
    newAcc.email[strcspn(newAcc.email, "\n")] = 0;
    
    printf("Account Type (Savings/Current): ");
    fgets(newAcc.accountType, 20, stdin);
    newAcc.accountType[strcspn(newAcc.accountType, "\n")] = 0;
    
    printf("Initial Deposit: ₹");
    scanf("%f", &newAcc.balance);
    getchar();
    
    accounts[totalAccounts] = newAcc;
    totalAccounts++;
    saveToFile();
    
    printf("\n✅ ACCOUNT CREATED SUCCESSFULLY!\n");
    printf("   Account Number: %d\n", newAcc.accountNo);
    pressAnyKey();
}

// Deposit money
void depositMoney() {
    clearScreen();
    printf("\n========== DEPOSIT MONEY ==========\n\n");
    
    int accNo;
    float amount;
    
    printf("Enter Account Number: ");
    scanf("%d", &accNo);
    getchar();
    
    int index = findAccount(accNo);
    if (index == -1) {
        printf("❌ Account not found!\n");
        pressAnyKey();
        return;
    }
    
    printf("Account Holder: %s\n", accounts[index].name);
    printf("Current Balance: ₹%.2f\n", accounts[index].balance);
    printf("Enter Amount to Deposit: ₹");
    scanf("%f", &amount);
    getchar();
    
    if (amount <= 0) {
        printf("❌ Invalid amount!\n");
        pressAnyKey();
        return;
    }
    
    accounts[index].balance += amount;
    saveToFile();
    
    printf("\n✅ ₹%.2f deposited successfully!\n", amount);
    printf("   New Balance: ₹%.2f\n", accounts[index].balance);
    pressAnyKey();
}

// Withdraw money
void withdrawMoney() {
    clearScreen();
    printf("\n========== WITHDRAW MONEY ==========\n\n");
    
    int accNo;
    float amount;
    
    printf("Enter Account Number: ");
    scanf("%d", &accNo);
    getchar();
    
    int index = findAccount(accNo);
    if (index == -1) {
        printf("❌ Account not found!\n");
        pressAnyKey();
        return;
    }
    
    printf("Account Holder: %s\n", accounts[index].name);
    printf("Current Balance: ₹%.2f\n", accounts[index].balance);
    printf("Enter Amount to Withdraw: ₹");
    scanf("%f", &amount);
    getchar();
    
    if (amount <= 0) {
        printf("❌ Invalid amount!\n");
        pressAnyKey();
        return;
    }
    
    if (amount > accounts[index].balance) {
        printf("❌ Insufficient balance!\n");
        pressAnyKey();
        return;
    }
    
    accounts[index].balance -= amount;
    saveToFile();
    
    printf("\n✅ ₹%.2f withdrawn successfully!\n", amount);
    printf("   New Balance: ₹%.2f\n", accounts[index].balance);
    pressAnyKey();
}

// Check balance
void checkBalance() {
    clearScreen();
    printf("\n========== CHECK BALANCE ==========\n\n");
    
    int accNo;
    
    printf("Enter Account Number: ");
    scanf("%d", &accNo);
    getchar();
    
    int index = findAccount(accNo);
    if (index == -1) {
        printf("❌ Account not found!\n");
        pressAnyKey();
        return;
    }
    
    printf("\n═══════════════════════════════\n");
    printf("Account Holder: %s\n", accounts[index].name);
    printf("Account Number: %d\n", accounts[index].accountNo);
    printf("Account Type: %s\n", accounts[index].accountType);
    printf("Current Balance: ₹%.2f\n", accounts[index].balance);
    printf("═══════════════════════════════\n");
    pressAnyKey();
}

// View all accounts
void viewAllAccounts() {
    clearScreen();
    printf("\n========== ALL ACCOUNTS ==========\n\n");
    
    if (totalAccounts == 0) {
        printf("No accounts found!\n");
        pressAnyKey();
        return;
    }
    
    printf("+----+------------------+---------------+------------------+\n");
    printf("| No | Account Number   | Name          | Balance (₹)      |\n");
    printf("+----+------------------+---------------+------------------+\n");
    
    for (int i = 0; i < totalAccounts; i++) {
        printf("| %-2d | %-16d | %-13s | ₹%-14.2f |\n",
               i+1,
               accounts[i].accountNo,
               accounts[i].name,
               accounts[i].balance);
    }
    printf("+----+------------------+---------------+------------------+\n");
    printf("\nTotal Accounts: %d\n", totalAccounts);
    pressAnyKey();
}

// Delete account
void deleteAccount() {
    clearScreen();
    printf("\n========== DELETE ACCOUNT ==========\n\n");
    
    int accNo;
    
    printf("Enter Account Number: ");
    scanf("%d", &accNo);
    getchar();
    
    int index = findAccount(accNo);
    if (index == -1) {
        printf("❌ Account not found!\n");
        pressAnyKey();
        return;
    }
    
    printf("\n⚠️  WARNING: This will delete account of %s\n", accounts[index].name);
    printf("   Balance: ₹%.2f\n", accounts[index].balance);
    printf("\nAre you sure? (y/n): ");
    char confirm;
    scanf("%c", &confirm);
    getchar();
    
    if (tolower(confirm) == 'y') {
        for (int i = index; i < totalAccounts - 1; i++) {
            accounts[i] = accounts[i + 1];
        }
        totalAccounts--;
        saveToFile();
        printf("\n✅ Account deleted successfully!\n");
    } else {
        printf("\n❌ Deletion cancelled.\n");
    }
    pressAnyKey();
}

// Main menu
void showMenu() {
    printf("\n");
    printf("╔════════════════════════════════════╗\n");
    printf("║     BANK MANAGEMENT SYSTEM         ║\n");
    printf("╠════════════════════════════════════╣\n");
    printf("║  1. Create Account                 ║\n");
    printf("║  2. Deposit Money                  ║\n");
    printf("║  3. Withdraw Money                 ║\n");
    printf("║  4. Check Balance                  ║\n");
    printf("║  5. View All Accounts              ║\n");
    printf("║  6. Delete Account                 ║\n");
    printf("║  7. Exit                           ║\n");
    printf("╚════════════════════════════════════╝\n");
    printf("\nEnter your choice: ");
}

// Main function
int main() {
    loadFromFile();
    int choice;
    
    while (1) {
        showMenu();
        scanf("%d", &choice);
        getchar();
        
        switch(choice) {
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
                viewAllAccounts();
                break;
            case 6:
                deleteAccount();
                break;
            case 7:
                printf("\nThank you for using Bank Management System!\n");
                return 0;
            default:
                printf("\n❌ Invalid choice! Please try again.\n");
                pressAnyKey();
        }
    }
    return 0;
}