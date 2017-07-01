#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>   
#include <sys/stat.h> 
#include <sys/socket.h>
#include <sys/un.h>

#include "easy_common.h"
#include "easy_ctrl_vinput.h"


static int s_fdVinput = -1;
static int s_nShiftKeyValue = 0x3B;
static int s_nVinputInit  = NOT_INITED;


void ReportKeyEvent(int nKeyValue, int nState)
{
    int anKeyData[4] = {0};
    int ii = 0;
    
    anKeyData[0] = nKeyValue;
    anKeyData[1] = nState;

    anKeyData[2]=0;
    anKeyData[3]=0;

    EB_LOGD("[%s.%d] nKeyValue: %d, nState: %d\n", __FUNCTION__, __LINE__, nKeyValue, nState);
    ioctl(s_fdVinput, IOCTK_KBD_STATUS, (void *)anKeyData);
    
    return;
}

/*    
* 向输入子系统汇报鼠标事件情况，以便作出反应。    
* data 数组的第0个字节：bit 0、1、2、3、4分别代表左、右、中、SIDE、EXTRA键的按下情况；    
* data 数组的第1个字节：表示鼠标的水平位移；  
* data 数组的第2个字节：表示鼠标的垂直位移；  
* data 数组的第3个字节：REL_WHEEL位移。  
*/ 

void ReportMouseState(int nKey,int nDx,int nDy)
{
    int anMouseData[4];
    
    anMouseData[0] = nKey;
    anMouseData[1] = nDx;
    anMouseData[2] = nDy;
    anMouseData[3] = 0;

    EB_LOGD("[%s.%d] key: %d, x: %d, y: %d\n", __FUNCTION__, __LINE__, nKey, nDx, nDy);
    ioctl(s_fdVinput, IOCTL_MOUSE_STATUS, (void *)anMouseData);
}

void ReportMouseEvent(char *pcDataBuf, int nDataLen)
{
    char *pcDataTmp = pcDataBuf;
    int clickType;
    int mode, x, y;
    char clickTypeStr[3];
    char modeStr[12], xStr[12], yStr[12];
    char *str1 = NULL;
    char *str2 = NULL;

    if(pcDataBuf == NULL)
    {
        return;
    }

    str1 = pcDataTmp + strlen("clicktype=");
    if (str1 - pcDataTmp >= nDataLen) return;
    str2 = strstr(str1, "&mode=");
    if (str2 == NULL) return;
    memset(clickTypeStr, 0, sizeof(clickTypeStr));
    memcpy((void*)clickTypeStr, str1, str2 - str1);
    clickType = atoi(clickTypeStr);
    EB_LOGD("clickType = %d\n", clickType);
    
    str1 = str2 + strlen("&mode=");
    if (str1 - pcDataTmp >= nDataLen) return;
    str2 = strstr(str1, "&x=");
    if (str2 == NULL) return;
    memset(modeStr, 0, sizeof(modeStr));
    memcpy((void*)modeStr, str1, str2 - str1);
    mode = atoi(modeStr);
    EB_LOGD("mode = %i\n", mode);

    str1 = str2 + strlen("&x=");
    if (str1 - pcDataTmp >= nDataLen) return;
    str2 = strstr(str1, "&y=");
    if (str2 == NULL) return;
    memset(xStr, 0, sizeof(xStr));
    memcpy((void*)xStr, str1, str2 - str1);
    x = atoi(xStr);
    EB_LOGD("x = %i\n", x);

    str1 = str2 + strlen("&y=");
    if (str1 - pcDataTmp >= nDataLen) return;
    memset(yStr, 0, sizeof(yStr));
    strcpy(yStr, str1);
    y = atoi(yStr);
    EB_LOGD("y = %i\n", y);

    switch(clickType)
    {
        case MOUSE_ACTION_MOVE:
        {
            ReportMouseState(0, x, y);
        }
        break;
        case MOUSE_RIGHT_SINGLE_CLICK:
        {                    
            ReportMouseState(2,0,0);
            ReportMouseState(0,0,0);
        }
        break;
        case MOUSE_RIGHT_DOUBLE_CLICK:
        {
            ReportMouseState(2,0,0);
            ReportMouseState(0,0,0);
            ReportMouseState(2,0,0);
            ReportMouseState(0,0,0);
        }
        break;
        case MOUSE_RIGHT_DOWN:
        {
            ReportMouseState(2,0,0);
        }
        break;
        case MOUSE_RIGHT_UP:
        {
            ReportMouseState(0,0,0);
        }
        break;
        case MOUSE_RGIHT_DOWN_MOVE:        
        {            
            ReportMouseState(2, x, y);
        }
        break;
        case MOUSE_LEFT_SINGLE_CLICK:
        {
            ReportMouseState(1,0,0);
            ReportMouseState(0,0,0);
        }
        break;
        case MOUSE_LEFT_DOUBLE_CLICK:
        {
            ReportMouseState(1,0,0);
            ReportMouseState(0,0,0);
            ReportMouseState(1,0,0);
            ReportMouseState(0,0,0);
        }
        break;            
        case MOUSE_LEFT_DOWN:
        {
            ReportMouseState(1,0,0);
        }
        break;
        case MOUSE_LEFT_UP:
        {
            ReportMouseState(0,0,0);
        }
        break;
        case MOUSE_LEFT_DOWN_MOVE:
        {
            ReportMouseState(1, x, y);
        }
        break;        
        default:
        {
            EB_LOGD("No such click type\n");
        }
        break;        
    }
}
static float floatmySwitch( char* pstr)
{
    return *(float *)pstr;
}

