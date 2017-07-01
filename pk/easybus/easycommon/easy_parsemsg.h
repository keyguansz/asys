#ifndef _EASY_PARSEMSG_H_
#define _EASY_PARSEMSG_H_

#include "easy_common.h"


#define EASY_BUS_FRAME_MAX_SIZE (1024*2)

typedef void (*pfMsgCallback)(EasybusMsg *data);


extern void easy_ctrl_register_msg_cb(pfMsgCallback *cb);

extern int easy_ctrl_decompose_frame(char *msg, int msgLen, EasybusAddr *addr, 
    EasybusMsg *pOutdata);

#endif/*_EASY_PARSEMSG_H_*/
