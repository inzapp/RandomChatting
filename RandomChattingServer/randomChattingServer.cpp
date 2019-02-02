#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#define SERVERPORT 9000
#define BUFSIZE 512

using namespace std;

CRITICAL_SECTION cs;

// print socket error and quit
void err_quit(char* msg){
    LPVOID lpMsgBuf;
    FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, WSAGetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&lpMsgBuf, 0, NULL);

    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// print socket error
void err_display(char* msg){
    LPVOID lpMsgBuf;
    FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, WSAGetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&lpMsgBuf, 0, NULL);

    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

class ChatRoom{
private:
    vector<SOCKET> socketVec;
    int retval;
    char myId[10] = "나 : ";
    char yourId[20] = "낮선 상대 : ";
    bool once = false;

public:
    unsigned int getClientCount(){
        return this->socketVec.size();
    }

    void addClient(SOCKET pSocket){
        this->socketVec.push_back(pSocket);
    }

    void sendMsg(char msg[], SOCKET mySocket, bool isInfoMsg){
        // delete '\n' character
        int len = strlen(msg);
        if(msg[len - 1] == '\n') msg[len - 1] = '\0';
        if(strlen(msg) == 0) return;

        // send
        for(unsigned int i=0; i<this->socketVec.size(); i++){
            if(isInfoMsg){
                retval = send(this->socketVec.at(i), msg, strlen(msg), 0);
            }
            else {
                if(this->socketVec.at(i) == mySocket) {
                    retval = send(this->socketVec.at(i), myId, strlen(myId), 0);
                } else {
                    retval = send(this->socketVec.at(i), yourId, strlen(yourId), 0);
                }

                retval = send(this->socketVec.at(i), msg, strlen(msg), 0);
                if(retval == SOCKET_ERROR){
                    err_display("send()");
                }
            }
        }
    }

    void deleteClient(){
        for(unsigned int i=0; i<this->socketVec.size(); i++){
            closesocket(this->socketVec.at(i));
        }

        this->socketVec.clear();
    }
};
vector<ChatRoom*> chatRoomVec;

void deleteRoom(ChatRoom* myRoom){
    for(unsigned int i=0; i<chatRoomVec.size(); i++){
        if(chatRoomVec.at(i) == myRoom){
            EnterCriticalSection(&cs);
            chatRoomVec.erase(chatRoomVec.begin() + i);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
            printf("[Delete room. room count : %d]\n", chatRoomVec.size());
            LeaveCriticalSection(&cs);
            return;
        }
    }
}

DWORD WINAPI ChatThread(LPVOID arg){
    SOCKET* socketPtr = (SOCKET*) arg;
    SOCKET client_sock = *socketPtr;

    // enter client into chat room
    if(chatRoomVec.size() == 0){
        ChatRoom* newChatRoom = new ChatRoom();
        newChatRoom->addClient(client_sock);
        chatRoomVec.push_back(newChatRoom);
    }else if(chatRoomVec.at(chatRoomVec.size() - 1)->getClientCount() == 1){
        chatRoomVec.at(chatRoomVec.size() - 1)->addClient(client_sock);
    }else{
        ChatRoom* newChatRoom = new ChatRoom();
        newChatRoom->addClient(client_sock);
        chatRoomVec.push_back(newChatRoom);
    }

    // get chat room pointer
    ChatRoom* myRoom = chatRoomVec.at(chatRoomVec.size() - 1);

    // send info message when client enter room
    if(myRoom->getClientCount() == 1){
        myRoom->sendMsg("[상대방을 대기중입니다]", client_sock, true);
    } else if(myRoom->getClientCount() == 2){
        myRoom->sendMsg("[상대방이 입장하였습니다]", client_sock, true);
    }

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
    printf("[New client entered. room count : %d]\n", chatRoomVec.size());

    int retval;
    char buf[BUFSIZE + 1];

    // networking with client
    while(1){
        retval = recv(client_sock, buf, BUFSIZE, 0);
        buf[retval] = '\0';


        if(retval == SOCKET_ERROR || retval == 0){
            myRoom->deleteClient();
            deleteRoom(myRoom);
            break;
        }

        myRoom->sendMsg(buf, client_sock, false);
    }

    // closesocket()
    closesocket(client_sock);
}

int main()
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    int retval;

    // init socket
    WSADATA wsa;
    int socketInitResult = WSAStartup(MAKEWORD(2, 2), &wsa);
    if(socketInitResult != 0) return 1;
    printf("[WSAStartup success]\n");

    // socket()
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_sock == INVALID_SOCKET) err_quit("socket()");
    printf("[Socket initialization success]\n");

    // bind()
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
    if(retval == SOCKET_ERROR) err_quit("bind()");
    printf("[Socket binding success]\n");

    // listen()
    retval = listen(listen_sock, SOMAXCONN);
    if(retval == SOCKET_ERROR) err_quit("listen()");
    printf("[Socket listening setting success]\n");
    printf("[Server initialization success]\n");
    printf("[Waiting for new client...]\n");

    // critical section
    InitializeCriticalSection(&cs);

    // var will be used is data networking
    SOCKET client_sock;
    SOCKADDR_IN clientaddr;
    int addrlen;

    while(1){
        // accept
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
        if(client_sock == INVALID_SOCKET){
            err_display("accept()");
            break;
        }

        // send client to thread
        HANDLE newThread = CreateThread(NULL, 0, ChatThread, &client_sock, 0, NULL);
        if(newThread == NULL) break;
        CloseHandle(newThread);
    }

    // closesocket()
    closesocket(listen_sock);

    // exit winsock
    WSACleanup();
    return 0;
}
