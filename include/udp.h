//
// Created by Swagger on 2021/7/10.
//

#ifndef FROMZEROSTARTRTSP_UDP_H
#define FROMZEROSTARTRTSP_UDP_H

#include <iostream>

class Udp{
public:

    Udp(std::string addr, int port);
    ~Udp();

private:

    static void checkError(int code, std::string msg);

private:

    int listenfd_;
    std::string addr_;
    int port_;

};




#endif //FROMZEROSTARTRTSP_UDP_H