static int ReportTcEvent(int pres,int x,int y)
{    
    int tcdata[4];    

    tcdata[0]=x;    
    tcdata[1]=y;    
    tcdata[2]=pres;    
    tcdata[3]=0;    

    EB_LOGD("[%s.%d] pres: %d, x: %d, y: %d\n", __FUNCTION__, __LINE__, pres, x, y);
    ioctl(s_fdVinput,IOCTK_TC_STATUS,(void *)tcdata);    
    return 0;
}

void ReportMutiTcEvent(char *pcDataBuf, int nDataLen)
{
    char *pcDataTmp = pcDataBuf;
    MutitcMsg_S sMutitcMsg;
    int fingerNum, press;
    int mode, x, y;
    char fingerNumStr[12], pressStr[12];
    char modeStr[12], xStr[12], yStr[12];
    char *str1 = NULL;
    char *str2 = NULL;
    int i = 0;
    
    if(pcDataBuf == NULL)
    {
        return;
    }

    str1 = pcDataTmp + strlen("mode=");
    if (str1 - pcDataTmp >= nDataLen) return;
    str2 = strstr(str1, "&fingernum=");
    if (str2 == NULL) return;
    memset(modeStr, 0, sizeof(modeStr));
    memcpy((void*)modeStr, str1, str2 - str1);
    mode = atoi(modeStr);
    EB_LOGD("mode = %i\n", mode);
    
    str1 = str2 + strlen("&fingernum=");
    if (str1 - pcDataTmp >= nDataLen) return;
    str2 = strstr(str1, "&x=");
    if (str2 == NULL) return;
    memset(fingerNumStr, 0, sizeof(fingerNumStr));
    memcpy((void*)fingerNumStr, str1, str2 - str1);
    fingerNum = atoi(fingerNumStr);
    EB_LOGD("fingerNum = %d\n", fingerNum);

    memset(&sMutitcMsg, 0, sizeof(MutitcMsg_S));
    sMutitcMsg.nFingerNum = fingerNum;
    
    for(i = 0; i < sMutitcMsg.nFingerNum; i++)
    {        
        str1 = str2 + strlen("&x=");
        if (str1 - pcDataTmp >= nDataLen) return;
        str2 = strstr(str1, "&y=");
        if (str2 == NULL) return;
        memset(xStr, 0, sizeof(xStr));
        memcpy((void*)xStr, str1, str2 - str1);
        sMutitcMsg.sFinger[i].nDx = atoi(xStr);
        EB_LOGD("x = %d\n", sMutitcMsg.sFinger[i].nDx);
        
        str1 = str2 + strlen("&y=");
        if (str1 - pcDataTmp >= nDataLen) return;
        str2 = strstr(str1, "&press=");
        if (str2 == NULL) return;
        memset(yStr, 0, sizeof(yStr));
        memcpy((void*)yStr, str1, str2 - str1);
        sMutitcMsg.sFinger[i].nDy = atoi(yStr);
        EB_LOGD("y = %d\n", sMutitcMsg.sFinger[i].nDy);
        
        str1 = str2 + strlen("&press=");
        if (str1 - pcDataTmp >= nDataLen) return;
        if ((str2 = strstr(str1, "&x=")) == NULL) {
            memset(pressStr, 0, sizeof(pressStr));
            strcpy(pressStr, str1);
        }else {
            memset(pressStr, 0, sizeof(pressStr));
            memcpy((void*)pressStr, str1, str2 - str1);
        }
        sMutitcMsg.sFinger[i].nPress = atoi(pressStr);
        EB_LOGD("press = %d\n", sMutitcMsg.sFinger[i].nPress);
        EB_LOGD("str2 = %s\n", str2);
        
        if(sMutitcMsg.sFinger[i].nDx >1280 || sMutitcMsg.sFinger[i].nDy > 720 || sMutitcMsg.sFinger[i].nDx < 0 || sMutitcMsg.sFinger[i].nDy < 0)
        {
            EB_LOGD("Out of Range(1280*720) or < 0\n");
             return;
        }
        EB_LOGD("msg multi touch point%d : x: %d y: %d press: %d \n", i, sMutitcMsg.sFinger[i].nDx, 
        sMutitcMsg.sFinger[i].nDy, sMutitcMsg.sFinger[i].nPress);
    }

    if (sMutitcMsg.nFingerNum > 1) {
        ioctl(s_fdVinput, IOCTK_MUTITC_STATUS, (void *)&sMutitcMsg);
    } else {
        ReportTcEvent(sMutitcMsg.sFinger[0].nPress, sMutitcMsg.sFinger[0].nDx,
            sMutitcMsg.sFinger[0].nDy);
    }
    return;
}

