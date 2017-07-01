#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include "easy_common.h"
#include "easy_parsemsg.h"

#define EASY_BUS_MSG_TYPE_SENSOR  "easysensor"

#define EASYBUS_FRAME_MIN_LEN  (56)
#define EASYBUS_MAIN_VER  (1)
#define EASYBUS_SUB_VER  (1)


static char s_aucMsgHead[4] = {0x45, 0x43, 0x45, 0x42};
static pfMsgCallback msgCb = NULL;

void easy_ctrl_register_msg_cb(pfMsgCallback *cb)
{
    msgCb = cb;
}

void ts_printf(char *format, ...)
{
    va_list ap;
    char *buf;
    int formated_str_len = 0;
    int i;

    buf = (char*)malloc(1024);
    if (buf == NULL) {
        return;
    }
    memset(buf, 0, sizeof(1024));
    
    va_start(ap, format);
    vsnprintf(buf, 1024-1, format, ap);
    va_end(ap);
    
    formated_str_len = strlen(buf);
    for( i = 0; i < formated_str_len; i++)
    {
        EB_LOGD("%c", buf[i]);
    }

    free(buf);
}

int easy_ctrl_decompose_frame(char *msg, int msgLen, EasybusAddr *addr, EasybusMsg *pOutdata)
{
    char *pcMsgTemp = msg;
    char *pData = NULL;
    EasybusMsg data;
    int dataLen = 0;
    int msgNum = 0;
    char crcFlag;
    unsigned int crcRecv;
    char encryptFlag;

    EB_LOGD("[%s.%d] msgLen: %d \n", __FUNCTION__, __LINE__, msgLen);

    if(msg == NULL || msgLen <= EASYBUS_FRAME_MIN_LEN || addr == NULL)
    {
        printf("[%s.%d] parameter error !\n", __FUNCTION__, __LINE__);
        return -1;
    }

    memset((void*)&data, 0, sizeof(data));
    memcpy((void*)&data.remoteAddr, (void*)addr, sizeof(EasybusAddr));

    EB_PRINT_MEM("the receive frame:", msg, msgLen);

    //帧起始符
    if (memcmp((void*)pcMsgTemp, s_aucMsgHead, 4) != 0) {
        printf("[%s.%d] the msg is not formatted correctly !\n", __FUNCTION__, __LINE__);
        return -2;
    }
    pcMsgTemp += 4;

    //主板本号
    if (pcMsgTemp[0] != EASYBUS_MAIN_VER) {
        printf("[%s.%d] main version error !!!\n", __FUNCTION__, __LINE__);
        return -3;
    }
    pcMsgTemp++;

    //次板本号
    if (pcMsgTemp[0] != EASYBUS_SUB_VER) {
        printf("[%s.%d] sub version error !!!\n", __FUNCTION__, __LINE__);
        return -4;
    }
    pcMsgTemp++;

    //校验标识
    crcFlag = pcMsgTemp[0];
    EB_LOGD("[%s.%d] crcFlag=%d\n", __FUNCTION__, __LINE__, crcFlag);
    if (crcFlag != 1) {
        return -5;
    }
    pcMsgTemp++;

    //加密标识
    encryptFlag = pcMsgTemp[0];
    EB_LOGD("[%s.%d] encryptFlag=%d\n", __FUNCTION__, __LINE__, encryptFlag);
    if (encryptFlag != 1) {
        return -6;
    }
    pcMsgTemp++;

    //保留字段
    //todo...
    pcMsgTemp += 16;

    //消息序号
    //todo...
    memcpy((void*)&msgNum, (void*)pcMsgTemp, 4);
    msgNum = ntohl(msgNum);
    EB_LOGD("[%s.%d] msgNum=%d\n", __FUNCTION__, __LINE__, msgNum);
    pcMsgTemp += 4;

    //消息类型
    memcpy((void*)data.msgType, (void*)pcMsgTemp, EASYBUS_MSGTYPE_MAX_LEN);
    EB_LOGD("[%s.%d] msgType=%s\n", __FUNCTION__, __LINE__, data.msgType);
    pcMsgTemp += EASYBUS_MSGTYPE_MAX_LEN;

    //消息数据长度
    memcpy((void*)&dataLen, (void*)pcMsgTemp, 4);
    data.msgDataSize = ntohl(dataLen);
    EB_LOGD("[%s.%d] data.msgDataSize=%d\n", __FUNCTION__, __LINE__, data.msgDataSize);
    pcMsgTemp += 4;

    //消息数据
    pData = pcMsgTemp;
    pcMsgTemp += data.msgDataSize;

    //校验码
    if (crcFlag == 1) {
        unsigned int crcGen;

        memcpy((void*)&crcRecv, (void*)pcMsgTemp, 4);
        crcRecv = ntohl(crcRecv);
        crcGen = easy_crc32(msg, pcMsgTemp - msg);
        EB_LOGD("[%s.%d] crcRecv:0x%x, crcGen:0x%x \n", __FUNCTION__, __LINE__, crcRecv, crcGen);
        if (crcRecv != crcGen) {
            printf("[%s.%d] crc error !!!\n", __FUNCTION__, __LINE__);
            return -7;
        }
    }
    pcMsgTemp += 4;

    //加密码
    if (encryptFlag == 1) {
        unsigned int encryptRecv, encryptGen;
        unsigned int crcTmp;
        char *p, *q;

        memcpy((void*)&encryptRecv, (void*)pcMsgTemp, 4);
        encryptRecv = ntohl(encryptRecv);
        p = (char *)&crcRecv;
        q = (char *)&encryptGen;
        q[1] = p[3]>>2;
        q[0] = p[2]<<3;
        q[3] = p[1]>>4;
        q[2] = p[0]<<5;
        EB_LOGD("[%s.%d] encryptRecv:0x%x, encryptGen:0x%x \n", __FUNCTION__, __LINE__, encryptRecv, encryptGen);
        if (encryptRecv != encryptGen) {
            printf("[%s.%d] encrypt error !!!\n", __FUNCTION__, __LINE__);
            return -8;
        }
    }
    pcMsgTemp += 4;    

    if (pcMsgTemp - msg != msgLen) {
        printf("[%s.%d] parse error !!!\n", __FUNCTION__, __LINE__);
        return -9;
    }
    
    //
    memcpy((void*)data.msgData, (void*)pData, data.msgDataSize);
    ts_printf("##EASYDATA [%s.%d] msgData=[%s]\n", 
                        __FUNCTION__, __LINE__, data.msgData);
    if (strcmp(data.msgType, EASY_BUS_MSG_TYPE_SENSOR) == 0 && pOutdata != NULL) {
        memcpy((void*)pOutdata, (void*)&data, sizeof(data));
    } else {
        if (msgCb != NULL) {
            msgCb(&data);
        }
    }

    return 0;
}

