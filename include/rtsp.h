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

struct RtpHeader{

    /* byte 0 */
    uint8_t csrcLen:4;
    uint8_t extension:1;
    uint8_t padding:1;
    uint8_t version:2;

    /* byte 1 */
    uint8_t payloadType:7;
    uint8_t marker:1;

    /* bytes 2,3 */
    uint16_t seq;

    /* bytes 4-7 */
    uint32_t timestamp;

    /* bytes 8-11 */
    uint32_t ssrc;

};

struct RtpPacket{
    struct RtpHeader header{};
    uint8_t payload[0];
};




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

    bool loadVideo();

    void loop();
    void entry();                               // RTSP解析入口
    Type parser_request_line();                 // 解析请求头
    bool split_line(char* data, int len);       // 将\r\n替换成\0\0
    void charOption2Type(char* target);         // 将char类型的option转为Type类型
    bool parser_headers();                      // 解析头字段
    void show();                                // For Debug
    void handle();                              // 解析各种请求生成对应响应


    ///////////////////////////////////////////////////////////////
    /// 响应部分

    void respond(Type types);
    void loadRespond(Type types);                // 填充响应



    //////////////////////////////////////////////////////////////
    /// 封包部分
    /**
     * check3检测00 00 01状态的NALU
     **/
    bool check3(char* buf);

    /**
     *  check4检测00 00 00 01状态的NALU
     **/
    bool check4(char* buf);

    /**
     *  初始化RTP包
     **/
    void rtpPacketInit(struct RtpPacket* rtpPacket, uint8_t version,
            uint8_t extend, uint8_t padding, uint8_t csrcLen,
            uint8_t marker, uint8_t payloadType, uint16_t seq,
            uint32_t timestamp, uint32_t ssrc);

    /**
     *  找到当前帧的下一个开始指针
     **/
    char* getNextNalu(char* frame, int len);

    /**
     *  从H264中读取帧
     *  @param fd h264文件
     *  @param frame 帧存储位置
     *  @return 返回读取一帧大小，即NALU长度
     **/
    int readH264Frame(int fd, char* frame);

    void sendRtpFrame(int fd, char* addr, int port,
                      struct RtpPacket* rtpPacket, char* frame, int frameSize);

    /**
     *  流开始
     **/
    void streamStart();



private:

    std::vector<char*> split_line_;         // 分行后的数据

    int video_fd_;                                  // 视频文件fd

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

    int client_rtp_port;
    int client_rtcp_port;
    int session_;



};

#endif //FROMZEROSTARTRTSP_RTSP_H
