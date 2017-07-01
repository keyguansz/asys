#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>
#include <cutils/sockets.h>

#include "easy_common.h"

#include "SensorsServer.h"
#include "easy_parsemsg.h"


int MakeSocketIn()
{
    struct sockaddr_in sClientAddr;
    int nSocketFd = -1;

    nSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    EB_LOGD("[%s.%d] nSocketFd = %d", __FUNCTION__, __LINE__, nSocketFd);
    if (nSocketFd < 0) 
    {
        printf("[%s.%d] create socket fd failed, errno = %d", __FUNCTION__, __LINE__, errno);
        return -1;
    }

    memset(&sClientAddr, 0, sizeof(sClientAddr));
    sClientAddr.sin_family = AF_INET;
    sClientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sClientAddr.sin_port = htons(UDP_LISTEN_PORT);

    if(bind(nSocketFd, (struct sockaddr *)(&sClientAddr), sizeof(sClientAddr)) < 0) 
    {
        printf("[%s.%d] socket bind failed, errno = %d", __FUNCTION__, __LINE__, errno);
        close(nSocketFd);
        nSocketFd = -1;
        return -1;
    }

    return nSocketFd;
}

int RecvSensorsData(int nSocketFd, void* pvBuff, int bufSize)
{
    int  nRecvLen;
    int nAddrLen;
    struct sockaddr_in recvAddr;
    char acData[1024];
    int contentLen;

    nAddrLen = sizeof(recvAddr);

    do {
        nRecvLen = recvfrom(nSocketFd, acData, sizeof(acData), 0, (struct sockaddr*)(&recvAddr), &nAddrLen);
        EB_LOGD("[%s.%d] socket recvfrom, nRecvLen = %d", __FUNCTION__, __LINE__, nRecvLen);

        if(nRecvLen > 0) {
            EasybusAddr addr;
            EasybusMsg data;
            
            memset((void*)&addr, 0, sizeof(addr));
            strcpy(addr.ip, inet_ntoa(recvAddr.sin_addr));
            addr.port = ntohs(recvAddr.sin_port);
            EB_LOGD("[%s.%d] recvAddr:%s:%d\n", __FUNCTION__, __LINE__, addr.ip, addr.port);

            memset((void*)&data, 0, sizeof(data));
            easy_ctrl_decompose_frame(acData, nRecvLen, &addr, &data);
            EB_LOGD("[%s.%d] data.msgData = %s", __FUNCTION__, __LINE__, data.msgData);
            contentLen = strlen(data.msgData);
            memcpy(pvBuff, data.msgData, contentLen<bufSize?contentLen:bufSize);
        }

        EB_LOGD("[%s.%d] errno = %d", __FUNCTION__, __LINE__, errno);
    } while (nRecvLen < 0 && errno == EINTR);

    EB_LOGD("[%s.%d] contentLen", __FUNCTION__, __LINE__, contentLen);
    return contentLen;
}

