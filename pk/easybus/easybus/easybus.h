#ifndef _EASYBUS_H_
#define _EASYBUS_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*
*  ����ֵ
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
  char ip[EASYBUS_ADDR_MAX_LEN]; /*ip��ַ����ʽ: 192.168.1.2*/
  unsigned short port; /*�˿ں�*/
}EasybusAddr;

typedef struct {
  EasybusAddr remoteAddr; /*Զ�ˣ����ֻ���PAD��Ӧ�õĵ�ַ*/
  char mode; /*ģʽ���������㲥��֪ͨ*/
  char msgType[EASYBUS_MSGTYPE_MAX_LEN + 1]; /*��Ϣ����*/
  char msgData[EASYBUS_MSGDATA_MAX_LEN]; /*��Ϣ����*/
  int msgDataSize; /*��Ϣ���ݴ�С*/
}EasybusMsg;


/***************************************************************************
*  ��      ��:  int easybus_attach(void)
*  ��      ��:  attach��easybus������
*  ��      ��:  ��
*  ��  ��  ֵ:  <0 ʧ�ܣ�0 �ɹ�
*  ��      ע:  
****************************************************************************/
int easybus_attach(void);

/***************************************************************************
*  ��      ��:  int easybus_detach(void)
*  ��      ��:  ��easybus������detach  
*  ��      ��:  ��
*  ��  ��  ֵ:  <0 ʧ�ܣ�0 �ɹ�
*  ��      ע:  
****************************************************************************/
int easybus_detach(void);

/***************************************************************************
*  ��      ��:  int easybus_interest(char *msgType)
*  ��      ��:  ע�����Ȥ����Ϣ����
*  ��      ��:  msgType ��Ϣ���ͣ�ASCII���룬����ܳ���16�ֽ�
*  ��  ��  ֵ:  <0 ʧ�ܣ�0�ɹ�
*  ��      ע:  �ýӿ�Ϊ������ʽ
****************************************************************************/
int easybus_interest(char *msgType);

/***************************************************************************
*  ��      ��:  int easybus_receive(EasybusMsg *revMsg)
*  ��      ��:  ��easybus��������
*  ��      ��:  revMsg ���յ���Ϣ
*  ��  ��  ֵ:  <0 ʧ�ܣ�0 �ɹ�
*  ��      ע:  �ýӿ�Ϊ������ʽ
****************************************************************************/
int easybus_receive(EasybusMsg *revMsg);

/***************************************************************************
*  ��      ��:  int easybus_send(EasybusMsg *sendMsg)
*  ��      ��:  ͨ��easybus��������
*  ��      ��:  sendMsg ���͵���Ϣ
*  ��  ��  ֵ:  <0 ʧ�ܣ�0 �ɹ�
*  ��      ע:   �ýӿ�Ϊ������ʽ
****************************************************************************/
int easybus_send(EasybusMsg *sendMsg);

#ifdef  __cplusplus
}
#endif

#endif/*_EASYBUS_H_*/
