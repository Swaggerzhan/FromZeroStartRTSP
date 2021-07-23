//
// Created by Swagger on 2021/7/10.
//

#ifndef FROMZEROSTARTRTSP_SOCKET_H
#define FROMZEROSTARTRTSP_SOCKET_H

#include <iostream>

class Buffer;


class Tcp{
public:
    Tcp(std::string addr, int port);
    ~Tcp();

    int Accept();
    char* getAddr() const;
    std::string getRemoteAddr();

    // 暂时开放
    Buffer* recv_buf;
    Buffer* send_buf;

    bool Recv();
    bool Send();

private:
    static void errorCheck(int code, std::string msg);

private:
    std::string addr_;
    int port_;
    int listenfd_;
    int clientSock_;

    std::string remoteAddr_;

};



#endif //FROMZEROSTARTRTSP_SOCKET_H
