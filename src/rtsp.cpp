//
// Created by Swagger on 2021/7/10.
//

#include "../include/rtsp.h"
#include "../include/tcp.h"
#include "../include/udp.h"
#include "../include/Buffer.h"
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::string;

const string fileDir = "/root/Video/";
const string fileName = "out.264";


bool RTSP::loadVideo() {
    string targetFile = fileDir + fileName;
    video_fd_ = open(targetFile.c_str(), O_RDONLY);
    if (video_fd_ < 0 ){
        cerr << "read file Error! " << endl; // for debug
        return false;
    }
    return true;
}



RTSP::RTSP(Tcp* rtsp_sock, Udp* rtp_sock, Udp* rtcp_sock)
:   rtsp_sock_(rtsp_sock),
    rtp_sock_(rtp_sock),
    rtcp_sock_(rtcp_sock),
    clientSock_(-1),
    isQuit_(false),
    session_(66778899) // for debug
{
    clientSock_ = rtsp_sock->Accept();
    loadVideo();
    cout << "RTSP got new Connection and Init OK! " << endl;
}

RTSP::~RTSP() {


}

void RTSP::entry() {
    char* data = rtsp_sock_->recv_buf->getBuf();
    // FOR DEBUG
    cerr << data << endl;
    int len = rtsp_sock_->recv_buf->getSize();
    if ( !split_line(data, len) ){
        cerr << "split_line() Error!" << endl;
        return;
    }
    parser_request_line();
    parser_headers();
    //show();
    ///////// 以下就开始为解析部分
    handle();

}

void RTSP::handle() {
    if ( type_ == Error ){
        // Error respond
        cerr << "handle got Error" << endl;
    }else if (type_ == Options){
        loadRespond(Options);
    }else if (type_ == Desc){
        loadRespond(Desc);
    }else if (type_ == Setup){
        // 解析出客户端的UDP地址
        auto it = headers_.find("Transport");
        if (it == headers_.end() ){
            cerr << "headers Transport didn't find!" << endl; // for debug
            return;
        }
        // for debug
        cout << "got headers: " << it->second << endl;
        char* tmp_buf = new char[it->second.size()+1];
        strcpy(tmp_buf, it->second.c_str());
        char* client_port = nullptr;
        cout << "tmp_buf: " << tmp_buf << endl;
        if ((client_port = strstr(tmp_buf, "client_port=")) == nullptr ){
            // for debug
            cerr << "header Transport didn't find client port !" << endl;
            return;
        }
        client_port += 12; // 去掉开头
        char* rtcp_port = nullptr;
        for (char* tmp=client_port; ;tmp++){
            if ( *tmp == '\0' )
                break;
            if ( *tmp == '-' ){
                *tmp = '\0';
                rtcp_port = tmp + 1;
                break;
            }
        }
        client_rtp_port = atoi(client_port);
        client_rtcp_port = atoi(rtcp_port);
        // for debug
        cout << "client_rtp_port: " << client_rtp_port << endl;
        cout << "client_rtcp_port: " << client_rtcp_port << endl;
        loadRespond(Setup);
    }else if (type_ == Play ){
        loadRespond(Play);
        cout << "start Play!!!" << endl; // for debug
    }

    rtsp_sock_->Send();
    if ( type_ == Play ){
        //// 先尝试在这个地方进行发送
        struct RtpPacket* rtpPacket = (struct RtpPacket*)malloc(50000);
        uint8_t* frame = (uint8_t*)malloc(50000);
        rtpHeaderInit(rtpPacket, 0, 0, 0, RTP_VERSION, RTP_PAYLOAD_TYPE_H264, 0,
                      0, 0, 0x88923423);
        uint32_t frameSize;
        int startCode;

        while ( true ){
            frameSize = getFrameFromH264File(video_fd_, (char*)frame, 50000);
            if (frameSize < 0 ){
                cerr << "read Error! " << endl;
                continue;
            }
            if ( check3((char*)frame) )
                startCode = 3;
            else
                startCode = 4;

            frameSize -= startCode;
            rtp_sock_->rtpSendH264Frame(rtp_sock_->getFd(),
                                        rtsp_sock_->getRemoteAddr().c_str(),
                                        client_rtp_port, rtpPacket, frame+startCode, frameSize);
            rtpPacket->header.timestamp += 90000 / 25;
            usleep( 1000 * 1000 / 25);
        }
        free(rtpPacket);
        free(frame);
    }
}


