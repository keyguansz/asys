#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>

#include <semaphore.h>
//#include <cutils/properties.h>

#include "easy_common.h"

#include "easy_ctrl_vinput.h"
#include "common.h"
#include "easy_ctrl_iface.h"
#include "easy_ctrl_eloop.h"

#define SERVER_LOCAL_PORT        (12170)
#define EASY_CTRL_SEM "EasyCtrol Service Semaphore"
static int s_nEasyCtrlInit = NOT_INITED;
struct eloop_data eloop_s;
sem_t *g_easy_ctrl_sem = NULL;



int easybus_init_sem()
{
    void *mem;
    sem_unlink(EASY_CTRL_SEM);
    g_easy_ctrl_sem = sem_open(EASY_CTRL_SEM, O_CREAT, O_RDWR, 0);
    if(NULL == g_easy_ctrl_sem)    
    {
        EB_LOGD("[%s.%d] sem_open fail\n", __FUNCTION__, __LINE__);
        return -1;    
    }

    #if 0
    int shm_id = shmget(SHM_ID, sizeof(sem_t), 0666|IPC_CREAT);
    mem = (sem_t*)shmat(shm_id, (const void*)0, 0);
    memcpy(mem, g_easy_ctrl_sem, sizeof(sem_t));

    sem_wait(g_easy_ctrl_sem);
    #endif

    return 0;
}

void easybus_notify_service_started()
{
    EB_LOGD("[%s.%d] easy_ctrl notify service done\n", __FUNCTION__, __LINE__);
    sem_post(g_easy_ctrl_sem);
}

int easy_ctrl_init()
{
    struct easy_ctrl_svc easy_s;

    EB_LOGD("[%s.%d] easy_ctrl_init start !!!\n", __FUNCTION__, __LINE__);

    if(s_nEasyCtrlInit == BEEN_INITED)
        return 0;

    memset(&eloop_s, 0, sizeof(eloop_s));

    if (eloop_init(&eloop_s, &easy_s)) {
        printf("Failed to initialize event loop");
        return -1;
    }

    memset(&easy_s, 0, sizeof(easy_s));

    easy_s.ctrl_iface = easy_ctrl_iface_init(&eloop_s, &easy_s);
    if (easy_s.ctrl_iface == NULL) {
        return -1;
    }

    s_nEasyCtrlInit = BEEN_INITED;

    /*通知easycontrol服务已经启动完成*/
    easybus_notify_service_started();
    eloop_run(&eloop_s);

    eloop_destroy(&eloop_s);

    return 0;    
}

void easy_ctrl_all_msg_cb(EasybusMsg *data)
{
    char *buf = NULL;
    int ret = -1;

    EB_LOGD("[%s.%d] start\n", __FUNCTION__, __LINE__);
    
    do
    {
        if (data == NULL) {
            ret = -1;
            break;
        }

        if ((buf = malloc(EASY_BUS_BUFF_MAX_LEN)) == 0) {
            ret = -2;
            break;
        }
        memset(buf, 0, EASY_BUS_BUFF_MAX_LEN);
        
        sprintf(buf, "addr.ip=%s,addr.port=%d,msgType=%s,msgData=%s", data->remoteAddr.ip, 
            data->remoteAddr.port, data->msgType, data->msgData);

        easy_msg(eloop_s.user_data, data->msgType, buf, strlen(buf));        
    }while(0);

    if (buf != NULL) free(buf);

    EB_LOGD("[%s.%d] end\n", __FUNCTION__, __LINE__);
}

void easy_ctrl_msg_dispatch(EasybusMsg *data)
{
    EB_LOGD("[%s.%d] start\n", __FUNCTION__, __LINE__);
    #if 0
    easy_ctrl_vinput_msg_cb(data);
    #endif
    easy_ctrl_all_msg_cb(data);
    EB_LOGD("[%s.%d] end\n", __FUNCTION__, __LINE__);
}

int main()
{
    int ret = -1;

    EB_LOGD("[%s.%d] easyControd start !!!\n", __FUNCTION__, __LINE__);

    /*创建信号量*/
    easybus_init_sem();
    //创建udp server
    ret = easy_ctrl_creat_udp_server(SERVER_LOCAL_PORT, easy_ctrl_decompose_frame);
    if (ret != 0) {
        printf("create udp server error !!!\n");
        return -1;
    }
    easy_ctrl_register_msg_cb(easy_ctrl_msg_dispatch);
    
    //key, mouse, tc...event init
    #if 0
    ret = easy_ctrl_vinput_Init();
    #endif

    ret = easy_ctrl_init();
    if (ret != 0) {
        printf("init error !!!\n");
        return -1;
    }

    EB_LOGD("[%s.%d] easyControd end !!!\n", __FUNCTION__, __LINE__);
    return 0;
}

