/*
 * wpa_supplicant/hostapd control interface library
 * Copyright (c) 2004-2007, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */



#ifdef CONFIG_CTRL_IFACE_UNIX
#include <sys/un.h>
#endif /* CONFIG_CTRL_IFACE_UNIX */

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef ANDROID
#include <cutils/sockets.h>
#endif /* ANDROID */

#include "easybus_core.h"

#include "easy_common.h"


#if defined(CONFIG_CTRL_IFACE_UNIX) || defined(CONFIG_CTRL_IFACE_UDP)
#define CTRL_IFACE_SOCKET
#endif /* CONFIG_CTRL_IFACE_UNIX || CONFIG_CTRL_IFACE_UDP */


/**
 * struct easybus_ctrl - Internal structure for control interface library
 *
 * This structure is used by the wpa_supplicant/hostapd control interface
 * library to store internal data. Programs using the library should not touch
 * this data directly. They can only use the pointer to the data structure as
 * an identifier for the control interface connection and use this as one of
 * the arguments for most of the control interface library functions.
 */
struct easybus_ctrl {
#ifdef CONFIG_CTRL_IFACE_UDP
    int s;
    struct sockaddr_in local;
    struct sockaddr_in dest;
    char *cookie;
#endif /* CONFIG_CTRL_IFACE_UDP */
#ifdef CONFIG_CTRL_IFACE_UNIX
    int s;
    struct sockaddr_un local;
    struct sockaddr_un dest;
#endif /* CONFIG_CTRL_IFACE_UNIX */
#ifdef CONFIG_CTRL_IFACE_NAMED_PIPE
    HANDLE pipe;
#endif /* CONFIG_CTRL_IFACE_NAMED_PIPE */
};


#ifdef CONFIG_CTRL_IFACE_UNIX

#ifndef CONFIG_CTRL_IFACE_CLIENT_DIR
#define CONFIG_CTRL_IFACE_CLIENT_DIR "/tmp"
#endif /* CONFIG_CTRL_IFACE_CLIENT_DIR */
#ifndef CONFIG_CTRL_IFACE_CLIENT_PREFIX
#define CONFIG_CTRL_IFACE_CLIENT_PREFIX "easybus_ctrl_"
#endif /* CONFIG_CTRL_IFACE_CLIENT_PREFIX */


struct easybus_ctrl * easybus_ctrl_open(const char *ctrl_path)
{
    struct easybus_ctrl *ctrl;
    static int counter = 0;
    int ret;
    size_t res;
    int tries = 0;

    ctrl = malloc(sizeof(*ctrl));
    if (ctrl == NULL)
        return NULL;
    memset(ctrl, 0, sizeof(*ctrl));

    ctrl->s = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (ctrl->s < 0) {
        free(ctrl);
        return NULL;
    }

