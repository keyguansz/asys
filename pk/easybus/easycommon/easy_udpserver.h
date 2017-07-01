#ifndef _EASY_UDPSERVER_H_
#define _EASY_UDPSERVER_H_

#include "easy_common.h"

typedef int (*pfParseMsgFuncs)(char *msg, int msgLen, EasybusAddr *addr, EasybusMsg *pOutdata);

int easy_ctrl_creat_udp_server(int nPort,pfParseMsgFuncs fParseMsgFunc);

#endif/*_EASY_UDPSERVER_H_*/
