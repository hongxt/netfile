//
// Created by Hongchen on 2023/6/3.
//
#include<iostream>
#include<WinSock2.h>
#include<winsock.h>
//#pragma comment(lib,"ws2_32.lib")
#define BUF_SIZE 1024
//#define PATH_LENGTH 20
using namespace std;
//char sendBuff[BUF_SIZE];
char recvBuff[BUF_SIZE];
//char fileName[PATH_LENGTH];

void parseCommandLine(const string& commandLine, string& ipAddress, int& port) {
    size_t firstSpace = commandLine.find(' ');
    if (firstSpace != string::npos) {
        ipAddress = commandLine.substr(0, firstSpace);
        string portStr = commandLine.substr(firstSpace + 1);
        port = stoi(portStr);
    }
}


int main() {
    //1 参数输入
    cout<<"Input:local-ip local-port"<<endl;
    string commandLine;
    getline(cin, commandLine);
    string ipAddress;
    int port;

    parseCommandLine(commandLine,ipAddress,port);
    cout<<"ipAddress:"<<ipAddress<<endl;
    cout<<"port:"<<port<<endl;
    //2 网络socket初始化
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "Initialization failed." << endl;
        return -1;
    }
    SOCKET server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server == -1) {
        cout << "Socket failed." << endl;
        return -1;
    }
    sockaddr_in my_addr, remote_addr;
    int nAddrlen = sizeof(remote_addr);
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.S_un.S_addr = inet_addr(ipAddress.c_str());;
    if (::bind(server, (sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR) {
        cout << "Bind error!" << endl;
        return -1;
    }
    //3 接收文件建立
    string fileName;
    int ret = recvfrom(server, recvBuff, BUF_SIZE, 0, (sockaddr*)&remote_addr, &nAddrlen);
    if (ret < 0) {
        cout << "Receive name failed." << endl;
        return -1;
    }
    fileName= recvBuff;
    cout << "Filename: " << fileName << endl;

    errno_t err;
    FILE *fp;
    if ((err = fopen_s(&fp, fileName.c_str(), "wb")) < 0) {
        cout << "Create failed." << endl;
        return -1;
    }
    int length;
    memset(recvBuff, 0, BUF_SIZE);
    while ((length = recvfrom(server, recvBuff, BUF_SIZE, 0, (sockaddr*)&remote_addr, &nAddrlen))) {
//        cout<<"length:"<<length<<endl;
        if (strcmp(recvBuff, "end")==0){
            cout<<"recvBuff:"<<recvBuff<<endl;
            break;
        }//接收结束信息
        if (length == 0) {
            cout << "An error occurred while receiving." << endl;
            return -1;
        }
        int ret = fwrite(recvBuff, 1, length, fp);
        if (ret < length) {
            cout << "Write failed." << endl;
            return -1;
        }
        memset(recvBuff, 0, BUF_SIZE);
        sendto(server, "success", sizeof("success") + 1, 0, (SOCKADDR*)&remote_addr, sizeof(remote_addr));
    }
    cout << "Successfully received!" << endl;
    fclose(fp);
    closesocket(server);
    WSACleanup();
    return 0;
}