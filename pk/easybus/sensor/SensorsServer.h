#ifndef _SENSORS_SERVER_H_
#define _SENSORS_SERVER_H_

//#include <cutils/log.h>

#define SENSORS_DATA_LEN        (24)
#define UDP_LISTEN_PORT            (12180)

int MakeSocketIn();
int RecvSensorsData(int nSocketFd, void* pvMsg, int nMsgSize);

#endif/*_SENSORS_SERVER_H_*/
