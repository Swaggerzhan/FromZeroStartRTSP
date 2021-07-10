//
// Created by Swagger on 2021/7/10.
//

#include "../include/rtsp.h"
#include "../include/tcp.h"
#include "../include/udp.h"
#include "../include/Buffer.h"

using std::cerr;
using std::cout;
using std::endl;

RTSP::RTSP(Tcp* rtsp_sock, Udp* rtp_sock, Udp* rtcp_sock)
:   rtsp_sock_(rtsp_sock),
    rtp_sock_(rtp_sock),
    rtcp_sock_(rtcp_sock),
    clientSock_(-1),
    isQuit_(false)
{
    clientSock_ = rtsp_sock->Accept();
    cout << "RTSP got new Connection and Init OK! " << endl;
}

RTSP::~RTSP() {


}


void RTSP::loop() {
    while ( !isQuit_ ){
        if ( !rtsp_sock_->Recv() ){
            // 失败
            cerr << "Recv() Error! " << endl;
            return;
        }
        // 判断是close还是正常回复
        if (rtsp_sock_->recv_buf->getSize() == 0){
            cout << "remote closed sock!" << endl;
        }
        // 正常解析
        char* target = rtsp_sock_->recv_buf->getBuf();

    }
}




