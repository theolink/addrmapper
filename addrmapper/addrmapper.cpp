#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

#ifdef _WIN32
#include "getopt.h"
#include <winsock2.h>
#include <ws2tcpip.h>	
#else
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>		
#include <netinet/in.h>	
#include <arpa/inet.h>
#include <ifaddrs.h>
#endif // _WIN32

using namespace std;

#ifndef _WIN32
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#endif // _WIN32

#define SOCK_BUFF_SIZE          32*1024         //socket send and recv buffer size
#define BUFF_SIZE               5*1024          //domain-address map size
#define DEFAULT_SERVER_PORT     12880           //default port
#define IPV6_ADDR_MAX_LEN       50
#define DEFAULT_TRANSFER_KEY    "tiansi"
#define MAPS_FILE_PATH          "./maps.dat"

#ifdef _WIN32
#define HOSTS_FILE_PATH         "C:/Windows/System32/drivers/etc/hosts"       //just for test
#else
#define HOSTS_FILE_PATH         "/etc/hosts" 
#endif // _WIN32




typedef struct {
    int sock;
    sockaddr_in addr;
}server_t, client_t;

vector<string> split(const char* str, char c) {
    string source = str;
    vector<string> results;
    size_t index = source.find(c);
    int last = 0;
    while (index != source.npos) {
        string tmp = source.substr(0, index);
        results.push_back(tmp);
        source = source.substr(index + 1, source.size());
        index = source.find(c);
    }
    results.push_back(source.substr(0, source.size()));
    return results;

}

vector<string> split(const string str, char c) {
    string source = str;
    vector<string> results;
    size_t index = source.find(c);
    int last = 0;
    while (index != source.npos) {
        string tmp = source.substr(0, index);
        results.push_back(tmp);
        source = source.substr(index + 1, source.size());
        index = source.find(c);
    }
    results.push_back(source.substr(0, source.size()));
    return results;

}

void trim(char* strIn, char* strOut, char c) {
    int i, j;
    i = 0;
    j = strlen(strIn) - 1;

    while (strIn[i] == c)
        ++i;
    while (strIn[j] == c)
        --j;
    strncpy(strOut, strIn + i, j - i + 1);
    strOut[j - i + 1] = '\0';
}

int firstNot(char* str, char c) {
    for (unsigned int i = 0; i < strlen(str); i++) {
        if (str[i] != c)
            return i;
    }
    return 0;
}

string chars2string(const char* c) {
    string str;
    for (int i = 0; c[i] != '\0'; i++)
    {
        str += c[i];
    }
    return str;
}

void writeMaps(string domain, string addr) {
    ifstream in;
    char line[1024] = { '\0' };
    in.open(MAPS_FILE_PATH);
    string tmpWrite;
    int i = 0;

    while (in.getline(line, 1024))
    {
        vector<string> subStr = split(line, ' ');
        if (subStr.size() > 0 && subStr.at(0) == domain) {
            i++;
            tmpWrite = tmpWrite + domain + ' ' + addr;
        }
        else {
            tmpWrite += chars2string(line);
        }
        tmpWrite += '\n';
    }
    if (i == 0) {
        tmpWrite = tmpWrite + domain + ' ' + addr + '\n';
    }
    in.close();
    ofstream out;
    out.open(MAPS_FILE_PATH);
    out.flush();
    cout << "wirte to file: " << tmpWrite << endl;
    out << tmpWrite;
    out.close();
}

void readMaps(char* maps) {
    ifstream in;
    char line[1024] = { '\0' };
    in.open(MAPS_FILE_PATH);
    string tmpRead;
    while (in.getline(line, 1024))
    {
        tmpRead = tmpRead + chars2string(line) + ',';
    }
    in.close();
    tmpRead.copy(maps, tmpRead.size() - 1, 0);

}

void swapMap(string *outMap, const string inMap) {
    vector<string> tmp = split(inMap, ' ');
    *outMap = tmp.at(1) + ' ' + tmp.at(0);
}


