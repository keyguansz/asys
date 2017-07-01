#ifndef _EASY_BUS_COMMON_H_
#define _EASY_BUS_COMMON_H_

#include "easybus.h"

#define LOG_TAG "Easybus"
//#include <cutils/log.h>

#define PROPERTY_VALUE_MAX 16
#define EASY_BUS_BUFF_MAX_LEN (1280)


#ifdef EB_DEBUG
#define  EB_PRINT_MEM  easy_print_mem
#define  EB_LOGD  easy_print //printf(__VA_ARGS__) //printf(__VA_ARGS__)
#else
#define  EB_LOGD(...)  ((void)0)
#define  EB_PRINT_MEM
#endif


extern unsigned long easy_crc32(void *pvStartAddress, unsigned long dwSizeInBytes);
extern void easy_print_mem(char *msg, char *buf, int size);
extern void easy_print(const char *fmt, ...);

#endif
