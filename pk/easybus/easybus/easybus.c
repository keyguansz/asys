#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/types.h>
 #include <sys/time.h>
#include <unistd.h>

#include "easybus.h"
#include "easy_common.h"



static struct easybus_ctrl *ctrl_conn = NULL;
static struct easybus_ctrl *monitor_conn = NULL;
/* socket pair used to exit from a blocking read */
static int exit_sockets[2] = { -1, -1 };

static char iface[PROPERTY_VALUE_MAX];
//static const char IFACE_DIR[]           = "/data/system/easyctrl";
static const char IFACE_DIR[]           = "/tmp/easyctrl";



/***************************************************************************
*  函      数:  int easybus_attach(void)
*  功      能:  attach  到easybus  总线上
*  参      数:  无
*  返回值:  <0 失败，0 成功
*  备      注:  
****************************************************************************/
int easybus_attach(void)
{
    char ifname[256];
    int count = 10;
    int ret = EasybusErr_Unknown;

    EB_LOGD("[%s.%d] start\n", __FUNCTION__, __LINE__);
    do
    {
        if ((access(IFACE_DIR, F_OK) == 0)) {
            snprintf(ifname, sizeof(ifname), "%s/%s", IFACE_DIR, iface);
        } else {
            strlcpy(ifname, iface, sizeof(ifname));
        }
    
        ctrl_conn = easybus_ctrl_open(ifname);
        
        while ((count-- > 0) && (ctrl_conn == NULL)) {
            /* sleep for 500ms */
            printf("easybus_ctrl_open failed %d :%s", count, strerror(errno));
            usleep (500000);
            ctrl_conn = easybus_ctrl_open(ifname);
        }
        if (ctrl_conn == NULL) {
            printf("Unable to open connection to supplicant on \"%s\": %s",
            ifname, strerror(errno));
            ret = EasybusErr_OpenCtrlConnFail;
            break;
        }
        EB_LOGD("[%s.%d] open ctrl_conn=0x%x\n", __FUNCTION__, __LINE__, ctrl_conn);

        monitor_conn = easybus_ctrl_open(ifname);
        if (monitor_conn == NULL) {
            easybus_ctrl_close(ctrl_conn);
            ctrl_conn = NULL;
            ret = EasybusErr_OpenMonitorConnFail;
            break;
        }
        EB_LOGD("[%s.%d] open monitor_conn=0x%x\n", __FUNCTION__, __LINE__, monitor_conn);
        
        EB_LOGD("[%s.%d] attach monitor_conn\n", __FUNCTION__, __LINE__);
        if (easybus_ctrl_attach(monitor_conn) != 0) 
    {
    EB_LOGD("[%s.%d] easybus_ctrl_attach failed!\n", __FUNCTION__, __LINE__);
    printf("easybus_ctrl_attach failed!\n");    
            easybus_ctrl_close(monitor_conn);
            easybus_ctrl_close(ctrl_conn);
            ctrl_conn = monitor_conn = NULL;
            ret = EasybusErr_AttachFail;
            break;
        }
        
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, exit_sockets) == -1)
    {
            easybus_ctrl_close(monitor_conn);
            easybus_ctrl_close(ctrl_conn);
            ctrl_conn = monitor_conn = NULL;
            ret = EasybusErr_SocketPairFail;
            break;
        }

        ret = EasybusErr_NoError;
    }while(0);

    EB_LOGD("[%s.%d] end ret=%d\n", __FUNCTION__, __LINE__, ret);
    return ret;
}


/***************************************************************************
*  函      数:  int easybus_detach(void)
*  功      能:  从easybus  总线上detach  
*  参      数:  无
*  返回值:  <0 失败，0 成功
*  备      注:  
****************************************************************************/
int easybus_detach(void)
{
    int ret = EasybusErr_Unknown;

    EB_LOGD("[%s.%d] start\n", __FUNCTION__, __LINE__);
    do
    {
        if (NULL == monitor_conn ) 
        {
            ret = EasybusErr_DetachFail;
            break;
        }
        
        if (easybus_ctrl_detach(monitor_conn) != 0) {
            ret = EasybusErr_DetachFail;
            break;
        }
    
        if (ctrl_conn != NULL) {
            easybus_ctrl_close(ctrl_conn);
            ctrl_conn = NULL;
        }
        if (monitor_conn != NULL) {
            easybus_ctrl_close(monitor_conn);
            monitor_conn = NULL;
        }
    
        if (exit_sockets[0] >= 0) {
            close(exit_sockets[0]);
            exit_sockets[0] = -1;
        }
    
        if (exit_sockets[1] >= 0) {
            close(exit_sockets[1]);
            exit_sockets[1] = -1;
        }
    
        ret = EasybusErr_NoError;
    }while(0);

    EB_LOGD("[%s.%d] end ret=%d\n", __FUNCTION__, __LINE__, ret);
    return ret;
}

