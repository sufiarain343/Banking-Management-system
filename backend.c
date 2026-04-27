#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

typedef struct {
    int accNo;
    char name[100];
    char cnic[20];
    char mobile[15];
    char email[100];
    char accType[20];
    float balance;
    char status[10];
} Account;

typedef struct {
    int accNo;
    char type[20];     // "Deposit", "Withdraw", "Account Created", "Account Closed"
    float amount;
    float balanceAfter;
    char date[30];
} Transaction;

Account accounts[1000];
Transaction transactions[10000];
int total = 0;
int transCount = 0;

// ================= FILE HANDLING =================
void load() {
    FILE *f = fopen("accounts.txt", "r");
    if (!f) return;
    fscanf(f, "%d\n", &total);
    for(int i = 0; i < total; i++) {
        fscanf(f, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%f|%[^\n]\n", 
               &accounts[i].accNo, accounts[i].name, accounts[i].cnic,
               accounts[i].mobile, accounts[i].email, accounts[i].accType,
               &accounts[i].balance, accounts[i].status);
    }
    fclose(f);
    
    // Load transactions
    FILE *ft = fopen("transactions.txt", "r");
    if (!ft) return;
    fscanf(ft, "%d\n", &transCount);
    for(int i = 0; i < transCount; i++) {
        fscanf(ft, "%d|%[^|]|%f|%f|%[^\n]\n", 
               &transactions[i].accNo, transactions[i].type,
               &transactions[i].amount, &transactions[i].balanceAfter,
               transactions[i].date);
    }
    fclose(ft);
}

void save() {
    FILE *f = fopen("accounts.txt", "w");
    fprintf(f, "%d\n", total);
    for(int i = 0; i < total; i++) {
        fprintf(f, "%d|%s|%s|%s|%s|%s|%.2f|%s\n", 
                accounts[i].accNo, accounts[i].name, accounts[i].cnic,
                accounts[i].mobile, accounts[i].email, accounts[i].accType,
                accounts[i].balance, accounts[i].status);
    }
    fclose(f);
}

void saveTransactions() {
    FILE *ft = fopen("transactions.txt", "w");
    fprintf(ft, "%d\n", transCount);
    for(int i = 0; i < transCount; i++) {
        fprintf(ft, "%d|%s|%.2f|%.2f|%s\n", 
                transactions[i].accNo, transactions[i].type,
                transactions[i].amount, transactions[i].balanceAfter,
                transactions[i].date);
    }
    fclose(ft);
}

// ================= ADD TRANSACTION =================
void addTransaction(int accNo, const char* type, float amount, float balanceAfter) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    char date[30];
    sprintf(date, "%02d/%02d/%d %02d:%02d:%02d",
            local->tm_mday, local->tm_mon + 1, local->tm_year + 1900,
            local->tm_hour, local->tm_min, local->tm_sec);
    
    transactions[transCount].accNo = accNo;
    strcpy(transactions[transCount].type, type);
    transactions[transCount].amount = amount;
    transactions[transCount].balanceAfter = balanceAfter;
    strcpy(transactions[transCount].date, date);
    transCount++;
    saveTransactions();
}

// ================= HELPERS =================
int genAccNo() {
    if(total == 0) return 1001;
    int max = accounts[0].accNo;
    for(int i = 1; i < total; i++)
        if(accounts[i].accNo > max) max = accounts[i].accNo;
    return max + 1;
}

int findAcc(int accNo) {
    for(int i = 0; i < total; i++)
        if(accounts[i].accNo == accNo && strcmp(accounts[i].status, "Active") == 0) 
            return i;
    return -1;
}

// ================= RESPONSE =================
void sendResponse(SOCKET client, const char* json) {
    char response[32768];
    sprintf(response,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Length: %d\r\n"
        "\r\n%s",
        (int)strlen(json), json);
    send(client, response, strlen(response), 0);
}

