/*
 * Event loop based on select() loop
 * Copyright (c) 2002-2005, Jouni Malinen <j@w1.fi>
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

#include "common.h"
#include "easy_ctrl_eloop.h"

int eloop_init(struct eloop_data *peloop, void *user_data)
{
    peloop->user_data = user_data;
    return 0;
}


static int eloop_sock_table_add_sock(struct eloop_data *peloop, struct eloop_sock_table *table,
                                     int sock, eloop_sock_handler handler,
                                     void *eloop_data, void *user_data)
{
    struct eloop_sock *tmp;

    if (table == NULL)
        return -1;

    tmp = (struct eloop_sock *)
        realloc(table->table,
               (table->count + 1) * sizeof(struct eloop_sock));
    if (tmp == NULL)
        return -1;

    tmp[table->count].sock = sock;
    tmp[table->count].eloop_data = eloop_data;
    tmp[table->count].user_data = user_data;
    tmp[table->count].handler = handler;
    table->count++;
    table->table = tmp;
    if (sock > peloop->max_sock)
        peloop->max_sock = sock;
    table->changed = 1;

    return 0;
}


static void eloop_sock_table_remove_sock(struct eloop_sock_table *table,
                                         int sock)
{
    int i;

    if (table == NULL || table->table == NULL || table->count == 0)
        return;

    for (i = 0; i < table->count; i++) {
        if (table->table[i].sock == sock)
            break;
    }
    if (i == table->count)
        return;
    if (i != table->count - 1) {
        memmove(&table->table[i], &table->table[i + 1],
               (table->count - i - 1) *
               sizeof(struct eloop_sock));
    }
    table->count--;
    table->changed = 1;
}


static void eloop_sock_table_set_fds(struct eloop_sock_table *table,
                     fd_set *fds)
{
    int i;

    FD_ZERO(fds);

    if (table->table == NULL)
        return;

    for (i = 0; i < table->count; i++)
        FD_SET(table->table[i].sock, fds);
}


static void eloop_sock_table_dispatch(struct eloop_sock_table *table,
                      fd_set *fds)
{
    int i;

    if (table == NULL || table->table == NULL)
        return;

    table->changed = 0;
    for (i = 0; i < table->count; i++) {
        if (FD_ISSET(table->table[i].sock, fds)) {
            table->table[i].handler(table->table[i].sock,
                        table->table[i].eloop_data,
                        table->table[i].user_data);
            if (table->changed)
                break;
        }
    }
}


static void eloop_sock_table_destroy(struct eloop_sock_table *table)
{
    if (table)
        free(table->table);
}


int eloop_register_read_sock(struct eloop_data *peloop, int sock, eloop_sock_handler handler,
                 void *eloop_data, void *user_data)
{
    return eloop_register_sock(peloop, sock, EVENT_TYPE_READ, handler,
                   eloop_data, user_data);
}


void eloop_unregister_read_sock(struct eloop_data *peloop, int sock)
{
    eloop_unregister_sock(peloop, sock, EVENT_TYPE_READ);
}


static struct eloop_sock_table *eloop_get_sock_table(struct eloop_data *peloop, eloop_event_type type)
{
    switch (type) {
    case EVENT_TYPE_READ:
        return &(peloop->readers);
    case EVENT_TYPE_WRITE:
        return &(peloop->writers);
    case EVENT_TYPE_EXCEPTION:
        return &(peloop->exceptions);
    }

    return NULL;
}


int eloop_register_sock(struct eloop_data *peloop, int sock, eloop_event_type type,
            eloop_sock_handler handler,
            void *eloop_data, void *user_data)
{
    struct eloop_sock_table *table;

    table = eloop_get_sock_table(peloop, type);
    return eloop_sock_table_add_sock(peloop, table, sock, handler,
                     eloop_data, user_data);
}


void eloop_unregister_sock(struct eloop_data *peloop, int sock, eloop_event_type type)
{
    struct eloop_sock_table *table;

    table = eloop_get_sock_table(peloop, type);
    eloop_sock_table_remove_sock(table, sock);
}


int eloop_register_timeout(struct eloop_data *peloop, unsigned int secs, unsigned int usecs,
               eloop_timeout_handler handler,
               void *eloop_data, void *user_data)
{
    struct eloop_timeout *timeout, *tmp, *prev;

    timeout = malloc(sizeof(*timeout));
    if (timeout == NULL)
        return -1;
    if (get_time(&timeout->time) < 0) {
        free(timeout);
        return -1;
    }
    timeout->time.sec += secs;
    timeout->time.usec += usecs;
    while (timeout->time.usec >= 1000000) {
        timeout->time.sec++;
        timeout->time.usec -= 1000000;
    }
    timeout->eloop_data = eloop_data;
    timeout->user_data = user_data;
    timeout->handler = handler;
    timeout->next = NULL;

    if (peloop->timeout == NULL) {
        peloop->timeout = timeout;
        return 0;
    }

    prev = NULL;
    tmp = peloop->timeout;
    while (tmp != NULL) {
        if (easy_time_before(&timeout->time, &tmp->time))
            break;
        prev = tmp;
        tmp = tmp->next;
    }

    if (prev == NULL) {
        timeout->next = peloop->timeout;
        peloop->timeout = timeout;
    } else {
        timeout->next = prev->next;
        prev->next = timeout;
    }

    return 0;
}


int eloop_cancel_timeout(struct eloop_data *peloop, eloop_timeout_handler handler,
             void *eloop_data, void *user_data)
{
    struct eloop_timeout *timeout, *prev, *next;
    int removed = 0;

    prev = NULL;
    timeout = peloop->timeout;
    while (timeout != NULL) {
        next = timeout->next;

        if (timeout->handler == handler &&
            (timeout->eloop_data == eloop_data ||
             eloop_data == ELOOP_ALL_CTX) &&
            (timeout->user_data == user_data ||
             user_data == ELOOP_ALL_CTX)) {
            if (prev == NULL)
                peloop->timeout = next;
            else
                prev->next = next;
            free(timeout);
            removed++;
        } else
            prev = timeout;

        timeout = next;
    }

    return removed;
}


int eloop_is_timeout_registered(struct eloop_data *peloop, eloop_timeout_handler handler,
                void *eloop_data, void *user_data)
{
    struct eloop_timeout *tmp;

    tmp = peloop->timeout;
    while (tmp != NULL) {
        if (tmp->handler == handler &&
            tmp->eloop_data == eloop_data &&
            tmp->user_data == user_data)
            return 1;

        tmp = tmp->next;
    }

    return 0;
}


#ifndef CONFIG_NATIVE_WINDOWS
static void eloop_handle_alarm(int sig)
{
    fprintf(stderr, "eloop: could not process SIGINT or SIGTERM in two "
        "seconds. Looks like there\n"
        "is a bug that ends up in a busy loop that "
        "prevents clean shutdown.\n"
        "Killing program forcefully.\n");
    exit(1);
}
#endif /* CONFIG_NATIVE_WINDOWS */


