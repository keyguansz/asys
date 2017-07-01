/*
 * WPA Supplicant / UDP socket -based control interface
 * Copyright (c) 2004-2005, Jouni Malinen <j@w1.fi>
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

//#include "includes.h"

#include "common.h"
#include "easy_ctrl_eloop.h"
//#include "config.h"
//#include "eapol_supp/eapol_supp_sm.h"
//#include "easy_ctrl_i.h"

#include "easy_common.h"

#include "easy_ctrl_iface.h"

#define perror printf
#define EASY_CTRL_IFACE_PORT 12171


static void easy_ctrl_iface_send(struct ctrl_iface_priv *priv,
                       char *msgtype, const char *buf,
                       size_t len);


static int easy_ctrl_iface_attach(struct ctrl_iface_priv *priv,
                        struct sockaddr_in *from,
                        socklen_t fromlen)
{
    struct easy_ctrl_dst *dst;

    dst = malloc(sizeof(*dst));
    if (dst == NULL)
        return -1;
    memset(dst, 0, sizeof(*dst));
    memcpy(&dst->addr, from, sizeof(struct sockaddr_in));
    dst->addrlen = fromlen;
    dst->next = priv->ctrl_dst;
    priv->ctrl_dst = dst;
    EB_LOGD("CTRL_IFACE monitor attached %s:%d\n",
           inet_ntoa(from->sin_addr), ntohs(from->sin_port));
    return 0;
}


static int easy_ctrl_iface_detach(struct ctrl_iface_priv *priv,
                        struct sockaddr_in *from,
                        socklen_t fromlen)
{
    struct easy_ctrl_dst *dst, *prev = NULL;

    dst = priv->ctrl_dst;
    while (dst) {
        if (from->sin_addr.s_addr == dst->addr.sin_addr.s_addr &&
            from->sin_port == dst->addr.sin_port) {
            if (prev == NULL)
                priv->ctrl_dst = dst->next;
            else
                prev->next = dst->next;
            free(dst);
            EB_LOGD("CTRL_IFACE monitor detached "
                   "%s:%d\n", inet_ntoa(from->sin_addr),
                   ntohs(from->sin_port));
            return 0;
        }
        prev = dst;
        dst = dst->next;
    }
    return -1;
}


static int easy_ctrl_iface_interest(struct ctrl_iface_priv *priv,
                       struct sockaddr_in *from,
                       socklen_t fromlen,
                       char *msgtype)
{
    struct easy_ctrl_dst *dst;
    int index;
    int ret = -1;
    
    EB_LOGD("[%s.%d] interest msgtype: %s from: %s:%d\n", __FUNCTION__, __LINE__, 
        msgtype, inet_ntoa(from->sin_addr), ntohs(from->sin_port));
    
    dst = priv->ctrl_dst;
    while (dst) {
        EB_LOGD("[%s.%d] dst: %s:%d, msgtypeSize: %d\n", __FUNCTION__, __LINE__, 
            inet_ntoa(dst->addr.sin_addr), ntohs(dst->addr.sin_port),dst->msgtypeSize);
            
        if (from->sin_addr.s_addr == dst->addr.sin_addr.s_addr 
            && from->sin_port == dst->addr.sin_port
            && dst->msgtypeSize < EASY_BUS_MSGTYPE_MAX_NUM) {
            
            for (index = 0; index < dst->msgtypeSize; index++) {
                if (strcmp(dst->msgtype[index], msgtype) == 0) 
                {
                    EB_LOGD("[%s.%d] already interest !\n", __FUNCTION__, __LINE__);
                    break;
                }
            }
            if (index == dst->msgtypeSize) {
                strcpy(dst->msgtype[dst->msgtypeSize++], msgtype);
                EB_LOGD("[%s.%d] interest success !\n", __FUNCTION__, __LINE__);
            }
            
            ret = 0;
            break;
        }
        dst = dst->next;
    }
    
    EB_LOGD("[%s.%d] ret: %d\n", __FUNCTION__, __LINE__, ret);
    return ret;
}


static char * easy_ctrl_iface_get_cookie(struct ctrl_iface_priv *priv,
                     size_t *reply_len)
{
    char *reply;
    reply = (char *)malloc(7 + 2 * COOKIE_LEN + 1);
    if (reply == NULL) {
        *reply_len = 1;
        return NULL;
    }

    memcpy(reply, "COOKIE=", 7);
    easy_snprintf_hex(reply + 7, 2 * COOKIE_LEN + 1,
             priv->cookie, COOKIE_LEN);

    *reply_len = 7 + 2 * COOKIE_LEN;
    return reply;
}

static int easy_ctrl_iface_process(char *buf, size_t bufLen)
{
    EasybusMsg msg;
    char *str1 = NULL;
    char *str2 = NULL;
    int ret = -1;

    memset((void*)&msg, 0, sizeof(msg));

    EB_LOGD("[%s.%d] buf: %s\n", __FUNCTION__, __LINE__, buf);

    do
    {
        if (strncmp(buf, "SEND ", 5) == 0) {
            //处理来自STB   APP   的数据
            str1 = buf + strlen("SEND addr.ip=");
            if (str1 - buf >= bufLen) break;
            str2 = strstr(str1, ",addr.port=");
            if (str2 == NULL) break;
            memcpy((void*)msg.remoteAddr.ip, str1, str2 - str1);
            EB_LOGD("ip = %s\n", msg.remoteAddr.ip);
            str1 = str2 + strlen(",addr.port=");
            if (str1 - buf >= bufLen) break;
            str2 = strstr(str1, ",msgType=");
            if (str2 == NULL) break;
            EB_LOGD("str2 - str1 = %d\n", str2 - str1);
            {
                char port_str[12];

                memset(port_str, 0, sizeof(port_str));
                memcpy((void*)port_str, str1, str2 - str1);
                EB_LOGD("port_str = %s\n", port_str);
                msg.remoteAddr.port = atoi(port_str);
            }
            EB_LOGD("port = %d\n", msg.remoteAddr.port);
            str1 = str2 + strlen(",msgType=");
            if (str1 - buf >= bufLen) break;
            str2 = strstr(str1, ",msgData=");
            if (str2 == NULL) break;
            EB_LOGD("str2 - str1 = %d\n", str2 - str1);
            memcpy((void*)msg.msgType, str1, str2 - str1);
            EB_LOGD("msgType = %s\n", msg.msgType);
            str1 = str2 + strlen(",msgData=");
            if (str1 - buf >= bufLen) break;
            strcpy(msg.msgData, str1);
            msg.msgDataSize = strlen(msg.msgData);
            EB_LOGD("msgData = %s\n", msg.msgData);
        
            EB_LOGD("easy_ctrl_iface_process addr.ip=%s,addr.port=%d,msgType=%s,msgData=%s\n", 
                msg.remoteAddr.ip, msg.remoteAddr.port, msg.msgType, msg.msgData);
            easy_ctrl_compose_frame(&msg);

            ret = 0;
        }
    }while(0);
    
    EB_LOGD("[%s.%d] ret: %d\n", __FUNCTION__, __LINE__, ret);
    return ret;
}

static void easy_ctrl_iface_receive(int sock, void *eloop_ctx,
                          void *sock_ctx)
{
    struct easy_ctrl_svc *easy_s = eloop_ctx;
    struct ctrl_iface_priv *priv = sock_ctx;
    char *buf, *pos;
    int res;
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    char *reply = NULL;
    size_t reply_len = 0;
    int new_attached = 0;
    unsigned char cookie[COOKIE_LEN];
    
    EB_LOGD("[%s.%d] start\n", __FUNCTION__, __LINE__);

    if ((buf = malloc(EASY_BUS_BUFF_MAX_LEN)) == NULL) {
        return;
    }
    memset(buf, 0, sizeof(EASY_BUS_BUFF_MAX_LEN));
    
    res = recvfrom(sock, buf, EASY_BUS_BUFF_MAX_LEN - 1, 0,
               (struct sockaddr *) &from, &fromlen);
    if (res < 0) {
        perror("recvfrom(ctrl_iface)");
        goto done;
    }

    buf[res] = '\0';
    
    EB_LOGD("[%s.%d] from addr: %s:%d\n", __FUNCTION__, __LINE__, 
        inet_ntoa(from.sin_addr), ntohs(from.sin_port));
    EB_LOGD("[%s.%d] receive res:%d, buf:%s\n", __FUNCTION__, __LINE__, res, buf);

    if (from.sin_addr.s_addr != htonl((127 << 24) | 1)) {
        /*
         * The OS networking stack is expected to drop this kind of
         * frames since the socket is bound to only localhost address.
         * Just in case, drop the frame if it is coming from any other
         * address.
         */
        printf("CTRL: Drop packet from unexpected "
               "source %s", inet_ntoa(from.sin_addr));
        goto done;
    }
    
    if (strcmp(buf, "GET_COOKIE") == 0) {
        reply = easy_ctrl_iface_get_cookie(priv, &reply_len);
        goto done;
    }

    /*
     * Require that the client includes a prefix with the 'cookie' value
     * fetched with GET_COOKIE command. This is used to verify that the
     * client has access to a bidirectional link over UDP in order to
     * avoid attacks using forged localhost IP address even if the OS does
     * not block such frames from remote destinations.
     */
    if (strncmp(buf, "COOKIE=", 7) != 0) {
        printf("CTLR: No cookie in the request - "
               "drop request");
        goto done;
    }

    if (hexstr2bin(buf + 7, cookie, COOKIE_LEN) < 0) {
        printf("CTLR: Invalid cookie format in the "
               "request - drop request");
        goto done;
    }

    if (memcmp(cookie, priv->cookie, COOKIE_LEN) != 0) {
        printf( "CTLR: Invalid cookie in the request - "
               "drop request");
        goto done;
    }

    pos = buf + 7 + 2 * COOKIE_LEN;
    while (*pos == ' ')
        pos++;

    if (strncmp(pos, "ATTACH", strlen("ATTACH")) == 0) {
        if (easy_ctrl_iface_attach(priv, &from, fromlen)){
            reply_len = 1; 
        }
        else {
            new_attached = 1;
            reply_len = 2;
        }
    } else if (strcmp(pos, "DETACH") == 0) {
        if (easy_ctrl_iface_detach(priv, &from, fromlen))
            reply_len = 1;
        else
            reply_len = 2;
    } else if (strncmp(pos, "INTEREST ", strlen("INTEREST ")) == 0) {
        char msgType[20];

        memset(msgType, 0, sizeof(msgType));
        sscanf(pos, "INTEREST msgType=%s", msgType);
        EB_LOGD("msgType = %s\n", msgType);
        if (easy_ctrl_iface_interest(priv, &from, fromlen, msgType))
            reply_len = 1;
        else
            reply_len = 2;
    } else {
        if (easy_ctrl_iface_process(pos, strlen(pos)))
            reply_len = 1;
        else
            reply_len = 2;
    }

 done:
     EB_LOGD("[%s.%d] reply=0x%x, reply_len=%d\n", __FUNCTION__, __LINE__, reply, reply_len);
    
    if (buf != NULL) {
        free(buf);
    }
    
    if (reply) {
        sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from,
               fromlen);
        free(reply);
    } else if (reply_len == 1) {
        sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *) &from,
               fromlen);
    } else if (reply_len == 2) {
        sendto(sock, "OK\n", 3, 0, (struct sockaddr *) &from,
               fromlen);
    }

    EB_LOGD("[%s.%d] end\n", __FUNCTION__, __LINE__);
}


