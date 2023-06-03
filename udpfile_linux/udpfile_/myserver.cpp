#include<iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<unistd.h>
#include<cstring>

#define BUF_SIZE 1024

using namespace std;

void parseCommandLine(const string& commandLine, string& ipAddress, int& port) {
    size_t firstSpace = commandLine.find(' ');
    if (firstSpace != string::npos) {
        ipAddress = commandLine.substr(0, firstSpace);
        string portStr = commandLine.substr(firstSpace + 1);
        port = stoi(portStr);
    }
}

int main() {
    cout << "Input: local-ip local-port" << endl;
    string commandLine;
    getline(cin, commandLine);
    string ipAddress;
    int port;

    parseCommandLine(commandLine, ipAddress, port);
    cout << "ipAddress: " << ipAddress << endl;
    cout << "port: " << port << endl;

    // Create socket
    int server = socket(AF_INET, SOCK_DGRAM, 0);
    if (server < 0) {
        cout << "Failed to create socket." << endl;
        return -1;
    }

    // Set up server address
    sockaddr_in my_addr, remote_addr;
    socklen_t nAddrlen = sizeof(remote_addr);
    memset((char*)&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ipAddress.c_str(), &(my_addr.sin_addr)) <= 0) {
        cout << "Invalid address." << endl;
        return -1;
    }

    // Bind socket to address
    if (bind(server, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0) {
        cout << "Bind error!" << endl;
        return -1;
    }

    // Receive file name
    char recvBuff[BUF_SIZE];
    memset(recvBuff, 0, BUF_SIZE);
    int ret = recvfrom(server, recvBuff, BUF_SIZE, 0, (struct sockaddr*)&remote_addr, &nAddrlen);
    if (ret < 0) {
        cout << "Failed to receive file name." << endl;
        return -1;
    }
    string fileName = recvBuff;
    cout << "Filename: " << fileName << endl;

    // Create file
    FILE* fp = fopen(fileName.c_str(), "wb");
    if (!fp) {
        cout << "Failed to create file." << endl;
        return -1;
    }

    // Receive file content
    while (true) {
        memset(recvBuff, 0, BUF_SIZE);
        ret = recvfrom(server, recvBuff, BUF_SIZE, 0, (struct sockaddr*)&remote_addr, &nAddrlen);
        if (ret < 0) {
            cout << "Failed to receive file content." << endl;
            break;
        }
        if (strcmp(recvBuff, "end") == 0) {
            cout << "File received successfully!" << endl;
            break;
        }
        size_t length = ret;
        size_t written = fwrite(recvBuff, 1, length, fp);
        if (written != length) {
            cout << "Failed to write to file." << endl;
            break;
        }
        sendto(server, "success", sizeof("success"), 0, (struct sockaddr*)&remote_addr, nAddrlen);
    }

    fclose(fp);
    close(server);
    return 0;
}
