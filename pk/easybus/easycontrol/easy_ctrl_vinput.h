#include "easy_udpserver.h"
#include "easy_parsemsg.h"


#define IOCTL_MOUSE_STATUS  0
#define IOCTK_KBD_STATUS    1
#define IOCTK_TC_STATUS     3
#define IOCTK_MUTITC_STATUS 4

typedef enum
{
    BEEN_INITED   = 0,    
    NOT_INITED    = 1
}VINPUT_INIT_VALUE;

#define Vinput_FILE                ("/dev/vinput")
#define CHECK_TIMES             (100)
#define CHECK_TIME_INTERVAL     (10*1000) //10ms

/*mouse clickType*/
typedef enum
{
    MOUSE_ACTION_MOVE = 1, /*ÒÆ¶¯*/
    MOUSE_RIGHT_SINGLE_CLICK,
    MOUSE_RIGHT_DOUBLE_CLICK,
    MOUSE_RIGHT_DOWN,
    MOUSE_RIGHT_UP,
    MOUSE_RGIHT_DOWN_MOVE,
    MOUSE_LEFT_SINGLE_CLICK,
    MOUSE_LEFT_DOUBLE_CLICK,
    MOUSE_LEFT_DOWN,
    MOUSE_LEFT_UP,
    MOUSE_LEFT_DOWN_MOVE,
    
    MOUSE_WHEEL=0x306,

    TC_DOWN=0x00,
    TC_UP  =0x01,
    TC_MOVE=0x02,
    
    KEYBOARD_EVENT=0x107,
    KEYBOARD_DOWN_UP=0x108
}TC_MOUSE_CLICKTYPE_E;

#define MAX_FINGER_NUM   5 

typedef struct
{
    int nDx;
    int nDy;
    int nPress;
}PointinfoMsg_S;

typedef struct
{
    int nFingerNum;
    PointinfoMsg_S sFinger[MAX_FINGER_NUM];
}MutitcMsg_S;


#define EASY_BUS_MSG_TYPE_DEVICE  "easydeviceaccess"
#define EASY_BUS_MSG_TYPE_KEY  "easykey"
#define EASY_BUS_MSG_TYPE_MOUSE  "easymouse"
#define EASY_BUS_MSG_TYPE_MULTIPOINTTOUCH  "easytouch"
#define EASY_BUS_MSG_TYPE_SENSOR  "easysensor"
#define EASY_BUS_MSG_TYPE_SCREEN  "easyscreen"
#define EASY_BUS_MSG_TYPE_LOGIN  "easylogin"
#define EASY_BUS_MSG_TYPE_LOGOUT  "easylogout"
#define EASY_BUS_MSG_TYPE_HEARTBEAT  "easyheartbeat"


typedef enum 
{
    EM_AC_MSG_TYPE_START           = 0x0000,
    EM_AC_MSG_TYPE_DEVICE_EVENT    = 0x0001,
    EM_AC_MSG_TYPE_KEY_EVENT       = 0x0002,
    EM_AC_MSG_TYPE_MOUSE_EVENT     = 0x0003,
    EM_AC_MSG_TYPE_MUTITC_EVENT    = 0x0004,
    EM_AC_MSG_TYPE_SENSOR_EVENT    = 0x0009,
    EM_AC_MSG_TYPE_LOGIN_EVENT     = 0x00A1,
    EM_AC_MSG_TYPE_REPLY_LOGIN     = 0x0021,
    EM_AC_MSG_TYPE_LOGOUT_EVENT    = 0x00A2,
    EM_AC_MSG_TYPE_REPLY_LOGOUT    = 0x0022,
    EM_AC_MSG_TYPE_HTBEAT_EVENT    = 0x00A3,
    EM_AC_MSG_TYPE_REPLY_HTBEAT    = 0x0023,
    EM_AC_MSG_TYPE_END
}STPMsgType_E;

extern int easy_ctrl_vinput_Init(void);
extern int easy_ctrl_vinput_deInit(void);
extern void easy_ctrl_vinput_msg_cb(EasybusMsg *data);