static void easy_ctrl_iface_msg_cb(void *ctx, char *msgtype,
                         const char *txt, size_t len)
{
    struct easy_ctrl_svc *easy_s = ctx;

    if (easy_s == NULL || easy_s->ctrl_iface == NULL)
        return;
    easy_ctrl_iface_send(easy_s->ctrl_iface, msgtype, txt, len);
}

static int get_random(unsigned char *buf, size_t len)
{
    FILE *f;
    size_t rc;

    f = fopen("/dev/urandom", "rb");
    if (f == NULL) {
        printf("Could not open /dev/urandom.\n");
        return -1;
    }

    rc = fread(buf, 1, len, f);
    fclose(f);

    return rc != len ? -1 : 0;
}

struct ctrl_iface_priv * easy_ctrl_iface_init(struct eloop_data *peloop, struct easy_ctrl_svc *easy_s)
{
    struct ctrl_iface_priv *priv;
    struct sockaddr_in addr;

    priv = malloc(sizeof(*priv));
    if (priv == NULL)
        return NULL;
    memset(priv, 0, sizeof(*priv));
    priv->easy_ctrl_s = easy_s;
    priv->sock = -1;
    get_random(priv->cookie, COOKIE_LEN);

    priv->sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (priv->sock < 0) {
        perror("socket(PF_INET)");
        goto fail;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl((127 << 24) | 1);
    addr.sin_port = htons(EASY_CTRL_IFACE_PORT);
    if (bind(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("bind(AF_INET)");
        goto fail;
    }

    eloop_register_read_sock(peloop, priv->sock, easy_ctrl_iface_receive,
                 easy_s, priv);
    easy_msg_register_cb(easy_ctrl_iface_msg_cb);

    return priv;

fail:
    if (priv->sock >= 0)
        close(priv->sock);
    free(priv);
    return NULL;
}


void easy_ctrl_iface_deinit(struct eloop_data *peloop, struct ctrl_iface_priv *priv)
{
    struct easy_ctrl_dst *dst, *prev;

    if (priv->sock > -1) {
        eloop_unregister_read_sock(peloop, priv->sock);
        if (priv->ctrl_dst) {
            /*
             * Wait a second before closing the control socket if
             * there are any attached monitors in order to allow
             * them to receive any pending messages.
             */
            EB_LOGD( "CTRL_IFACE wait for attached "
                   "monitors to receive messages");
            sleep(1);
        }
        close(priv->sock);
        priv->sock = -1;
    }

    dst = priv->ctrl_dst;
    while (dst) {
        prev = dst;
        dst = dst->next;
        free(prev);
    }
    free(priv);
}


static void easy_ctrl_iface_send(struct ctrl_iface_priv *priv,
                       char *msgtype, const char *buf,
                       size_t len)
{
    struct easy_ctrl_dst *dst, *next;

    dst = priv->ctrl_dst;
    
    EB_LOGD("[%s.%d] priv->sock = %d\n", __FUNCTION__, __LINE__, priv->sock);
    
    if (priv->sock < 0 || dst == NULL) return;

    EB_LOGD("[%s.%d] msgtype=%s bufLen=%d, buf=%s\n", __FUNCTION__, __LINE__, msgtype, len, buf);

    while (dst) {
        int index;
        int res;
        
        next = dst->next;

        EB_LOGD("[%s.%d] dst->addr: %s:%d\n", __FUNCTION__, __LINE__, 
            inet_ntoa(dst->addr.sin_addr), ntohs(dst->addr.sin_port));

        for (index = 0; index < dst->msgtypeSize; index++) {
            EB_LOGD("[%s.%d] dst->msgtype[%d]: %s\n", __FUNCTION__, __LINE__, index, dst->msgtype[index]);
               if (strcmp(msgtype, dst->msgtype[index]) == 0) {
                
                EB_LOGD("[%s.%d] sendto: %s:%d\n", __FUNCTION__, __LINE__, 
                    inet_ntoa(dst->addr.sin_addr), ntohs(dst->addr.sin_port));

                   if ((res = sendto(priv->sock, buf, len, 0,
                          (struct sockaddr *) &dst->addr, sizeof(dst->addr))) < 0) {
                       perror("sendto(CTRL_IFACE monitor)");
                    printf("[%s.%d] sendto failed !\n", __FUNCTION__, __LINE__);
                       dst->errors++;
                       if (dst->errors > 10) {
                           easy_ctrl_iface_detach(priv, &dst->addr, dst->addrlen);
                       }
                   } else {
                       dst->errors = 0;
                    EB_LOGD("[%s.%d] sendto success !, res=%d\n", __FUNCTION__, __LINE__, res);
                   }
               }
        }
        dst = next;
    }
}


void easy_ctrl_iface_wait(struct ctrl_iface_priv *priv)
{
    EB_LOGD( "CTRL_IFACE - %s - wait for monitor",
           priv->easy_ctrl_s->ifname);
    eloop_wait_for_read_sock(priv->sock);
}

