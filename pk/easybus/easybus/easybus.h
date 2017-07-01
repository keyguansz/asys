#ifndef _EASYBUS_H_
#define _EASYBUS_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*
*  错误值
*/
enum {
    EasybusErr_NoError                  =   0,
    EasybusErr_Unknown                  =  -1,
    EasybusErr_NoMemory                 =  -2,
    EasybusErr_BadParam                 =  -3,
    EasybusErr_Invalid                  =  -4,
    EasybusErr_OpenCtrlConnFail         =  -5,
    EasybusErr_OpenMonitorConnFail      =  -6,
    EasybusErr_AttachFail               =  -7,
    EasybusErr_SocketPairFail           =  -8,
    EasybusErr_DetachFail               =  -9,
    EasybusErr_InterestFail             =  -10,
    EasybusErr_CtrlConnNoExist          =  -11,
    EasybusErr_MonitorConnNoExist       =  -12,
    EasybusErr_ReceiveFail              =  -13,
    EasybusErr_ReceiveEOF               =  -14,
    EasybusErr_SendFail                 =  -15,

};

#define EASYBUS_ADDR_MAX_LEN (24)
#define EASYBUS_MSGTYPE_MAX_LEN (16)
#define EASYBUS_ATTR_PAIR_MAX_LEN (512)
#define EASYBUS_MSGDATA_MAX_LEN (1024)

typedef struct {
  char ip[EASYBUS_ADDR_MAX_LEN]; /*ip地址，格式: 192.168.1.2*/
  unsigned short port; /*端口号*/
}EasybusAddr;

typedef struct {
  EasybusAddr remoteAddr; /*远端（如手机、PAD）应用的地址*/
  char mode; /*模式：单播，广播，通知*/
  char msgType[EASYBUS_MSGTYPE_MAX_LEN + 1]; /*消息类型*/
  char msgData[EASYBUS_MSGDATA_MAX_LEN]; /*消息数据*/
  int msgDataSize; /*消息数据大小*/
}EasybusMsg;


/***************************************************************************
*  函      数:  int easybus_attach(void)
*  功      能:  attach到easybus总线上
*  参      数:  无
*  返  回  值:  <0 失败，0 成功
*  备      注:  
****************************************************************************/
int easybus_attach(void);

/***************************************************************************
*  函      数:  int easybus_detach(void)
*  功      能:  从easybus总线上detach  
*  参      数:  无
*  返  回  值:  <0 失败，0 成功
*  备      注:  
****************************************************************************/
int easybus_detach(void);

/***************************************************************************
*  函      数:  int easybus_interest(char *msgType)
*  功      能:  注册感兴趣的消息类型
*  参      数:  msgType 消息类型，ASCII编码，最大不能超过16字节
*  返  回  值:  <0 失败，0成功
*  备      注:  该接口为阻塞方式
****************************************************************************/
int easybus_interest(char *msgType);

/***************************************************************************
*  函      数:  int easybus_receive(EasybusMsg *revMsg)
*  功      能:  从easybus接收数据
*  参      数:  revMsg 接收的消息
*  返  回  值:  <0 失败，0 成功
*  备      注:  该接口为阻塞方式
****************************************************************************/
int easybus_receive(EasybusMsg *revMsg);

/***************************************************************************
*  函      数:  int easybus_send(EasybusMsg *sendMsg)
*  功      能:  通过easybus发送数据
*  参      数:  sendMsg 发送的消息
*  返  回  值:  <0 失败，0 成功
*  备      注:   该接口为阻塞方式
****************************************************************************/
int easybus_send(EasybusMsg *sendMsg);

#ifdef  __cplusplus
}
#endif

#endif/*_EASYBUS_H_*/
