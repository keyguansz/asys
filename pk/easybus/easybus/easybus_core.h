/*
 * wpa_supplicant/hostapd control interface library
 * Copyright (c) 2004-2006, Jouni Malinen <j@w1.fi>
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

#ifndef EASYBUS_CORE_H
#define EASYBUS_CORE_H

#ifdef  __cplusplus
extern "C" {
#endif


/* wpa_supplicant/hostapd control interface access */

/**
 * wpa_ctrl_open - Open a control interface to wpa_supplicant/hostapd
 * @ctrl_path: Path for UNIX domain sockets; ignored if UDP sockets are used.
 * Returns: Pointer to abstract control interface data or %NULL on failure
 *
 * This function is used to open a control interface to wpa_supplicant/hostapd.
 * ctrl_path is usually /var/run/wpa_supplicant or /var/run/hostapd. This path
 * is configured in wpa_supplicant/hostapd and other programs using the control
 * interface need to use matching path configuration.
 */
struct easybus_ctrl * easybus_ctrl_open(const char *ctrl_path);


/**
 * easybus_ctrl_close - Close a control interface to wpa_supplicant/hostapd
 * @ctrl: Control interface data from easybus_ctrl_open()
 *
 * This function is used to close a control interface.
 */
void easybus_ctrl_close(struct easybus_ctrl *ctrl);


/**
 * easybus_ctrl_request - Send a command to wpa_supplicant/hostapd
 * @ctrl: Control interface data from easybus_ctrl_open()
 * @cmd: Command; usually, ASCII text, e.g., "PING"
 * @cmd_len: Length of the cmd in bytes
 * @reply: Buffer for the response
 * @reply_len: Reply buffer length
 * @msg_cb: Callback function for unsolicited messages or %NULL if not used
 * Returns: 0 on success, -1 on error (send or receive failed), -2 on timeout
 *
 * This function is used to send commands to wpa_supplicant/hostapd. Received
 * response will be written to reply and reply_len is set to the actual length
 * of the reply. This function will block for up to two seconds while waiting
 * for the reply. If unsolicited messages are received, the blocking time may
 * be longer.
 *
 * msg_cb can be used to register a callback function that will be called for
 * unsolicited messages received while waiting for the command response. These
 * messages may be received if easybus_ctrl_request() is called at the same time as
 * wpa_supplicant/hostapd is sending such a message. This can happen only if
 * the program has used easybus_ctrl_attach() to register itself as a monitor for
 * event messages. Alternatively to msg_cb, programs can register two control
 * interface connections and use one of them for commands and the other one for
 * receiving event messages, in other words, call easybus_ctrl_attach() only for
 * the control interface connection that will be used for event messages.
 */
int easybus_ctrl_request(struct easybus_ctrl *ctrl, const char *cmd, size_t cmd_len,
             char *reply, size_t *reply_len,
             void (*msg_cb)(char *msg, size_t len));


/**
 * easybus_ctrl_attach - Register as an event monitor for the control interface
 * @ctrl: Control interface data from easybus_ctrl_open()
 * Returns: 0 on success, -1 on failure, -2 on timeout
 *
 * This function registers the control interface connection as a monitor for
 * wpa_supplicant/hostapd events. After a success easybus_ctrl_attach() call, the
 * control interface connection starts receiving event messages that can be
 * read with easybus_ctrl_recv().
 */
int easybus_ctrl_attach(struct easybus_ctrl *ctrl);


/**
 * easybus_ctrl_detach - Unregister event monitor from the control interface
 * @ctrl: Control interface data from easybus_ctrl_open()
 * Returns: 0 on success, -1 on failure, -2 on timeout
 *
 * This function unregisters the control interface connection as a monitor for
 * wpa_supplicant/hostapd events, i.e., cancels the registration done with
 * easybus_ctrl_attach().
 */
int easybus_ctrl_detach(struct easybus_ctrl *ctrl);


/**
 * easybus_ctrl_recv - Receive a pending control interface message
 * @ctrl: Control interface data from easybus_ctrl_open()
 * @reply: Buffer for the message data
 * @reply_len: Length of the reply buffer
 * Returns: 0 on success, -1 on failure
 *
 * This function will receive a pending control interface message. This
 * function will block if no messages are available. The received response will
 * be written to reply and reply_len is set to the actual length of the reply.
 * easybus_ctrl_recv() is only used for event messages, i.e., easybus_ctrl_attach()
 * must have been used to register the control interface as an event monitor.
 */
int easybus_ctrl_recv(struct easybus_ctrl *ctrl, char *reply, size_t *reply_len);


/**
 * easybus_ctrl_pending - Check whether there are pending event messages
 * @ctrl: Control interface data from easybus_ctrl_open()
 * Returns: 1 if there are pending messages, 0 if no, or -1 on error
 *
 * This function will check whether there are any pending control interface
 * message available to be received with easybus_ctrl_recv(). easybus_ctrl_pending() is
 * only used for event messages, i.e., easybus_ctrl_attach() must have been used to
 * register the control interface as an event monitor.
 */
int easybus_ctrl_pending(struct easybus_ctrl *ctrl);


/**
 * easybus_ctrl_get_fd - Get file descriptor used by the control interface
 * @ctrl: Control interface data from easybus_ctrl_open()
 * Returns: File descriptor used for the connection
 *
 * This function can be used to get the file descriptor that is used for the
 * control interface connection. The returned value can be used, e.g., with
 * select() while waiting for multiple events.
 *
 * The returned file descriptor must not be used directly for sending or
 * receiving packets; instead, the library functions easybus_ctrl_request() and
 * easybus_ctrl_recv() must be used for this.
 */
int easybus_ctrl_get_fd(struct easybus_ctrl *ctrl);

#define CONFIG_CTRL_IFACE_UDP
#define CTRL_IFACE_SOCKET

#ifdef CONFIG_CTRL_IFACE_UDP
#define EASY_CTRL_IFACE_PORT 12171
#endif /* CONFIG_CTRL_IFACE_UDP */


#ifdef  __cplusplus
}
#endif

#endif /* EASYBUS_CORE_H */
