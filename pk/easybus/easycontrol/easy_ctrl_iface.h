/*
 * WPA Supplicant / UNIX domain socket -based control interface
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

#ifndef CTRL_IFACE_H
#define CTRL_IFACE_H

/* Per-interface ctrl_iface */

/**
 * struct easy_ctrl_dst - Internal data structure of control interface monitors
 *
 * This structure is used to store information about registered control
 * interface monitors into struct wpa_supplicant. This data is private to
 * ctrl_iface_udp.c and should not be touched directly from other files.
 */
#define COOKIE_LEN 8

#define EASY_BUS_MSGTYPE_MAX_NUM 20

struct easy_ctrl_dst {
    struct easy_ctrl_dst *next;
    struct sockaddr_in addr;
    socklen_t addrlen;
    char msgtype[EASY_BUS_MSGTYPE_MAX_NUM][EASYBUS_MSGTYPE_MAX_LEN + 1];
    int msgtypeSize;
    int errors;
};


struct ctrl_iface_priv {
    struct easy_ctrl_svc *easy_ctrl_s;
    int sock;
    struct easy_ctrl_dst *ctrl_dst;
    unsigned char cookie[COOKIE_LEN];
};

struct easy_ctrl_svc {
    char ifname[128];

    struct ctrl_iface_priv *ctrl_iface;
};

enum { MSG_MSGDUMP, MSG_DEBUG, MSG_INFO, MSG_WARNING, MSG_ERROR };
/* Shared functions from ctrl_iface.c; to be called by ctrl_iface backends */

/**
 * easy_ctrl_iface_process - Process ctrl_iface command
 * @wpa_s: Pointer to wpa_supplicant data
 * @buf: Received command buffer (nul terminated string)
 * @resp_len: Variable to be set to the response length
 * Returns: Response (*resp_len bytes) or %NULL on failure
 *
 * Control interface backends call this function when receiving a message that
 * they do not process internally, i.e., anything else than ATTACH, DETACH,
 * and LEVEL. The return response value is then sent to the external program
 * that sent the command. Caller is responsible for freeing the buffer after
 * this. If %NULL is returned, *resp_len can be set to two special values:
 * 1 = send "FAIL\n" response, 2 = send "OK\n" response. If *resp_len has any
 * other value, no response is sent.
 */
//char * easy_ctrl_iface_process(struct wpa_supplicant *wpa_s,
                     //char *buf, size_t *resp_len);

/**
 * easy_ctrl_iface_process - Process global ctrl_iface command
 * @global: Pointer to global data from easy_init()
 * @buf: Received command buffer (nul terminated string)
 * @resp_len: Variable to be set to the response length
 * Returns: Response (*resp_len bytes) or %NULL on failure
 *
 * Control interface backends call this function when receiving a message from
 * the global ctrl_iface connection. The return response value is then sent to
 * the external program that sent the command. Caller is responsible for
 * freeing the buffer after this. If %NULL is returned, *resp_len can be set to
 * two special values: 1 = send "FAIL\n" response, 2 = send "OK\n" response. If
 * *resp_len has any other value, no response is sent.
 */
//char * easy_global_ctrl_iface_process(struct wpa_global *global,
                        //char *buf, size_t *resp_len);


/* Functions that each ctrl_iface backend must implement */

/**
 * easy_ctrl_iface_init - Initialize control interface
 * @wpa_s: Pointer to wpa_supplicant data
 * Returns: Pointer to private data on success, %NULL on failure
 *
 * Initialize the control interface and start receiving commands from external
 * programs.
 *
 * Required to be implemented in each control interface backend.
 */
struct ctrl_iface_priv *
easy_ctrl_iface_init();

/**
 * easy_ctrl_iface_deinit - Deinitialize control interface
 * @priv: Pointer to private data from easy_ctrl_iface_init()
 *
 * Deinitialize the control interface that was initialized with
 * easy_ctrl_iface_init().
 *
 * Required to be implemented in each control interface backend.
 */
void easy_ctrl_iface_deinit(struct eloop_data *peloop, struct ctrl_iface_priv *priv);

/**
 * easy_ctrl_iface_wait - Wait for ctrl_iface monitor
 * @priv: Pointer to private data from easy_ctrl_iface_init()
 *
 * Wait until the first message from an external program using the control
 * interface is received. This function can be used to delay normal startup
 * processing to allow control interface programs to attach with
 * %wpa_supplicant before normal operations are started.
 *
 * Required to be implemented in each control interface backend.
 */
void easy_ctrl_iface_wait(struct ctrl_iface_priv *priv);

/**
 * easy_global_ctrl_iface_init - Initialize global control interface
 * @global: Pointer to global data from easy_init()
 * Returns: Pointer to private data on success, %NULL on failure
 *
 * Initialize the global control interface and start receiving commands from
 * external programs.
 *
 * Required to be implemented in each control interface backend.
 */
//struct ctrl_iface_global_priv *
//easy_global_ctrl_iface_init(struct wpa_global *global);

/**
 * easy_global_ctrl_iface_deinit - Deinitialize global ctrl interface
 * @priv: Pointer to private data from easy_global_ctrl_iface_init()
 *
 * Deinitialize the global control interface that was initialized with
 * easy_global_ctrl_iface_init().
 *
 * Required to be implemented in each control interface backend.
 */
//void easy_global_ctrl_iface_deinit(
//    struct ctrl_iface_global_priv *priv);

#else /* CONFIG_CTRL_IFACE */

static inline struct ctrl_iface_priv *
easy_ctrl_iface_init()
{
    return (void *) -1;
}

static inline void
easy_ctrl_iface_deinit(struct ctrl_iface_priv *priv)
{
}

static inline void
easy_ctrl_iface_send(struct ctrl_iface_priv *priv, char *msgtype,
                   char *buf, size_t len)
{
}

static inline void
easy_ctrl_iface_wait(struct ctrl_iface_priv *priv)
{
}

//static inline struct ctrl_iface_global_priv *
//easy_global_ctrl_iface_init(struct wpa_global *global)
//{
//    return (void *) 1;
//}

//static inline void
//easy_global_ctrl_iface_deinit(struct ctrl_iface_global_priv *priv)
//{
//}

#endif /* CTRL_IFACE_H */