void writeHosts(vector<string> maps) {
    ifstream in;
    char line[1024] = { '\0' };
    string tmpWrite;
    int i = 0;
    bool flag = true;

    if (maps.size() <= 0)
        return;

    string firstDomain = maps.at(0).substr(0, maps.at(0).find(' '));

    in.open(HOSTS_FILE_PATH);

    while (in.getline(line, 1024))
    {
        vector<string> subStr = split(line, ' ');
#ifdef _WIN32
        if (subStr.size() > 1 && subStr.at(0) == firstDomain) {
#else
        if (subStr.size() > 1 && subStr.at(1) == firstDomain) {
#endif // _WIN32

            flag = false;
            vector<string>::iterator it = maps.begin();
            for (; it != maps.end(); ++it) {
#ifdef _WIN32
                tmpWrite = tmpWrite + *it + '\n';
#else
                string swapped;
                swapMap(&swapped, *it);
                tmpWrite = tmpWrite + swapped + '\n';
#endif // _WIN32           
                i++;
            }
        }
        else {
            if (i - 1 > 0) {
                i--;
            }
            else {
                tmpWrite = tmpWrite + chars2string(line) + '\n';
            }
        }
    }
    if (flag) {
        vector<string>::iterator it = maps.begin();
        for (; it != maps.end(); ++it) {
#ifdef _WIN32
            tmpWrite = tmpWrite + *it + '\n';
#else
            string swapped;
            swapMap(&swapped, *it);
            tmpWrite = tmpWrite + swapped + '\n';
#endif // _WIN32
        }
    }
    in.close();
    ofstream out;
    out.open(HOSTS_FILE_PATH);
    out.flush();
    out << tmpWrite;
    out.close();
}


int getAddr6(char* pAddr6) {
#ifdef _WIN32
    struct addrinfo *ailist=NULL, *aip=NULL, hint;
    struct sockaddr_in6 *sinp6;
    char hostname[255] = { 0 };
    gethostname(hostname, sizeof(hostname));

    hint.ai_family = AF_INET6;       
    hint.ai_socktype = SOCK_STREAM;  
    hint.ai_flags = AI_PASSIVE;      
    hint.ai_protocol = 0;            
    hint.ai_addrlen = 0;             
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;

    if (getaddrinfo(hostname, NULL, &hint, &ailist) < 0){
        cout << "get addrinfo error: " << (char *)gai_strerror(errno) << endl;
        return 0;
    }
    if (ailist == NULL){
        cout << "error: no ipv6 address found!" << endl;
        return 0;
    }

    for (aip = ailist; aip != NULL; aip = aip->ai_next) {
        ZeroMemory(pAddr6, IPV6_ADDR_MAX_LEN);
        sinp6 = (struct sockaddr_in6 *)aip->ai_addr;  
        inet_ntop(AF_INET6, &sinp6->sin6_addr, pAddr6, IPV6_ADDR_MAX_LEN);
        if (strncmp(pAddr6, "fe80", 4) == 0||strlen(pAddr6)<15)
            continue;
        return 1;
    }
    cout << "error: no ipv6 address found!" << endl;

#else
    struct ifaddrs *ifalist = NULL, *ifap = NULL;
    struct sockaddr_in6 *sinp6;

    getifaddrs(&ifalist);

    for (ifap = ifalist; ifap != NULL; ifap = ifap->ifa_next) {
        if (ifap->ifa_addr->sa_family == AF_INET6) {
            ZeroMemory(pAddr6, IPV6_ADDR_MAX_LEN);
            sinp6 = (struct sockaddr_in6 *)ifap->ifa_addr;
            inet_ntop(AF_INET6, &sinp6->sin6_addr, pAddr6, IPV6_ADDR_MAX_LEN);
            //实际环境注释下面两行
            /*if (strncmp(pAddr6, "::", 2) == 0)
                continue;*/
            //实际环境中解除以下注释以筛选有效ip
            
            if (strncmp(pAddr6, "fe80", 4) == 0 || strlen(pAddr6) < 15)
                continue;
            return 1;
        }    
    }
    if (ifalist) {
        freeifaddrs(ifalist);
        ifap = NULL;
    }
#endif // _WIN32
    return 0;
}

void genMap(char *map, const char *pKey, const char *pDomain, const char * pAddress) {
    string tmpMap = chars2string(pKey) + ' ' + chars2string(pDomain) + ' ' + chars2string(pAddress);
    strncpy(map, tmpMap.c_str(), tmpMap.size());
}

/* init socket ----------------------------------------------------------------*/
void initSock() {
#ifdef _WIN32
    WSADATA data;
    WSAStartup(MAKEWORD(2, 0), &data);
#endif
}

/* clean socket ---------------------------------------------------------------*/
void cleanSock() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void socketClose(int socket) {
#ifdef _WIN32
    closesocket(socket);
#else
    close(socket);
#endif // _WIN32

}
/* get socket error -----------------------------------------------------------*/

int errsock(void) {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

/* set socket option ----------------------------------------------------------*/
int setsock(int sock)
{
    int bs = SOCK_BUFF_SIZE;

#ifdef _WIN32
    int tv = 0;
#else
    struct timeval tv = { 0 };
#endif

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) == -1 ||
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof(tv)) == -1) {
        cout << "sockopt error: notimeo :" << errsock() << endl;
        socketClose(sock);
        return 0;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char *)&bs, sizeof(bs)) == -1 ||
        setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char *)&bs, sizeof(bs)) == -1) {
        cout << "sockopt error: bufsize :" << errsock() << endl;
        return 0;
    }
    return 1;
}



