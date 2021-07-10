#include "include/tcp.h"
#include "include/udp.h"

#define LOCAL_ADDR "0.0.0.0"
#define RTSP_PORT   8888
#define RTP_PORT    5555
#define RTCP_PORT   5556


int main() {
    Tcp rtsp(LOCAL_ADDR, RTSP_PORT);
    Udp rtp(LOCAL_ADDR, RTP_PORT);
    Udp rtcp(LOCAL_ADDR, RTCP_PORT);
}