    ctrl->local.sun_family = AF_UNIX;
    counter++;
try_again:
    ret = snprintf(ctrl->local.sun_path, sizeof(ctrl->local.sun_path),
              CONFIG_CTRL_IFACE_CLIENT_DIR "/"
              CONFIG_CTRL_IFACE_CLIENT_PREFIX "%d-%d",
              (int) getpid(), counter);
    if (ret < 0 || (size_t) ret >= sizeof(ctrl->local.sun_path)) {
        close(ctrl->s);
        free(ctrl);
        return NULL;
    }
    tries++;
    if (bind(ctrl->s, (struct sockaddr *) &ctrl->local,
            sizeof(ctrl->local)) < 0) {
        if (errno == EADDRINUSE && tries < 2) {
            /*
             * getpid() returns unique identifier for this instance
             * of easybus_ctrl, so the existing socket file must have
             * been left by unclean termination of an earlier run.
             * Remove the file and try again.
             */
            unlink(ctrl->local.sun_path);
            goto try_again;
        }
        close(ctrl->s);
        free(ctrl);
        return NULL;
    }

#ifdef ANDROID
    chmod(ctrl->local.sun_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    chown(ctrl->local.sun_path, AID_SYSTEM, AID_WIFI);
    /*
     * If the ctrl_path isn't an absolute pathname, assume that
     * it's the name of a socket in the Android reserved namespace.
     * Otherwise, it's a normal UNIX domain socket appearing in the
     * filesystem.
     */
    if (ctrl_path != NULL && *ctrl_path != '/') {
        char buf[21];
        snprintf(buf, sizeof(buf), "wpa_%s", ctrl_path);
        if (socket_local_client_connect(
                ctrl->s, buf,
                ANDROID_SOCKET_NAMESPACE_RESERVED,
                SOCK_DGRAM) < 0) {
            close(ctrl->s);
            unlink(ctrl->local.sun_path);
            free(ctrl);
            return NULL;
        }
        return ctrl;
    }
#endif /* ANDROID */

    ctrl->dest.sun_family = AF_UNIX;
    res = strlcpy(ctrl->dest.sun_path, ctrl_path,
             sizeof(ctrl->dest.sun_path));
    if (res >= sizeof(ctrl->dest.sun_path)) {
        close(ctrl->s);
        free(ctrl);
        return NULL;
    }
    if (connect(ctrl->s, (struct sockaddr *) &ctrl->dest,
            sizeof(ctrl->dest)) < 0) {
        close(ctrl->s);
        unlink(ctrl->local.sun_path);
        free(ctrl);
        return NULL;
    }

    return ctrl;
}


void easybus_ctrl_close(struct easybus_ctrl *ctrl)
{
    if (ctrl == NULL)
        return;
    unlink(ctrl->local.sun_path);
    if (ctrl->s >= 0)
        close(ctrl->s);
    free(ctrl);
}

#endif /* CONFIG_CTRL_IFACE_UNIX */


#ifdef CONFIG_CTRL_IFACE_UDP

struct easybus_ctrl * easybus_ctrl_open(const char *ctrl_path)
{
    struct easybus_ctrl *ctrl;
    char buf[128];
    size_t len;
    
    EB_LOGD("easybus_ctrl_open start!\n");

    ctrl = malloc(sizeof(*ctrl));
    if (ctrl == NULL)
        return NULL;
    memset(ctrl, 0, sizeof(*ctrl));

    ctrl->s = socket(PF_INET, SOCK_DGRAM, 0);
    if (ctrl->s < 0) {
        printf("create socket fail !");
        free(ctrl);
        return NULL;
    }
    EB_LOGD("easybus_ctrl_open ctrl->s = %d!\n", ctrl->s);

    ctrl->local.sin_family = AF_INET;
    ctrl->local.sin_addr.s_addr = htonl((127 << 24) | 1);
    if (bind(ctrl->s, (struct sockaddr *) &ctrl->local,
         sizeof(ctrl->local)) < 0) {
        close(ctrl->s);
        free(ctrl);
        return NULL;
    }
    EB_LOGD("easybus_ctrl_open bind ok!\n");

    ctrl->dest.sin_family = AF_INET;
    ctrl->dest.sin_addr.s_addr = htonl((127 << 24) | 1);
    ctrl->dest.sin_port = htons(EASY_CTRL_IFACE_PORT);
    if (connect(ctrl->s, (struct sockaddr *) &ctrl->dest,
            sizeof(ctrl->dest)) < 0) {
        printf("connect fail !");
        close(ctrl->s);
        free(ctrl);
        return NULL;
    }
    EB_LOGD("easybus_ctrl_open connect ok!\n");

    len = sizeof(buf) - 1;
    if (easybus_ctrl_request(ctrl, "GET_COOKIE", 10, buf, &len, NULL) == 0) {
        buf[len] = '\0';
        ctrl->cookie = strdup(buf);
    }