#define SENSOR_TYPE_ACCELERATION    1
#define SENSOR_TYPE_ORIENTATION    3
#define SENSOR_TYPE_GYROSCOPE     4
#define SENSOR_TYPE_GRAVITY    9
#define SENSOR_TYPE_LINEAR_ACCELERATION    10
#define SENSOR_TYPE_ROTATION_VECTOR    11

void ReportSensorEvent(char *pcDataBuf, int nDataLen)
{
    char *pcDataTmp = pcDataBuf;
    int dataType, sensorType, accuracyTmp;
    char dataTypeStr[12], sensorTypeStr[12], accuracyStr[12];
    char *str1 = NULL;
    char *str2 = NULL;
    
    str1 = pcDataTmp + strlen("datatype=");
    if (str1 - pcDataTmp >= nDataLen) return;
    str2 = strstr(str1, "&sensortype=");
    if (str2 == NULL) return;
    memset(dataTypeStr, 0, sizeof(dataTypeStr));
    memcpy((void*)dataTypeStr, str1, str2 - str1);
    dataType = atoi(dataTypeStr);
    EB_LOGD("dataType = %d\n", dataType);
        
    str1 = str2 + strlen("&sensortype=");
    if (str1 - pcDataTmp >= nDataLen) return;
    str2 = strstr(str1, "&accuracy=");
    if (str2 == NULL) return;
    memset(sensorTypeStr, 0, sizeof(sensorTypeStr));
    memcpy((void*)sensorTypeStr, str1, str2 - str1);
    sensorType = atoi(sensorTypeStr);
    EB_LOGD("sensorType = %d\n", sensorType);

    if (sensorType == SENSOR_TYPE_ACCELERATION || sensorType == SENSOR_TYPE_GYROSCOPE
        || sensorType == SENSOR_TYPE_GRAVITY || sensorType == SENSOR_TYPE_LINEAR_ACCELERATION
        || sensorType == SENSOR_TYPE_ROTATION_VECTOR) {
        float x, y, z, value;
        char xStr[12], yStr[12], zStr[12], valueStr[12];
        
        str1 = str2 + strlen("&accuracy=");
        if (str1 - pcDataTmp >= nDataLen) return;
        str2 = strstr(str1, "&x=");
        if (str2 == NULL) return;
        memset(accuracyStr, 0, sizeof(accuracyStr));
        memcpy((void*)accuracyStr, str1, str2 - str1);
        accuracyTmp = atoi(accuracyStr);
        EB_LOGD("accuracy = %d\n", accuracyTmp);
    
        str1 = str2 + strlen("&x=");
        if (str1 - pcDataTmp >= nDataLen) return;
        str2 = strstr(str1, "&y=");
        if (str2 == NULL) return;
        memset(xStr, 0, sizeof(xStr));
        memcpy((void*)xStr, str1, str2 - str1);
        x = atof(xStr);
        EB_LOGD("x = %f\n", x);
    
        str1 = str2 + strlen("&y=");
        if (str1 - pcDataTmp >= nDataLen) return;
        str2 = strstr(str1, "&z=");
        if (str2 == NULL) return;
        memset(yStr, 0, sizeof(yStr));
        memcpy((void*)yStr, str1, str2 - str1);
        y = atof(yStr);
        EB_LOGD("y = %f\n", y);
    
        str1 = str2 + strlen("&z=");
        if (str1 - pcDataTmp >= nDataLen) return;
        memset(zStr, 0, sizeof(zStr));
        if ((str2 = strstr(str1, "&value=")) == NULL) {
            strcpy((void*)zStr, str1);
            z = atof(zStr);
            EB_LOGD("z = %f\n", z);
        } else {
            memcpy((void*)zStr, str1, str2 - str1);
            z = atof(zStr);
            EB_LOGD("z = %f\n", z);

             str1 = str2 + strlen("&value=");
             if (str1 - pcDataTmp >= nDataLen) return;
             memset(valueStr, 0, sizeof(valueStr));
             strcpy((void*)valueStr, str1);
             value = atof(valueStr);
             EB_LOGD("value = %f\n", value);
        }
    } else if (sensorType == SENSOR_TYPE_ORIENTATION) {
        float azimuth, pitch, roll;
        char azimuthStr[12], pitchStr[12], rollStr[12];
        
        str1 = str2 + strlen("&accuracy=");
        if (str1 - pcDataTmp >= nDataLen) return;
        str2 = strstr(str1, "&azimuth=");
        if (str2 == NULL) return;
        memset(accuracyStr, 0, sizeof(accuracyStr));
        memcpy((void*)accuracyStr, str1, str2 - str1);
        accuracyTmp = atoi(accuracyStr);
        EB_LOGD("accuracy = %d\n", accuracyTmp);
    
        str1 = str2 + strlen("&azimuth=");
        if (str1 - pcDataTmp >= nDataLen) return;
        str2 = strstr(str1, "&pitch=");
        if (str2 == NULL) return;
        memset(azimuthStr, 0, sizeof(azimuthStr));
        memcpy((void*)azimuthStr, str1, str2 - str1);
        azimuth = atof(azimuthStr);
        EB_LOGD("azimuth = %f\n", azimuth);
    
        str1 = str2 + strlen("&pitch=");
        if (str1 - pcDataTmp >= nDataLen) return;
        str2 = strstr(str1, "&roll=");
        if (str2 == NULL) return;
        memset(pitchStr, 0, sizeof(pitchStr));
        memcpy((void*)pitchStr, str1, str2 - str1);
        pitch = atof(pitchStr);
        EB_LOGD("pitch = %f\n", pitch);
    
        str1 = str2 + strlen("&roll=");
        if (str1 - pcDataTmp >= nDataLen) return;
        memset(rollStr, 0, sizeof(rollStr));
        strcpy((void*)rollStr, str1);
        roll = atof(rollStr);
        EB_LOGD("roll = %f\n", roll);    
    }

}