static void eloop_handle_signal(struct eloop_data *peloop, int sig)
{
    int i;

#ifndef CONFIG_NATIVE_WINDOWS
    if ((sig == SIGINT || sig == SIGTERM) && !peloop->pending_terminate) {
        /* Use SIGALRM to break out from potential busy loops that
         * would not allow the program to be killed. */
        peloop->pending_terminate = 1;
        signal(SIGALRM, eloop_handle_alarm);
        alarm(2);
    }
#endif /* CONFIG_NATIVE_WINDOWS */

    peloop->signaled++;
    for (i = 0; i < peloop->signal_count; i++) {
        if (peloop->signals[i].sig == sig) {
            peloop->signals[i].signaled++;
            break;
        }
    }
}


static void eloop_process_pending_signals(struct eloop_data *peloop)
{
    int i;

    if (peloop->signaled == 0)
        return;
    peloop->signaled = 0;

    if (peloop->pending_terminate) {
#ifndef CONFIG_NATIVE_WINDOWS
        alarm(0);
#endif /* CONFIG_NATIVE_WINDOWS */
        peloop->pending_terminate = 0;
    }

    for (i = 0; i < peloop->signal_count; i++) {
        if (peloop->signals[i].signaled) {
            peloop->signals[i].signaled = 0;
            peloop->signals[i].handler(peloop->signals[i].sig,
                         peloop->user_data,
                         peloop->signals[i].user_data);
        }
    }
}


int eloop_register_signal(struct eloop_data *peloop, int sig, eloop_signal_handler handler,
              void *user_data)
{
    struct eloop_signal *tmp;

    tmp = (struct eloop_signal *)
        realloc(peloop->signals,
               (peloop->signal_count + 1) *
               sizeof(struct eloop_signal));
    if (tmp == NULL)
        return -1;

    tmp[peloop->signal_count].sig = sig;
    tmp[peloop->signal_count].user_data = user_data;
    tmp[peloop->signal_count].handler = handler;
    tmp[peloop->signal_count].signaled = 0;
    peloop->signal_count++;
    peloop->signals = tmp;
    signal(sig, eloop_handle_signal);

    return 0;
}


