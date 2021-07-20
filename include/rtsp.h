//
// Created by Swagger on 2021/7/10.
//

#ifndef FROMZEROSTARTRTSP_RTSP_H
#define FROMZEROSTARTRTSP_RTSP_H
#include <iostream>
#include <vector>
#include <map>

class Tcp;
class Udp;

class RTSP{
public:

    enum Type{
        Options,
        Desc,
        Setup,
        Play,
        Error,   // 内置判断
    };
    typedef std::pair<std::string, std::string> Header;

    RTSP(Tcp* rtsp_sock, Udp* rtp_sock, Udp* rtcp_sock);

    ~RTSP();

    void loop();
    void entry();                               // RTSP解析入口
    Type parser_request_line();                 // 解析请求头
    bool split_line(char* data, int len);       // 将\r\n替换成\0\0
    void charOption2Type(char* target);         // 将char类型的option转为Type类型
    bool parser_headers();                      // 解析头字段
    void show();                                // For Debug


private:

    std::vector<char*> split_line_;         // 分行后的数据

    Tcp* rtsp_sock_;
    Udp* rtp_sock_;
    Udp* rtcp_sock_;
    int clientSock_;
    bool isQuit_;

    int CSeq_;                                      // CSeq
    Type type_;                                     // 请求类型
    std::string url_;                               // url
    std::string version_;                           // 版本

    std::map<std::string, std::string> headers_;    // 头字段
};

#endif //FROMZEROSTARTRTSP_RTSP_H
