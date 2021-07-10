//
// Created by Swagger on 2021/7/10.
//

#ifndef FROMZEROSTARTRTSP_RTSP_H
#define FROMZEROSTARTRTSP_RTSP_H


class Tcp;
class Udp;

class RTSP{
public:

    RTSP(Tcp* rtsp_sock, Udp* rtp_sock, Udp* rtcp_sock);

    ~RTSP();

    void loop();

private:

    Tcp* rtsp_sock_;
    Udp* rtp_sock_;
    Udp* rtcp_sock_;
    int clientSock_;
    bool isQuit_;


};

#endif //FROMZEROSTARTRTSP_RTSP_H