    return ctrl;
}


void easybus_ctrl_close(struct easybus_ctrl *ctrl)
{
    close(ctrl->s);
    free(ctrl->cookie);
    free(ctrl);
}

#endif /* CONFIG_CTRL_IFACE_UDP */


#ifdef CTRL_IFACE_SOCKET
int easybus_ctrl_request(struct easybus_ctrl *ctrl, const char *cmd, size_t cmd_len,
             char *reply, size_t *reply_len,
             void (*msg_cb)(char *msg, size_t len))
{
    struct timeval tv;
    int res;
    fd_set rfds;
    const char *_cmd;
    char *cmd_buf = NULL;
    size_t _cmd_len;

#ifdef CONFIG_CTRL_IFACE_UDP
    if (ctrl->cookie) {
        char *pos;
        _cmd_len = strlen(ctrl->cookie) + 1 + cmd_len;
        cmd_buf = malloc(_cmd_len);
        if (cmd_buf == NULL)
            return -1;
        memset((void*)cmd_buf, 0, sizeof(cmd_buf));
        _cmd = cmd_buf;
        pos = cmd_buf;
        strlcpy(pos, ctrl->cookie, _cmd_len);
        pos += strlen(ctrl->cookie);
        *pos++ = ' ';
        memcpy(pos, cmd, cmd_len);
    } else
#endif /* CONFIG_CTRL_IFACE_UDP */
    {
        _cmd = cmd;
        _cmd_len = cmd_len;
    }

    if ((res = send(ctrl->s, _cmd, _cmd_len, 0)) < 0) {
        free(cmd_buf);
        return -1;
    }
    free(cmd_buf);

    for (;;) {
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(ctrl->s, &rfds);
        res = select(ctrl->s + 1, &rfds, NULL, NULL, &tv);
        if (res < 0) return res;
        
        if (FD_ISSET(ctrl->s, &rfds)) {
            res = recv(ctrl->s, reply, *reply_len, 0);
            if (res < 0) return res;
            if (res > 0 && reply[0] == '<') {
                /* This is an unsolicited message from
                 * wpa_supplicant, not the reply to the
                 * request. Use msg_cb to report this to the
                 * caller. */
                if (msg_cb) {
                    /* Make sure the message is nul
                     * terminated. */
                    if ((size_t) res == *reply_len)
                        res = (*reply_len) - 1;
                    reply[res] = '\0';
                    msg_cb(reply, res);
                }
                continue;
            }
            *reply_len = res;
            break;
        } else {
            return -2;
        }
    }

    return 0;
}
#endif /* CTRL_IFACE_SOCKET */


static int easybus_ctrl_attach_helper(struct easybus_ctrl *ctrl, int attach)
{
    char buf[10];
    int ret;
    size_t len = 10;

    ret = easybus_ctrl_request(ctrl, attach?"ATTACH":"DETACH", 6,
                   buf, &len, NULL);
    if (ret < 0)
        return ret;
    if (len == 3 && memcmp(buf, "OK\n", 3) == 0)
        return 0;
    return -1;
}


int easybus_ctrl_attach(struct easybus_ctrl *ctrl)
{
    return easybus_ctrl_attach_helper(ctrl, 1);
}


int easybus_ctrl_detach(struct easybus_ctrl *ctrl)
{
    return easybus_ctrl_attach_helper(ctrl, 0);
}


#ifdef CTRL_IFACE_SOCKET

int easybus_ctrl_recv(struct easybus_ctrl *ctrl, char *reply, size_t *reply_len)
{
    int res;

    EB_LOGD("[%s.%d] easybus_ctrl_recv\n", __FUNCTION__, __LINE__);
    res = recv(ctrl->s, reply, *reply_len, 0);    
    if (res < 0)
    {
        EB_LOGD("[%s.%d] easybus_ctrl_recv res=%d\n", __FUNCTION__, __LINE__, res);
    }
    
    *reply_len = res;
    return res;
}


int easybus_ctrl_pending(struct easybus_ctrl *ctrl)
{
    //struct timeval tv;
    fd_set rfds;
    int res;
    
    //tv.tv_sec = 0;
    //tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(ctrl->s, &rfds);
    res = select(ctrl->s + 1, &rfds, NULL, NULL, NULL);
    EB_LOGD("[%s.%d] select res=%d\n", __FUNCTION__, __LINE__, res);
    return FD_ISSET(ctrl->s, &rfds);
}


int easybus_ctrl_get_fd(struct easybus_ctrl *ctrl)
{
    return ctrl->s;
}

#endif /* CTRL_IFACE_SOCKET */

