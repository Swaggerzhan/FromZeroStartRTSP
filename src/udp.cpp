//
// Created by Swagger on 2021/7/10.
//

#include "../include/udp.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

using std::string;
using std::cerr;
using std::endl;
using std::cout;


Udp::Udp(string addr, int port)
:   addr_(std::move(addr)),
    listenfd_(-1),
    port_(port)
{
    listenfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    checkError(listenfd_, "socket()");

    sockaddr_in localAddr_{};
    memset(&localAddr_, 0, sizeof(localAddr_));
    localAddr_.sin_family = AF_INET;
    localAddr_.sin_port = htons(port_);
    localAddr_.sin_addr.s_addr = inet_addr(addr_.c_str());

    int ret = bind(listenfd_, (sockaddr*)&localAddr_, sizeof(localAddr_));
    checkError(ret, "bind()");
    cout << "udp init OK ! " << endl;

}

Udp::~Udp() {

}


void Udp::checkError(int code, std::string msg) {
    if (code < 0){
        cerr << msg << " Error! code: " << code << endl;
        exit(code);
    }
}