int easy_ctrl_compose_frame(EasybusMsg *data)
{
    char *frame, *tmp;
    int contentLen;
    char crcFlag = 1;
    unsigned int crcGen;
    char encryptFlag = 1;

    EB_LOGD("[%s.%d] start\n", __FUNCTION__, __LINE__);

    if (data == NULL) return -1;
    
    if ((frame = malloc(EASY_BUS_FRAME_MAX_SIZE)) == NULL) {
        return -2;
    }
    memset(frame, 0, EASY_BUS_FRAME_MAX_SIZE);

    tmp = frame;

    //帧起始符
    memcpy((void*)tmp, (void*)s_aucMsgHead, 4);
    tmp += 4;

    //主板本号
    tmp[0] = EASYBUS_MAIN_VER;
    tmp++;

    //次板本号
    tmp[0] = EASYBUS_SUB_VER;
    tmp++;

    //校验标识
    if (crcFlag == 1) {
        tmp[0] = 1;
    }
    tmp++;

    //加密标识
    if (encryptFlag == 1) {
        tmp[0] = 1;
    }
    tmp++;

    //保留字段
    //todo...
    tmp += 16;

    //消息序号
    //todo...
    tmp += 4;

    //消息类型
    memcpy((void*)tmp, (void*)data->msgType, EASYBUS_MSGTYPE_MAX_LEN);
    tmp += EASYBUS_MSGTYPE_MAX_LEN;

    //消息数据长度
    EB_LOGD("[%s.%d] data->msgDataSize:%d \n", __FUNCTION__, __LINE__, data->msgDataSize);
    contentLen = htonl(data->msgDataSize);
    memcpy((void*)tmp, (void*)&contentLen, 4);
    tmp += 4;

    //消息数据
    memcpy((void*)tmp, (void*)data->msgData, data->msgDataSize);
    tmp += data->msgDataSize;

    //校验码
    if (crcFlag == 1) {
        unsigned int crcTmp;
        
        crcGen = easy_crc32(frame, tmp - frame);
        EB_LOGD("[%s.%d] crcGen:0x%x \n", __FUNCTION__, __LINE__, crcGen);
        crcTmp = htonl(crcGen);
        memcpy((void*)tmp, (void*)&crcTmp, 4);
    }
    tmp += 4;

    //加密码
    if (encryptFlag == 1) {
        unsigned int encryptGen;
        unsigned char *p, *q;

        p = (char *)&crcGen;
        q = (char *)&encryptGen;
        q[1] = p[3]>>2;
        q[0] = p[2]<<3;
        q[3] = p[1]>>4;
        q[2] = p[0]<<5;
        EB_LOGD("[%s.%d] encryptGen:0x%x \n", __FUNCTION__, __LINE__, encryptGen);
        encryptGen = htonl(encryptGen);
        memcpy((void*)tmp, (void*)&encryptGen, 4);
    }
    tmp += 4;    

    EB_PRINT_MEM("the composed frame:", frame, tmp - frame);

    thr_udp_send(data->remoteAddr.ip, data->remoteAddr.port, frame, tmp - frame);

    if (frame != NULL) free(frame);

    return 0;
}

