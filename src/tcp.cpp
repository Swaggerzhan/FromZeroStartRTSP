//
// Created by Swagger on 2021/7/10.
//

#include "../include/Buffer.h"
#include "../include/tcp.h"
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using std::endl;
using std::cout;
using std::cerr;
using std::string;


Tcp::Tcp(std::string addr, int port)
:   addr_(std::move(addr)),
    port_(port),
    recv_buf(nullptr),
    send_buf(nullptr)
{
    recv_buf = new Buffer(2048);
    send_buf = new Buffer(2048);

    listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
    errorCheck(listenfd_, "socket()");

    int reuse = 1;
    setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in localAddr{};
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_addr.s_addr = inet_addr(addr_.c_str());
    localAddr.sin_port = htons(port_);
    localAddr.sin_family = AF_INET;
    int ret = bind(listenfd_, (sockaddr*)&localAddr, sizeof(localAddr));
    errorCheck(ret, "bind()");

    ret = listen(listenfd_, 5);
    errorCheck(ret, "listen()");
    cout << "socket init OK !" << endl;
}

Tcp::~Tcp() {
    delete recv_buf;
    delete send_buf;
}


void Tcp::errorCheck(int code, string msg) {
    if (code < 0){
        cerr << msg << " Error " << "exit code: " << code << endl;
        exit(code);
    }
}

char* Tcp::getAddr() const {
    string addr = "112.74.168.48";
    return const_cast<char*>(addr.c_str());
    //return (char*)addr_.c_str();
}


int Tcp::Accept() {

    sockaddr_in remoteAddr{};
    memset(&remoteAddr, 0, sizeof(remoteAddr));
    socklen_t remoteAddrSz = sizeof(remoteAddr);
    int sock = ::accept(listenfd_, (sockaddr*)&remoteAddr, &remoteAddrSz);
    errorCheck(sock, "accept()");
    clientSock_ = sock;
    return sock;

}


bool Tcp::Recv() {
    int len = ::read(clientSock_, recv_buf->getBuf(), recv_buf->getCapacity());
    cout << "got len: " << len << endl;
    if ( len < 0 ){
        // error
        cerr << "::read() Error!" << endl;
        return false;
    }
    recv_buf->setSize(len);
    if ( len == 0 ){
        // close
        close(clientSock_);
        return true;
    }
    return true;

}

bool Tcp::Send() {
    cout << "-------------------------下---------------------------" << endl;
    cout << "total len: " << send_buf->getSize() << endl;
    int len = ::write(clientSock_, send_buf->getBuf(), send_buf->getSize());
    cout << send_buf->getBuf() << endl;
    cout << "send len: " << len << endl; // for debug
    cout << "-------------------------上---------------------------" << endl;
    if (len < 0){
        cerr << "::write() Error!" << endl;
        return false;
    }

    return true;
}