void RTSP::show(){
    cout << "CSeq: " << CSeq_ << endl;
    cout << "Type: " << type_ << endl;
    cout << "Version: " << version_ << endl;
    for (auto Iter = headers_.begin(); Iter != headers_.end(); Iter ++ ){
        cout << Iter->first << ": " << Iter->second << endl;
    }
}


void RTSP::loop() {
    while ( !isQuit_ ){
        //cout << "loop again!!!!!!!!!!!!!!" << endl;
        if ( !rtsp_sock_->Recv() ){
            // 失败
            cerr << "Recv() Error! " << endl;
            return;
        }
        // 判断是close还是正常回复
        if (rtsp_sock_->recv_buf->getSize() == 0){
            cout << "remote closed sock!" << endl;
            isQuit_ = true;
        }
        // 正常解析
        entry();

    }
}

RTSP::Type RTSP::parser_request_line() {
    if ( split_line_.empty() )
        return Error;
    char* request_line = split_line_[0];
    char* options = request_line;
    for (char *tmp = options; ;tmp ++){
        if ( *tmp == '\t' || *tmp == ' '){
            *tmp = '\0';
            request_line = tmp + 1;
            break;
        }
    }
    char* url = request_line;
    char* version = nullptr;
    for (char *tmp = url; ;tmp ++){
        if ( *tmp == '\t' || *tmp == ' '){
            *tmp = '\0';
            version = tmp + 1;
            break;
        }
    }
    if ( !options || !url || !version)
        return Error;
    url_ = string(url);
    version_ = string(version);
    charOption2Type(options);
    return type_;
}

bool RTSP::split_line(char* data, int len) {
    if ( !data )
        return false;
    split_line_.clear();
    char* pre = data;
    for (int i=0; i<len-1; i++){
        if (data[i] == '\r' && data[i+1] == '\n'){
            data[i] = '\0';
            data[i + 1] == '\0';
            if ( i >= len - 2)
                break;
            split_line_.push_back(pre);
            pre = data + i + 2;
        }
    }
    return true;
}


void RTSP::charOption2Type(char* target) {
    if ( strncasecmp(target, "OPTIONS", 7) == 0 ){
        type_ = Options;
        return;
    }else if ( strncasecmp(target, "DESCRIBE", 8) == 0 ){
        type_ = Desc;
        return;
    }else if ( strncasecmp(target, "SETUP", 5) == 0 ){
        type_ = Setup;
        return;
    }else if ( strncasecmp(target, "PLAY", 4) == 0 ){
        type_ = Play;
        return;
    }
}

bool RTSP::parser_headers(){
    if ( split_line_.empty() )
        return false;
    for (int i=1; i<split_line_.size(); i++){
        char* line = split_line_[i];
        //cout << i << ": " << line << endl;
        char* first = line;
        char* second = nullptr;
        for (char* tmp = line; *tmp != '\0' ;tmp ++){
            if ( *tmp == ':' || *tmp == '\t' || *tmp == ' '){
                *tmp = '\0';
                second = tmp + 1;
            }

        }
        if ( strncasecmp(first, "CSeq", 4) == 0 ){
            CSeq_ = atoi(second);
        }else{
            if ( !first || !second ){
                cerr << "got nullptr!!!" << endl;
                continue;
            }
            headers_.insert(Header(string(first), string(second)));
        }
    }
    //exit(-2);
    return true;
}



