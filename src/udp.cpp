//
// Created by Swagger on 2021/7/10.
//

#include "../include/udp.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

using std::string;
using std::cerr;
using std::endl;
using std::cout;


bool check3(char* buf){
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return true;
    return false;
}


bool check4(char* buf){
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
        return true;
    return false;
}


int getFrameFromH264File(int fd, char* frame, int size){
    if ( fd < 0 ){
        cerr << "fd Error!" << endl; // for debug
        return -1;
    }

    int rSize, frameSize;
    char* nextStartCode;
    rSize = ::read(fd, frame, size);
    if ( !check3(frame) && !check4(frame)){
        cerr << "check failed! " << endl; // for debug
        return -1;
    }
    // 跳过当前头三个字节
    nextStartCode = findNextStartCode(frame+3, rSize-3);
    if ( nextStartCode == nullptr ){ // 没有了，从头开始重新播放
        lseek(fd, 0, SEEK_SET);
        frameSize = rSize;
    }else { // 还有，重新设定读取到的位置
        frameSize = nextStartCode - frame; // 帧数长度
        lseek(fd, frameSize-rSize, SEEK_CUR ); // 重新将读取长度设定回下一个帧开始位置
    }

    return frameSize;

}


char* findNextStartCode(char* buf, int len){
    if (len < 3)
        return nullptr;
    for (int i=0; i<len-3; i++){
        if (check3(buf) || check4(buf))
            return buf;
        buf ++;
    }
    if (check3(buf))
        return buf;
    return nullptr;
}


void rtpHeaderInit(struct RtpPacket* rtpPacket, uint8_t csrcLen,
                   uint8_t extension, uint8_t padding, uint8_t version,
                   uint8_t payloadType, uint8_t marker, uint16_t seq,
                   uint32_t timestamp, uint32_t ssrc){
    rtpPacket->header.csrcLen = csrcLen;
    rtpPacket->header.extension = extension;
    rtpPacket->header.padding = padding;
    rtpPacket->header.version = version;
    rtpPacket->header.payloadType = payloadType;
    rtpPacket->header.marker = marker;
    rtpPacket->header.seq = seq;
    rtpPacket->header.timestamp = timestamp;
    rtpPacket->header.ssrc = ssrc;
}

int rtpSendPacket(int socket, std::string ip, int port,
                  struct RtpPacket* rtpPacket, uint32_t dataSize){
    struct sockaddr_in addr{};
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    rtpPacket->header.seq = htons(rtpPacket->header.seq);
    rtpPacket->header.timestamp = htonl(rtpPacket->header.timestamp);
    rtpPacket->header.ssrc = htonl(rtpPacket->header.ssrc);

    int ret = sendto(socket, (char*)rtpPacket, dataSize+RTP_HEADER_SIZE, 0,
                     (sockaddr*)&addr, sizeof(addr));

    rtpPacket->header.seq = ntohs(rtpPacket->header.seq);
    rtpPacket->header.ssrc = ntohl(rtpPacket->header.ssrc);
    rtpPacket->header.timestamp = ntohl(rtpPacket->header.timestamp);
    return ret;
}


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


int Udp::getPort() {
    return port_;
}

int Udp::getFd() {
    return listenfd_;
}


void Udp::checkError(int code, std::string msg) {
    if (code < 0){
        cerr << msg << " Error! code: " << code << endl;
        exit(code);
    }
}


int Udp::rtpSendH264Frame(int socket, std::string ip, int port, struct RtpPacket *rtpPacket,
        uint8_t *frame, uint32_t frameSize)
{
    int sendBytes = 0;
    uint8_t naluType = frame[0];
    int ret;
    if (frameSize <= RTP_MAX_PKT_SIZE ){ // nalu长度小于最大包长，使用单一NALU单元模式
        memcpy(rtpPacket->payload, frame, frameSize);
        ret = rtpSendPacket(socket, ip, port, rtpPacket, frameSize);
        if ( ret < 0 )
            return -1;

        rtpPacket->header.seq ++; // 序列号递增
        sendBytes += ret;
        if ( (naluType & 0x1f) == 7 || (naluType & 0x1f) == 8)
            goto out;
    }else { // nalu长度大于最大包长，需要进行分包传输
        int pktNum = frameSize / RTP_MAX_PKT_SIZE; // 完整包
        int remainPktSize = frameSize % RTP_MAX_PKT_SIZE; // 剩下部分
        int pos = 1;
        for (int i=0; i<pktNum; i++){
            rtpPacket->payload[0] = (naluType & 0x60 ) | 28;
            rtpPacket->payload[1] = naluType & 0x1f;

            if ( i == 0 ){ // 分包第一个
                rtpPacket->payload[1] |= 0x80;
            }else if ( remainPktSize == 0 && i == pktNum-1){ // 分包最后一个
                rtpPacket->payload[1] |= 0x40;
            }

            memcpy(rtpPacket->payload+2, frame + pos, RTP_MAX_PKT_SIZE);
            ret = rtpSendPacket(socket, ip, port, rtpPacket, RTP_MAX_PKT_SIZE+2);
            if ( ret < 0 ){
                cerr << "rtpSendPacket() Error! 分包" << endl; // for debug
                return -1;
            }
            rtpPacket->header.seq ++;
            sendBytes += ret;
            pos += RTP_MAX_PKT_SIZE;
        }
        if ( remainPktSize > 0 ){ //剩余的数据包
            rtpPacket->payload[0] = (naluType & 0x60) | 28;
            rtpPacket->payload[1] = naluType | 0x1f;
            rtpPacket->payload[1] |= 0x40; // 结束

            memcpy(rtpPacket->payload+2, frame + pos, remainPktSize + 2);
            /* 其中的+2为多出来的2个header */
            ret = rtpSendPacket(socket, ip, port, rtpPacket, remainPktSize+2);
            if ( ret < 0 ){
                cerr << "rtpSendPacket in remainPktSize Error! " << endl;
                return -1;
            }
            rtpPacket->header.seq ++;
            sendBytes += ret;
        }

    }
    out:
    return sendBytes;
}