void runServer(const u_short port, const string key) {
    server_t server;
    client_t client;
    char buff[BUFF_SIZE];

    if ((server.sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cout << "socket error: " << errsock() << endl;
        return;
    }

    if (!setsock(server.sock)) {
        socketClose(server.sock);
        return ;
    }

    server.addr.sin_family = AF_INET;
    server.addr.sin_port = htons(port);
#ifdef _WIN32
    server.addr.sin_addr.S_un.S_addr = INADDR_ANY;
#else
    server.addr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif // _WIN32

        if (bind(server.sock, (sockaddr *)&(server.addr), sizeof(server.addr)) == -1) {
        cout << "bind error: " << errsock() << endl;
        socketClose(server.sock);
        return;
    }

    if (listen(server.sock, 5) == -1) {
        cout << "listen error: " << errsock() << endl;
        socketClose(server.sock);
        return ;
    }
    socklen_t addr_len = sizeof(client.addr);

    while (1) {
        cout << "waitting for connecting.." << endl;
        if ((client.sock = accept(server.sock, (sockaddr *)&(client.addr), &addr_len)) == -1) {
            cout << "accept error: " << errsock() << endl;
            continue;
        }

        ZeroMemory(buff, BUFF_SIZE);
        if (recv(client.sock, buff, BUFF_SIZE, 0) == -1) {
            cout << "recv error: " << errsock() << endl;
            socketClose(client.sock);
            continue;
        }

        vector<string> strs = split(buff, ' ');
        if (strs.size() == 1) {
            cout << "received download request, checking key.." << endl;
            if (key == strs.at(0)) {
                ZeroMemory(buff, BUFF_SIZE);
                readMaps(buff);
                cout << "valid key, send maps to client: \n" << buff << endl;
                send(client.sock, buff, strlen(buff), 0);
                socketClose(client.sock);
                cout << "complete!" << endl;
                continue;
            }
            cout << "invalid key!" << endl;
        }
        else if (strs.size() == 3) {
            cout << "received upload request, checking key.." << endl;
            if (key == strs.at(0)) {
                cout << "valid key, update maps: " << strs.at(1) << ' ' << strs.at(2) << endl;
                writeMaps(strs.at(1), strs.at(2));
                socketClose(client.sock);
                cout << "complete!" << endl;
                continue;
            }
            cout << "invalid key!" << endl;
        }
    }
}

void runUpdateClient(const char *pSAddress, const u_short port, const char *pCDomain, const char *key) {
    client_t client;
    char buff[BUFF_SIZE];
    char localAddr6[IPV6_ADDR_MAX_LEN];
    ZeroMemory(buff, BUFF_SIZE);
    ZeroMemory(localAddr6, IPV6_ADDR_MAX_LEN);

    if ((client.sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cout << "socket error: " << errsock() << endl;
        return;
    }

    if (!setsock(client.sock)) {
        socketClose(client.sock);
        cout << "set socket error: " << errsock() << endl;
        return;
    }

    client.addr.sin_family = AF_INET;
    client.addr.sin_port = htons(port);
    inet_pton(AF_INET, pSAddress, &(client.addr.sin_addr));

    if (connect(client.sock, (sockaddr *)&(client.addr), sizeof(client.addr)) == -1) {
        cout << "connect error: " << errsock() << endl;
        socketClose(client.sock);
        return;
    }

    if (!getAddr6(localAddr6))
        return;
    genMap(buff, key, pCDomain, localAddr6);

    cout << "upload map: " << buff << endl;
    if (send(client.sock, buff, strlen(buff), 0) == -1) {
        cout << "send error: " << errsock() << endl;
        socketClose(client.sock);
        return;
    }

    cout << "complete!" << endl;
    socketClose(client.sock);
}


void runGetClient(const char *pSAddress, const u_short port, const char *key) {
    client_t client;
    char buff[BUFF_SIZE];
    ZeroMemory(buff, BUFF_SIZE);

    if ((client.sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cout << "socket error: " << errsock() << endl;
        return;
    }

    if (!setsock(client.sock)) {
        socketClose(client.sock);
        return;
    }

    client.addr.sin_family = AF_INET;
    client.addr.sin_port = htons(port);
    inet_pton(AF_INET, pSAddress, &(client.addr.sin_addr));

    if (connect(client.sock, (sockaddr *)&(client.addr), sizeof(client.addr)) == -1) {
        cout << "connect error: " << errsock() << endl;
        socketClose(client.sock);
        return;
    }

    cout << "download maps..." << endl;
    
    strncpy(buff, key, strlen(key));
    if (send(client.sock, buff, strlen(buff), 0) == -1) {
        cout << "send error: " << errsock() << endl;
        socketClose(client.sock);
        return;
    }


    ZeroMemory(buff, BUFF_SIZE);
    recv(client.sock, buff, BUFF_SIZE, 0);

    cout << "write maps to hosts:\n" << buff << endl;

    vector<string> maps = split(buff, ',');
    writeHosts(maps);

    cout << "complete!" << endl;

    socketClose(client.sock);

}



int resolveOpt(int argc, char *argv[], bool *pServer, bool *pUClient, bool *pGClient,
    u_short *pPort, char *pCDomain, char *pSAddress, char *pKey)
{
    int opt;
    const char *optstring = "sp:ud:ga:k:";

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
        case 's':
            *pServer = true;
            break;
        case 'u':
            *pUClient = true;
            break;
        case 'g':
            *pGClient = true;
            break;
        case 'p':
            try
            {
                *pPort = atoi(optarg);
            }
            catch (const std::exception&)
            {
                cout << "invalid port: " << optarg << endl;
                return 0;
            }
            break;
        case 'd':
            
            strncpy(pCDomain, optarg, strlen(optarg));
            break;
        case 'a':
            strncpy(pSAddress, optarg, strlen(optarg));
            break;
        case 'k':
            strncpy(pKey, optarg, strlen(optarg));
            break;
        case '?':
            cout << "error optopt: " << optopt << endl;
            cout << "error opterr: " << opterr << endl;
            break;
        }
    }
    return 1;
}

void start(const bool server, const bool uClient, const bool gClient,
    const u_short pPort, const char *pCDomain, const char *pSAddress, const char *pKey)
{
    if (!uClient && !gClient) {
        cout << "run as server.." << endl;
        runServer(pPort, pKey);
    }
    else if (!server && uClient && !gClient) {
        
        if (strlen(pSAddress) == 0) {
            cout << "you must set server address!" << endl;
            return;
        }
        if (strlen(pCDomain) == 0) {
            cout << "you must set client domain!" << endl;
            return;
        }
        cout << "update maps.." << endl;
        runUpdateClient(pSAddress, pPort, pCDomain, pKey);
    }
    else if (!server && !uClient && gClient) {
        if (strlen(pSAddress) == 0) {
            cout << "you must set server address!" << endl;
            return;
        }
        cout << "get maps.." << endl;
        runGetClient(pSAddress, pPort, pKey);
    }
    else {
        cout << "invalid arg: you have to choose which role your application run as,"
            << " and you have to choose just one role! " << endl;
        return;
    }
}

/*
*Three role:
* 1. Server:
*   -s: run as a server
* 2. Update Client:
*   -u: run as update client
*   -d: set client virtual domain name, like: example.test.com
* 3. Get Client:
*   -g: run as get client
*
* Other param:
*   -p: server port
*   -a: set server address when run as a client
*   -k: set a key
*/
int main(int argc, char *argv[])
{
    bool server = false;
    bool uClient = false;
    bool gClient = false;

    u_short port = DEFAULT_SERVER_PORT;
    char cDomain[100] = { '\0' };
    char sAddress[50] = { '\0' };
    char key[100] = DEFAULT_TRANSFER_KEY;

    initSock();

    if (!resolveOpt(argc, argv, &server, &uClient, &gClient, &port, cDomain, sAddress, key)) 
        return 0;  

    start(server, uClient, gClient, port, cDomain, sAddress, key);

}

