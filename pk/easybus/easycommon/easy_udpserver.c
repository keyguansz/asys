#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdarg.h>

#include "easy_common.h"
#include "easy_udpserver.h"

#define EASY_SOCKET_RECEIVE_BUF_MAX_LEN       (1280)

static pfParseMsgFuncs s_fun_ParseMsg = NULL;
static int s_udp_socket = -1;

void  *easy_ctrl_udp_receive_thread(void *arg)
{
    struct sockaddr_in sServAddr;
    int n;
    socklen_t servaddr_len;
    int nPort = -1;
    fd_set read_fds;
       struct timeval sTimeoutVal;
    char *pcRecvs = NULL;
    struct timeval now;

    if(arg == NULL)
    {
        printf("[%s.%d] arg = NULL\n", __FUNCTION__, __LINE__);
        return NULL;        
    }

    nPort = *(int*)arg;
    free(arg);
    arg = NULL;

    pcRecvs = (char *)malloc(EASY_SOCKET_RECEIVE_BUF_MAX_LEN);    
    if(pcRecvs == NULL)
    {
        EB_LOGD("[%s.%d] pcRecvs = NULL\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    
    EB_LOGD("[%s.%d] nPort = %d\n", __FUNCTION__, __LINE__, nPort); 
            
    s_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    EB_LOGD("[%s.%d] socket=%d\n", __FUNCTION__, __LINE__, s_udp_socket);
    if(s_udp_socket < 0)
    {
        printf("[%s.%d] socket error\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    
    bzero(&sServAddr, sizeof(sServAddr));
    sServAddr.sin_family = AF_INET;
    sServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    sServAddr.sin_port = htons(nPort);  
    if((n = bind(s_udp_socket, (struct sockaddr *)&sServAddr, sizeof(sServAddr))) < 0)
    {
        printf("[%s.%d] bind error:%d, %s\n", __FUNCTION__, __LINE__, n, strerror(errno));
        return NULL;
    }
        
    while(1)
    {    
           FD_ZERO(&read_fds);
        FD_SET(s_udp_socket, &read_fds);
        
        sTimeoutVal.tv_sec  = 2;
        sTimeoutVal.tv_usec = 0;
        
        n = select(s_udp_socket + 1, &read_fds, 0, 0, &sTimeoutVal);
        if(n <= 0)
        {
            continue;
        }
        else
        {
            struct sockaddr_in recvAddr;
            int addr_len =sizeof(struct sockaddr_in);
            gettimeofday(&now, NULL);
            n = recvfrom(s_udp_socket, (char *)(pcRecvs), EASY_SOCKET_RECEIVE_BUF_MAX_LEN, 0, 
                (struct sockaddr *)&recvAddr, &addr_len);
            
            EB_LOGD("##EASYDATA [%u] [%s.%d] from addr: %s:%d\n", 
                        now.tv_sec, __FUNCTION__, __LINE__, 
                        inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
            EB_LOGD("##EASYDATA [%s.%d] receive len:%d\n", 
                        __FUNCTION__, __LINE__, n);
            
            if(s_fun_ParseMsg != NULL)
            {
                EasybusAddr addr;

                memset((void*)&addr, 0, sizeof(addr));
                strcpy(addr.ip, inet_ntoa(recvAddr.sin_addr));
                addr.port = ntohs(recvAddr.sin_port);
                EB_LOGD("[%s.%d] recvAddr:%s:%d\n", __FUNCTION__, __LINE__, addr.ip, addr.port);
                s_fun_ParseMsg(pcRecvs, n, &addr, NULL);
            }
        }        
    }
}

void thr_udp_send(char *ip, int port, char *cmd, unsigned short cmd_len)
{
    struct sockaddr_in sockaddr;
    int ret;

    EB_LOGD("thr_udp_send start\n");

    if (ip == NULL || cmd == NULL) {
        printf("[%s.%d] parameter error !\n", __FUNCTION__, __LINE__);
        return;
    }
    
    EB_LOGD("ip:%s:%d, cmd_len:%d\n", ip, port, cmd_len);

    bzero(&sockaddr, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(ip);
    sockaddr.sin_port = htons(port);
    if ((ret = sendto(s_udp_socket, cmd, cmd_len, 0, (struct sockaddr *) &sockaddr, sizeof(sockaddr))) < 0)
    {
        printf("thr_udp_send sendto failed ! %s\n", strerror(errno));
    }
    
    EB_LOGD("thr_udp_send end\n");
}

int easy_ctrl_creat_udp_server(int nPort, pfParseMsgFuncs fParseMsgFunc)
{
    static pthread_t tidUdpReceive;
    int *pnPort = NULL;
    int nRet = -1;

    EB_LOGD("[%s.%d] nPort = %d\n", __FUNCTION__, __LINE__, nPort);

    pnPort = (int *)malloc(sizeof(int));
    *pnPort = nPort;

    //启动UDP消息接收线程
    nRet = pthread_create(&tidUdpReceive, NULL, easy_ctrl_udp_receive_thread, pnPort);       
    if (nRet != 0)
    {
        free(pnPort);
        printf("Failed:create easy_ctrl_udp_receive_thread thread\n");
        return -1;
    }
    pthread_detach(tidUdpReceive);
    
    s_fun_ParseMsg = fParseMsgFunc;

    return 0;
}


