#include<iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<unistd.h>
#include<cstring>
#include <chrono>
#define BUF_SIZE 1024

void parseCommandLine(const std::string& commandLine, std::string& ipAddress, int& port, std::string& fileName) {
    size_t firstSpace = commandLine.find(' ');
    if (firstSpace != std::string::npos) {
        ipAddress = commandLine.substr(0, firstSpace);

        size_t secondSpace = commandLine.find(' ', firstSpace + 1);
        if (secondSpace != std::string::npos) {
            std::string portStr = commandLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
            port = std::stoi(portStr);

            fileName = commandLine.substr(secondSpace + 1);
        }
    }
}

int main() {
    std::cout << "Input: target-ip target-port localfile-tosend" << std::endl;
    std::string commandLine;
    std::getline(std::cin, commandLine);

    std::string ipAddress;
    int port;
    std::string fileName;

    parseCommandLine(commandLine, ipAddress, port, fileName);

    // 1. Create socket
    int client = socket(AF_INET, SOCK_DGRAM, 0);
    if (client == -1) {
        std::cout << "Failed to create socket." << std::endl;
        return -1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ipAddress.c_str(), &(serverAddr.sin_addr)) <= 0) {
        std::cout << "Invalid target IP address." << std::endl;
        return -1;
    }

    // 2. Open file
    FILE* fp = fopen(fileName.c_str(), "rb");
    if (!fp) {
        std::cout << "Failed to open file." << std::endl;
        close(client);
        return -1;
    }

    // 3.1 Send file name
    sendto(client, fileName.c_str(), strlen(fileName.c_str()), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    // 3.2 Send file
    char sendBuff[BUF_SIZE];
    int length;
    int ret;
    auto startTime = std::chrono::high_resolution_clock::now();
    while ((length = fread(sendBuff, 1, BUF_SIZE, fp)) > 0) {
        ret = sendto(client, sendBuff, length, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (ret == -1) {
            std::cout << "An error occurred while sending." << std::endl;
            fclose(fp);
            close(client);
            return -1;
        }

        char recvBuff[BUF_SIZE];
        socklen_t serverAddrLen = sizeof(serverAddr);
        ret = recvfrom(client, recvBuff, BUF_SIZE, 0, (struct sockaddr*)&serverAddr, &serverAddrLen);
        if (ret == -1) {
            std::cout << "Failed to receive." << std::endl;
            fclose(fp);
            close(client);
            return -1;
        }
        else {
            if (strcmp(recvBuff, "success") != 0) {
                std::cout << "Failed to receive." << std::endl;
                fclose(fp);
                close(client);
                return -1;
            }
        }
        memset(sendBuff, 0, BUF_SIZE);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "send time: " << duration / 1000 << " s" << std::endl;

    // 4. Send file end flag
    const char* endFlag = "end";
    length = strlen(endFlag);
    ret = sendto(client, endFlag, length, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret == -1) {
        std::cout << "An error occurred while sending file end flag." << std::endl;
    }

    std::cout << "Successfully sent!" << std::endl;
    std::cout << "---------------------OVER SENDING...---------------------" << std::endl;

    fclose(fp);
    close(client);
    return 0;
}