void ReportLogEvent(int nMsgType, char *pcDataBuf, int nDataLen)
{
    return;
}

void easy_ctrl_vinput_msg_cb(EasybusMsg *data)
{        
    EB_LOGD("[%s.%d] start\n", __FUNCTION__, __LINE__);
    
    /*是否已经初始化*/
    if(s_nVinputInit == NOT_INITED)
    {
        EB_LOGD("key Vinput not init \n");
        easy_ctrl_vinput_Init();
        if(s_nVinputInit == NOT_INITED)
        {
            printf("key Vinput init failed\n");
            return;
        }
    }

    EB_LOGD("[%s.%d] data->msgType:%s\n", __FUNCTION__, __LINE__, data->msgType);

    if (strcmp(data->msgType, EASY_BUS_MSG_TYPE_KEY) == 0) {
        char *pcDataTmp = data->msgData;
        int contentLen = strlen(data->msgData);
        int keyValue;
        int shiftFlag;
        char keyValueStr[12];
        char shiftFlagStr[2];
        char *str1 = NULL;
        char *str2 = NULL;

        str1 = pcDataTmp + strlen("keyvalue=");
        if (str1 - pcDataTmp >= contentLen) return;
        str2 = strstr(str1, "&functionkey=");
        if (str2 == NULL) return;
        memset(keyValueStr, 0, sizeof(keyValueStr));
        memcpy((void*)keyValueStr, str1, str2 - str1);
        keyValue = atoi(keyValueStr);
        EB_LOGD("keyValue = %d\n", keyValue);
        
        str1 = str2 + strlen("&functionkey=");
        if (str1 - pcDataTmp >= contentLen) return;
        memset(shiftFlagStr, 0, sizeof(shiftFlagStr));
        strcpy(shiftFlagStr, str1);
        shiftFlag = atoi(shiftFlagStr);
        EB_LOGD("shiftFlag = %d\n", shiftFlag);

        if(shiftFlag == 1)
        {
            ReportKeyEvent(s_nShiftKeyValue, 1);
            ReportKeyEvent(keyValue, 1);
            ReportKeyEvent(s_nShiftKeyValue, 0);
            ReportKeyEvent(keyValue, 0);
        }
        else
        {
            ReportKeyEvent(keyValue, 1);
            ReportKeyEvent(keyValue, 0);
        }
    } else if (strcmp(data->msgType, EASY_BUS_MSG_TYPE_MOUSE) == 0) {
        ReportMouseEvent(data->msgData, strlen(data->msgData));
    } else if (strcmp(data->msgType, EASY_BUS_MSG_TYPE_MULTIPOINTTOUCH) == 0) {
        ReportMutiTcEvent(data->msgData, strlen(data->msgData));
    } else if (strcmp(data->msgType, EASY_BUS_MSG_TYPE_SENSOR) == 0) {
        ReportSensorEvent(data->msgData, strlen(data->msgData));
    } else {
    
    }

    EB_LOGD("[%s.%d] end\n", __FUNCTION__, __LINE__);
}

int easy_ctrl_vinput_Init()
{
    
    EB_LOGD("[%s.%d] start\n", __FUNCTION__, __LINE__);
    
    if(s_nVinputInit == NOT_INITED)
    {
        int i;
        for(i = 0; i < CHECK_TIMES; i++)
        {
            s_fdVinput = open(Vinput_FILE, O_RDONLY);
            if(s_fdVinput >= 0)
            {
                break;
            }
            usleep(CHECK_TIME_INTERVAL);
        }
        if(s_fdVinput < 0)
        {
            printf("Error:can't open Vinput,%s, s_fdVinput = %d, errno = %d\n", Vinput_FILE, s_fdVinput, errno);
            return -1;   
        }
        
        s_nVinputInit = BEEN_INITED;

    }

    return 0;
}

int easy_ctrl_vinput_deInit(void)
{
    s_nVinputInit = NOT_INITED;
    if(s_fdVinput >= 0)
    {
        printf("close smart input\n");
        close(s_fdVinput);
        s_fdVinput = -1;
    }
    return 0;
}