int eloop_register_signal_terminate(struct eloop_data *peloop, eloop_signal_handler handler,
                    void *user_data)
{
    int ret = eloop_register_signal(peloop, SIGINT, handler, user_data);
    if (ret == 0)
        ret = eloop_register_signal(peloop, SIGTERM, handler, user_data);
    if (ret == 0)
        ret = eloop_register_signal(peloop, SIGSEGV, handler, user_data);
    return ret;
}


int eloop_register_signal_reconfig(struct eloop_data *peloop, eloop_signal_handler handler,
                   void *user_data)
{
#ifdef CONFIG_NATIVE_WINDOWS
    return 0;
#else /* CONFIG_NATIVE_WINDOWS */
    return eloop_register_signal(peloop, SIGHUP, handler, user_data);
#endif /* CONFIG_NATIVE_WINDOWS */
}


void eloop_run(struct eloop_data *peloop)
{
    fd_set *rfds, *wfds, *efds;
    int res;
    struct timeval _tv;
    struct easy_time tv, now;

    rfds = malloc(sizeof(*rfds));
    wfds = malloc(sizeof(*wfds));
    efds = malloc(sizeof(*efds));
    if (rfds == NULL || wfds == NULL || efds == NULL) {
        printf("eloop_run - malloc failed\n");
        goto out;
    }

    while (!peloop->terminate &&
           (peloop->timeout || peloop->readers.count > 0 ||
        peloop->writers.count > 0 || peloop->exceptions.count > 0)) {
        if (peloop->timeout) {
            get_time(&now);
            if (easy_time_before(&now, &peloop->timeout->time))
                easy_time_sub(&peloop->timeout->time, &now, &tv);
            else
                tv.sec = tv.usec = 0;
#if 0
            printf("next timeout in %lu.%06lu sec\n",
                   tv.sec, tv.usec);
#endif
            _tv.tv_sec = tv.sec;
            _tv.tv_usec = tv.usec;
        }

        eloop_sock_table_set_fds(&peloop->readers, rfds);
        eloop_sock_table_set_fds(&peloop->writers, wfds);
        eloop_sock_table_set_fds(&peloop->exceptions, efds);
        res = select(peloop->max_sock + 1, rfds, wfds, efds,
                 peloop->timeout ? &_tv : NULL);
        if (res < 0 && errno != EINTR && errno != 0) {
            perror("select");
            goto out;
        }
        eloop_process_pending_signals(peloop);

        /* check if some registered timeouts have occurred */
        if (peloop->timeout) {
            struct eloop_timeout *tmp;

            get_time(&now);
            if (!easy_time_before(&now, &peloop->timeout->time)) {
                tmp = peloop->timeout;
                peloop->timeout = peloop->timeout->next;
                tmp->handler(tmp->eloop_data,
                         tmp->user_data);
                free(tmp);
            }

        }

        if (res <= 0)
            continue;

        eloop_sock_table_dispatch(&peloop->readers, rfds);
        eloop_sock_table_dispatch(&peloop->writers, wfds);
        eloop_sock_table_dispatch(&peloop->exceptions, efds);
    }

out:
    free(rfds);
    free(wfds);
    free(efds);
}


void eloop_terminate(struct eloop_data *peloop)
{
    peloop->terminate = 1;
}


void eloop_destroy(struct eloop_data *peloop)
{
    struct eloop_timeout *timeout, *prev;

    timeout = peloop->timeout;
    while (timeout != NULL) {
        prev = timeout;
        timeout = timeout->next;
        free(prev);
    }
    eloop_sock_table_destroy(&peloop->readers);
    eloop_sock_table_destroy(&peloop->writers);
    eloop_sock_table_destroy(&peloop->exceptions);
    free(peloop->signals);
}


int eloop_terminated(struct eloop_data *peloop)
{
    return peloop->terminate;
}


void eloop_wait_for_read_sock(int sock)
{
    fd_set rfds;

    if (sock < 0)
        return;

    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
    select(sock + 1, &rfds, NULL, NULL, NULL);
}


void * eloop_get_user_data(struct eloop_data *peloop)
{
    return peloop->user_data;
}
