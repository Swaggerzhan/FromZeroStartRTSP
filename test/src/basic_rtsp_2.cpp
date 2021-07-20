//
// Created by Swagger on 2021/7/19.
//


#include "../include/basic_rtsp_2.h"
#include "../../include/tcp.h"
#include "../../include/udp.h"
#include "../../include/rtsp.h"
#include <iostream>

using std::endl;
using std::cout;
using std::string;

void basic2_entry(){
    string local_address = "0.0.0.0";
    const int tcp_port = 8888;
    const int rtp_port = 5555;
    const int rtcp_port = 5556;
    Tcp rtsp(local_address, tcp_port);
    Udp rtp(local_address, rtp_port);
    Udp rtcp(local_address, rtcp_port);
    RTSP rtsp_server(&rtsp, &rtp, &rtcp);
    cout << "rtsp Init OK! " << endl;
    rtsp_server.loop();

}