/***************************************************************************
*  函      数:  int easybus_receive(EasybusMsg *revMsg)
*  功      能:  从easybus  接收数据
*  参      数:  revMsg 接收的消息
*  返回值:  <0 失败，0 成功
*  备      注:  该接口为阻塞的
****************************************************************************/
int easybus_receive(EasybusMsg *revMsg)
{
    char *buf;
    size_t nread = EASY_BUS_BUFF_MAX_LEN;
    int res;
    int fd;
    int ret = EasybusErr_Unknown;
    
    EB_LOGD("[%s.%d] start\n", __FUNCTION__, __LINE__);
    EB_LOGD("[%s.%d] monitor_conn=0x%x\n", __FUNCTION__, __LINE__, monitor_conn);
    
    do 
    {
        if (revMsg == NULL) {
            ret = EasybusErr_BadParam;
        }
        
        if (monitor_conn == NULL) {
            ret = EasybusErr_MonitorConnNoExist;
            break;
        }
        
        if ((buf = malloc(EASY_BUS_BUFF_MAX_LEN)) == NULL) {
            ret = EasybusErr_NoMemory;
            break;
        }
        EB_LOGD("[%s.%d] easybus_receive\n", __FUNCTION__, __LINE__);
            memset(buf, 0, EASY_BUS_BUFF_MAX_LEN);

        EB_LOGD("[%s.%d] before easybus_ctrl_recv\n", __FUNCTION__, __LINE__);

        fd = easybus_ctrl_get_fd(monitor_conn);

        res = easybus_ctrl_recv(monitor_conn, buf, &nread);  
        EB_LOGD("[%s.%d] res: %d, nread: %d\n", __FUNCTION__, __LINE__, res, nread);
        if (res < 0) {
            ret = EasybusErr_ReceiveFail;
            break;
        }

        if( nread == 0)
        {
            EB_LOGD("[%s.%d] recv EOF\n", __FUNCTION__, __LINE__);
            ret = EasybusErr_ReceiveEOF;
            break; 
        }
        buf[nread] = '\0';
        
    
        EB_LOGD("[%s.%d] receive buf: %s\n", __FUNCTION__, __LINE__, buf);
        
        char *str1 = NULL;
        char *str2 = NULL;
        int bufSize = strlen(buf);
        
        str1 = buf + strlen("addr.ip=");
        if (str1 - buf >= bufSize) break;
        str2 = strstr(str1, ",addr.port=");
        if (str2 == NULL) break;
        memcpy((void*)revMsg->remoteAddr.ip, str1, str2 - str1);
        EB_LOGD("ip = %s\n", revMsg->remoteAddr.ip);
        str1 = str2 + strlen(",addr.port=");
        if (str1 - buf >= bufSize) break;
        str2 = strstr(str1, ",msgType=");
        if (str2 == NULL) break;
        EB_LOGD("str2 - str1 = %d\n", str2 - str1);
        {
            char port_str[12];
            
            memset(port_str, 0, sizeof(port_str));
            memcpy((void*)port_str, str1, str2 - str1);
            EB_LOGD("port_str = %s\n", port_str);
            revMsg->remoteAddr.port = atoi(port_str);
        }
        EB_LOGD("port = %d\n", revMsg->remoteAddr.port);
        str1 = str2 + strlen(",msgType=");
        if (str1 - buf >= bufSize) break;
        str2 = strstr(str1, ",msgData=");
        if (str2 == NULL) break;
        EB_LOGD("str2 - str1 = %d\n", str2 - str1);
        memcpy((void*)revMsg->msgType, str1, str2 - str1);
        EB_LOGD("msgType = %s\n", revMsg->msgType);
        str1 = str2 + strlen(",msgData=");
        if (str1 - buf >= bufSize) break;
        revMsg->msgDataSize = strlen(str1);
        EB_LOGD("msgDataSize = %d\n", revMsg->msgDataSize);
        if (revMsg->msgDataSize > EASYBUS_MSGDATA_MAX_LEN) break;
        strcpy(revMsg->msgData, str1);
        EB_LOGD("msgData = %s\n", revMsg->msgData);
        
        EB_LOGD("[%s.%d] addr:%s:%d, msgType=%s, msgData=%s\n", __FUNCTION__, __LINE__, 
            revMsg->remoteAddr.ip, revMsg->remoteAddr.port, revMsg->msgType, revMsg->msgData);
        
        ret = EasybusErr_NoError;
    }while(0);
    
    if (buf != NULL) free(buf);
    
    EB_LOGD("[%s.%d] end ret=%d\n", __FUNCTION__, __LINE__, ret);
    return ret;
}


