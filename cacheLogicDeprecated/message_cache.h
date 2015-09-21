//
// Created by Dusan Klinec on 16.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_MESSAGE_CACHE_H
#define OPENSIPS_1_11_2_TLS_MESSAGE_CACHE_H

#include "common.h"
#include "../../locking.h"

/**
 * Structure for one per user msilo management.
 * "receiver:sender" -> _msilo_ctl_el
 */
typedef struct _msilo_ctl_el {
    str receiver;
    str sender;

    // If the current record is being processed (waiting for tx callback).
    int processing:1;
    // if this record is being queued for processing.
    int queued:1;

    // Preloaded messages for this receiver:sender key from database.
    gen_lock_t msg_cache_lock;
    unsigned long msg_cache_size;
    msg_cache_el msg_cache_head;
    msg_cache_el msg_cache_tail;
};

/**
 * Structure holding loaded message from the database to be sent.
 * Linked list element.
 */
typedef struct _msg_cache_el
{
    int msgid;
    int flag;
    int retry_ctr; // Retry counter for failed message.

    // Message reconstruction, for sending.
    str msg_from;
    str msg_to;
    str msg_body;
    str msg_ctype;
    time_t msg_rtime;

    // List elements, double linking.
    struct _msg_list_el * prev;
    struct _msg_list_el * next;
} t_msg_cache_el, *msg_cache_el;

/**
 * Whole message cache with locks & so on.
 */
typedef struct _msg_cache
{
    int nrsent;
    int nrdone;
    msg_list_el lsent;
    msg_list_el ldone;
    gen_lock_t  sem_sent;
    gen_lock_t  sem_done;
} t_msg_cache, *msg_cache;



#endif //OPENSIPS_1_11_2_TLS_MESSAGE_CACHE_H