void RTSP::loadRespond(Type types) {
    if ( types == Error )
        return;
    char* buf = rtsp_sock_->send_buf->getBuf();

    switch (types) {
        case Options: {
            sprintf(buf, "RTSP/1.0 200 OK\r\n"
                          "CSeq: %d\r\n"
                          "Public: OPTIONS, DESCRIBE, SETUP, PLAY\r\n"
                          "\r\n",
                    CSeq_);
            rtsp_sock_->send_buf->setSize(strlen(buf));
            break;
        }
        case Desc: {
            char* sdp = new char[1024];
            sprintf(sdp, "v=0\r\n"
                         "o=- 9%ld 1 IN IP4 %s\r\n"
                         "t=0 0\r\n"
                         "a=control:*\r\n"
                         "m=video 0 RTP/AVP 96\r\n"
                         "a=rtpmap:96 H264/90000\r\n"
                         "a=control:track0\r\n",
                    time(NULL), rtsp_sock_->getAddr());
            sprintf(buf, "RTSP/1.0 200 OK\r\n"
                          "CSeq: %d\r\n"
                          "Content-Base: %s\r\n"
                          "Content-type: application/sdp\r\n"
                          "Content-length: %d\r\n\r\n"
                          "%s",
                    CSeq_,
                    url_.c_str(),
                    strlen(sdp),
                    sdp);
            rtsp_sock_->send_buf->setSize(strlen(buf));
            delete [] sdp;
            break;
        }
        case Setup: {
            sprintf(buf, "RTSP/1.0 200 OK\r\n"
                         "CSeq: %d\r\n"
                         "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n"
                         "Session: %d\r\n"
                         "\r\n",
                        CSeq_, client_rtp_port, client_rtcp_port,
                        rtp_sock_->getPort(), rtcp_sock_->getPort(),
                        session_
                    );
            rtsp_sock_->send_buf->setSize(strlen(buf));
            break;
        }
        case Play: {
            sprintf(buf, "RTSP/1.0 200 OK\r\n"
                         "CSeq: %d\r\n"
                         "Range: npt=0.000-\r\n"
                         "Session: %d; timeout=60\r\n"
                         "\r\n", CSeq_, session_);
            rtsp_sock_->send_buf->setSize(strlen(buf));
            break;
        }
    }
}

//////////////////////////////////////////////////////////////
/// 封包部分

bool RTSP::check3(char* buf) {
    if ( buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return true;
    return false;
}

bool RTSP::check4(char* buf){
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
        return true;
    return false;
}

void RTSP::rtpPacketInit(struct RtpPacket *rtpPacket, uint8_t version, uint8_t extend,
        uint8_t padding, uint8_t csrcLen, uint8_t marker, uint8_t payloadType,
        uint16_t seq, uint32_t timestamp, uint32_t ssrc)
{
    /* 1 bytes */
    rtpPacket->header.csrcLen = csrcLen;
    rtpPacket->header.extension = extend;
    rtpPacket->header.padding = padding;
    rtpPacket->header.version = version;

    /* 2 bytes */
    rtpPacket->header.payloadType = payloadType;
    rtpPacket->header.marker = marker;

    /* 3 4 bytes */
    rtpPacket->header.seq = seq;

    /* 5 6 7 8 bytes */
    rtpPacket->header.timestamp = timestamp;

    /* 9 10 11 12 bytes */
    rtpPacket->header.ssrc = ssrc;
    // 初始化完毕
}


int RTSP::readH264Frame(int fd, char *frame) {
    if ( fd < 0 ){
        cerr << "readH264Frame() fd Error!" << endl; // for debug
        return -1;
    }
    int ReadSize = ::read(fd, frame, 0x10000);
    if (ReadSize < 0 ){
        cerr << "readH264Frame() ::read() Error!" << endl; // for debug
        return -1;
    }
    if ( !check3(frame) && !check4(frame)){
        cerr << "could not find NALU" << endl; // for debug
        return -1;
    }
    int frameSize;
    /* 需要跳过当前的Nalu分割00 00 01 */
    char* nextNalu = getNextNalu(frame + 3, ReadSize - 3);
    if ( nextNalu == nullptr ){ // 已经没有下一个Nalu了，那么从头再来
        lseek(fd, 0, SEEK_SET);
        frameSize = ReadSize;
    } else {
        frameSize = nextNalu - frame; // 得到当前Nalu的长度
        /* frameSize-ReadSize得到相差于ReadSize的长度，为负数，通过相减使下次重新读取时在新的Nalu位置 */
        lseek(fd, (frameSize-ReadSize), SEEK_CUR);
    }
    return frameSize;

}


char* RTSP::getNextNalu(char *frame, int len) {
    if (!frame)
        return nullptr;

    for (int i=0; i<len; i++){
        if (check3(frame + i) || check4(frame + i))
            return frame + i;
    }

    return nullptr;
}


void RTSP::streamStart() {

    struct RtpPacket *rtpPacket = static_cast<RtpPacket*>(malloc(0x10000));
    char* frame = new char[0x10000];

    rtpPacketInit(rtpPacket, 2, 0, 0, 0, 0,
                  RTP_PAYLOAD_TYPE_H264, 0, 0, 0x11223344);

    while (){
        int frameSize = readH264Frame(video_fd_, frame);
        if ( frameSize < 0 ){
            cerr << "readH264Frame Error!" << endl; // for debug
            continue;
        }
        int offset = -1;
        if (check3(frame)) // 去掉开头的Nalu分隔符
            offset = 3;
        else
            offset = 4;
        frameSize -= offset;


    }

}