// ================= REQUEST HANDLER =================
void handle(SOCKET client, char* req) {
    // Handle CORS preflight
    if (strncmp(req, "OPTIONS", 7) == 0) {
        char response[] =
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
        send(client, response, strlen(response), 0);
        return;
    }

    char method[10], path[256];
    sscanf(req, "%s %s", method, path);

    printf("Request: %s %s\n", method, path);

    // ================= GET ALL ACCOUNTS =================
    if(strcmp(method, "GET") == 0 && strcmp(path, "/api/accounts") == 0) {
        char json[65536] = "{\"accounts\":[";

        for(int i = 0; i < total; i++) {
            if(strcmp(accounts[i].status, "Active") != 0) continue;
            
            char acc[4096];
            sprintf(acc,
                "{\"accountNo\":%d,\"name\":\"%s\",\"cnic\":\"%s\",\"mobile\":\"%s\","
                "\"email\":\"%s\",\"accType\":\"%s\",\"balance\":%.2f,\"status\":\"%s\"}",
                accounts[i].accNo, accounts[i].name, accounts[i].cnic,
                accounts[i].mobile, accounts[i].email, accounts[i].accType,
                accounts[i].balance, accounts[i].status);

            strcat(json, acc);
            if(i < total-1) strcat(json, ",");
        }

        strcat(json, "]}");
        sendResponse(client, json);
    }

    // ================= GET TRANSACTION HISTORY =================
    else if(strcmp(method, "GET") == 0 && strncmp(path, "/api/history", 12) == 0) {
        int accNo = 0;
        char* q = strstr(path, "?account=");
        if(q) accNo = atoi(q + 9);
        
        char json[131072] = "{\"transactions\":[";
        int first = 1;
        
        for(int i = transCount - 1; i >= 0; i--) {
            if(transactions[i].accNo == accNo) {
                if(!first) strcat(json, ",");
                char trans[1024];
                sprintf(trans,
                    "{\"type\":\"%s\",\"amount\":%.2f,\"balanceAfter\":%.2f,\"date\":\"%s\"}",
                    transactions[i].type, transactions[i].amount, 
                    transactions[i].balanceAfter, transactions[i].date);
                strcat(json, trans);
                first = 0;
            }
        }
        strcat(json, "]}");
        sendResponse(client, json);
    }

    // ================= CREATE ACCOUNT =================
    else if(strcmp(method, "POST") == 0 && strcmp(path, "/api/accounts") == 0) {
        char* body = strstr(req, "\r\n\r\n");
        if(body) {
            body += 4;
            
            char name[100] = "", cnic[20] = "", mobile[15] = "", email[100] = "", accType[20] = "";
            float deposit = 0;

            char* n = strstr(body, "\"name\":\"");
            if(n) { n += 8; char* e = strstr(n, "\""); if(e) { strncpy(name, n, e-n); name[e-n]=0; } }
            
            char* c = strstr(body, "\"cnic\":\"");
            if(c) { c += 8; char* e = strstr(c, "\""); if(e) { strncpy(cnic, c, e-c); cnic[e-c]=0; } }
            
            char* m = strstr(body, "\"mobile\":\"");
            if(m) { m += 9; char* e = strstr(m, "\""); if(e) { strncpy(mobile, m, e-m); mobile[e-m]=0; } }
            
            char* e = strstr(body, "\"email\":\"");
            if(e) { e += 8; char* ep = strstr(e, "\""); if(ep) { strncpy(email, e, ep-e); email[ep-e]=0; } }
            
            char* a = strstr(body, "\"accType\":\"");
            if(a) { a += 10; char* ep = strstr(a, "\""); if(ep) { strncpy(accType, a, ep-a); accType[ep-a]=0; } }
            
            char* d = strstr(body, "\"deposit\":");
            if(d) deposit = atof(d + 10);

            if(strlen(name) == 0 || strlen(cnic) == 0) {
                sendResponse(client, "{\"success\":false,\"message\":\"Name and CNIC required\"}");
                return;
            }

            Account newAcc;
            newAcc.accNo = genAccNo();
            strcpy(newAcc.name, name);
            strcpy(newAcc.cnic, cnic);
            strcpy(newAcc.mobile, mobile);
            strcpy(newAcc.email, email);
            strcpy(newAcc.accType, strlen(accType) > 0 ? accType : "Savings");
            newAcc.balance = deposit;
            strcpy(newAcc.status, "Active");

            accounts[total++] = newAcc;
            save();
            
            // Add transaction history
            addTransaction(newAcc.accNo, "Account Created", deposit, deposit);

            char json[512];
            sprintf(json, "{\"success\":true,\"accountNo\":%d,\"message\":\"Account created successfully\"}", newAcc.accNo);
            sendResponse(client, json);
        }
    }

    // ================= DEPOSIT =================
    else if(strcmp(method, "POST") == 0 && strcmp(path, "/api/deposit") == 0) {
        char* body = strstr(req, "\r\n\r\n");
        if(body) {
            body += 4;
            int accNo = 0;
            float amount = 0;
            sscanf(body, "{\"accountNo\":%d,\"amount\":%f}", &accNo, &amount);

            int idx = findAcc(accNo);
            if(idx != -1 && amount > 0) {
                accounts[idx].balance += amount;
                save();
                addTransaction(accNo, "Deposit", amount, accounts[idx].balance);
                
                char json[256];
                sprintf(json, "{\"success\":true,\"newBalance\":%.2f,\"message\":\"Deposit successful\"}", accounts[idx].balance);
                sendResponse(client, json);
            } else {
                sendResponse(client, "{\"success\":false,\"message\":\"Account not found\"}");
            }
        }
    }

    // ================= WITHDRAW =================
    else if(strcmp(method, "POST") == 0 && strcmp(path, "/api/withdraw") == 0) {
        char* body = strstr(req, "\r\n\r\n");
        if(body) {
            body += 4;
            int accNo = 0;
            float amount = 0;
            sscanf(body, "{\"accountNo\":%d,\"amount\":%f}", &accNo, &amount);

            int idx = findAcc(accNo);
            if(idx != -1 && amount > 0 && accounts[idx].balance >= amount) {
                accounts[idx].balance -= amount;
                save();
                addTransaction(accNo, "Withdraw", amount, accounts[idx].balance);
                
                char json[256];
                sprintf(json, "{\"success\":true,\"newBalance\":%.2f,\"message\":\"Withdrawal successful\"}", accounts[idx].balance);
                sendResponse(client, json);
            } else if(idx != -1 && accounts[idx].balance < amount) {
                sendResponse(client, "{\"success\":false,\"message\":\"Insufficient balance\"}");
            } else {
                sendResponse(client, "{\"success\":false,\"message\":\"Account not found\"}");
            }
        }
    }

    // ================= CHECK BALANCE =================
    else if(strcmp(method, "GET") == 0 && strncmp(path, "/api/balance", 12) == 0) {
        int accNo = 0;
        char* q = strstr(path, "?account=");
        if(q) accNo = atoi(q + 9);

        int idx = findAcc(accNo);
        if(idx != -1) {
            char json[1024];
            sprintf(json,
                "{\"success\":true,\"accountNo\":%d,\"name\":\"%s\",\"cnic\":\"%s\","
                "\"mobile\":\"%s\",\"email\":\"%s\",\"accType\":\"%s\",\"balance\":%.2f,\"status\":\"%s\"}",
                accounts[idx].accNo, accounts[idx].name, accounts[idx].cnic,
                accounts[idx].mobile, accounts[idx].email, accounts[idx].accType,
                accounts[idx].balance, accounts[idx].status);
            sendResponse(client, json);
        } else {
            sendResponse(client, "{\"success\":false,\"message\":\"Account not found\"}");
        }
    }

    // ================= DELETE ACCOUNT =================
    else if(strcmp(method, "POST") == 0 && strcmp(path, "/api/delete") == 0) {
        char* body = strstr(req, "\r\n\r\n");
        if(body) {
            body += 4;
            int accNo = 0;
            sscanf(body, "{\"accountNo\":%d}", &accNo);

            int idx = findAcc(accNo);
            if(idx != -1) {
                float finalBalance = accounts[idx].balance;
                strcpy(accounts[idx].status, "Closed");
                save();
                addTransaction(accNo, "Account Closed", finalBalance, 0);
                
                sendResponse(client, "{\"success\":true,\"message\":\"Account closed successfully\"}");
            } else {
                sendResponse(client, "{\"success\":false,\"message\":\"Account not found\"}");
            }
        }
    }

    else {
        sendResponse(client, "{\"error\":\"Not found\"}");
    }
}

// ================= MAIN =================
int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    load();

    printf("\n========================================\n");
    printf("  C BACKEND SERVER RUNNING!\n");
    printf("  Port: %d\n", PORT);
    printf("========================================\n\n");

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if(bind(server, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("ERROR: Port %d is busy!\n", PORT);
        return 1;
    }

    listen(server, 5);
    printf("Server READY!\n");
    printf("Test: http://localhost:%d/api/accounts\n\n", PORT);

    while(1) {
        struct sockaddr_in client;
        int len = sizeof(client);
        SOCKET clientSocket = accept(server, (struct sockaddr*)&client, &len);

        if(clientSocket != INVALID_SOCKET) {
            char buffer[32768];
            int bytes = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
            buffer[bytes] = '\0';
            handle(clientSocket, buffer);
            closesocket(clientSocket);
        }
    }

    closesocket(server);
    WSACleanup();
    return 0;
}