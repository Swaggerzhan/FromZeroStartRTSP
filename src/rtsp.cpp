//
// Created by Swagger on 2021/7/10.
//

#include "../include/rtsp.h"
#include "../include/tcp.h"
#include "../include/udp.h"
#include "../include/Buffer.h"
#include <cstring>

using std::cerr;
using std::cout;
using std::endl;
using std::vector;
using std::string;

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
    show();
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