/***************************************************************************
*  函      数:  int easybus_send(EasybusMsg *sendMsg)
*  功      能:  通过easybus  发送数据
*  参      数:  sendMsg 发送的消息
*  返回值:  <0 失败，0 成功
*  备      注:   该接口为阻塞的
****************************************************************************/
int easybus_send(EasybusMsg *sendMsg)
{
    char *buf = NULL;
    char reply[10];
    size_t replyLen = 10;
    int res;
    int ret = EasybusErr_Unknown;
    
    EB_LOGD("[%s.%d] start\n", __FUNCTION__, __LINE__);
    EB_LOGD("[%s.%d] monitor_conn=0x%x\n", __FUNCTION__, __LINE__, ctrl_conn);

    do
    {
        if( NULL == ctrl_conn)
        {
            printf("ERROR: easybus_send fail, ctrl_conn is NULL.\n");
            ret = EasybusErr_BadParam;
            break;
        }
        if (sendMsg == NULL || sendMsg->msgDataSize > EASYBUS_MSGDATA_MAX_LEN) {
            ret = EasybusErr_BadParam;
            break;
        }
        
        if ((buf = malloc(EASY_BUS_BUFF_MAX_LEN)) == 0) {
            ret = EasybusErr_NoMemory;
            break;
        }
        memset(buf, 0, EASY_BUS_BUFF_MAX_LEN);
                
        sprintf(buf, "SEND addr.ip=%s,addr.port=%d,msgType=%s,msgData=", sendMsg->remoteAddr.ip, 
            sendMsg->remoteAddr.port, sendMsg->msgType);
        strncat(buf, sendMsg->msgData, sendMsg->msgDataSize);

        EB_LOGD("[%s.%d] send buf:%s\n", __FUNCTION__, __LINE__, buf);
        
        memset(reply, 0, sizeof(reply));
        res = easybus_ctrl_request(ctrl_conn, buf, strlen(buf), reply, &replyLen, NULL);
        EB_LOGD("[%s.%d] res=%d, reply=%s\n", __FUNCTION__, __LINE__, res, reply);
        if (res == 0 && memcmp(reply, "OK\n", 3) == 0) {
            ret = EasybusErr_NoError;
        } else {
            if (ret == -2) {
                /* unblocks the monitor receive socket for termination */
                write(exit_sockets[0], "T", 1);
            } 
            
            ret = EasybusErr_SendFail;
        }
        
    }while(0);
    
    if (buf != NULL) free(buf);
    
    EB_LOGD("[%s.%d] end ret=%d\n", __FUNCTION__, __LINE__, ret);
    return ret;
}

/***************************************************************************
*  函      数:  int easybus_interest(char *msgType)
*  功      能:  注册感兴趣的消息类型
*  参      数:  msgType 消息类型，ASCII编码，最大不能超过16字节
*  返回值:  <0 失败，0成功
*  备      注:  
****************************************************************************/
int easybus_interest(char *msgType)
{
    char interest[100];
    char reply[10];
    size_t replyLen = 10;
    int ret = EasybusErr_Unknown;
    int res;
        
    EB_LOGD("[%s.%d] start msgType=%s\n", __FUNCTION__, __LINE__, msgType);
    EB_LOGD("[%s.%d] monitor_conn=0x%x\n", __FUNCTION__, __LINE__, monitor_conn);

    do
    {
        if(NULL == monitor_conn)
        {
            ret = EasybusErr_BadParam;
            break;
        }
        
        if (msgType == NULL) {
            ret = EasybusErr_BadParam;
            break;
        }
        
        memset(interest, 0, sizeof(interest));
        memset(reply, 0, sizeof(reply));
        
        sprintf(interest, "INTEREST msgType=%s", msgType);
        
        res = easybus_ctrl_request(monitor_conn, interest, strlen(interest), 
            reply, &replyLen, NULL);
        EB_LOGD("[%s.%d] res=%d, reply=%s\n", __FUNCTION__, __LINE__, res, reply);
        if (res == 0 && memcmp(reply, "OK\n", 3) == 0) {
            ret = EasybusErr_NoError;
        } else {
            if (ret == -2) {
                /* unblocks the monitor receive socket for termination */
                write(exit_sockets[0], "T", 1);
            } 
        
            ret = EasybusErr_InterestFail;
        }
    }while(0);
    
    EB_LOGD("[%s.%d] end ret=%d\n", __FUNCTION__, __LINE__, ret);
    return ret;
}

