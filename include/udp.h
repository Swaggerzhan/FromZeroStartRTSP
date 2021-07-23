//
// Created by Swagger on 2021/7/10.
//

#ifndef FROMZEROSTARTRTSP_UDP_H
#define FROMZEROSTARTRTSP_UDP_H

#include <iostream>

#define RTP_HEADER_SIZE 12
#define RTP_MAX_PKT_SIZE 1400
#define RTP_VERSION 2
#define RTP_PAYLOAD_TYPE_H264 96

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

/**
 * 检测buf是否是以00 00 01开头
 **/
bool check3(char* buf);

/**
 *  检测buf是否是以00 00 00 01开头
 **/
bool check4(char* buf);

/**
 *  从h264文件中读取帧
 *  @param fd 文件fd
 *  @param frame 帧读取后存储位置，需要提前申请
 *  @param size 帧大小
 **/
int getFrameFromH264File(int fd, char* frame, int size);

/**
 * 通过给定的buf找出下一个NALU单元
 *
 **/
char* findNextStartCode(char* buf, int len);


void rtpHeaderInit(struct RtpPacket* rtpPacket, uint8_t csrcLen,
        uint8_t extension, uint8_t padding, uint8_t version,
        uint8_t payloadType, uint8_t marker, uint16_t seq,
        uint32_t timestamp, uint32_t ssrc);

int rtpSendPacket(int socket, std::string ip, int port,
                  struct RtpPacket* rtpPacket, uint32_t dataSize);

class Udp{
public:

    Udp(std::string addr, int port);
    ~Udp();

    int getPort();
    int getFd();

    int rtpSendH264Frame(int socket, std::string ip, int port,
                         struct RtpPacket* rtpPacket, uint8_t* frame,
                                 uint32_t frameSize);

private:

    static void checkError(int code, std::string msg);

private:

    int listenfd_;
    std::string addr_;
    int port_;

};




#endif //FROMZEROSTARTRTSP_UDP_H
