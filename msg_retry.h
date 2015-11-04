//
// Created by Dusan Klinec on 26.10.15.
//

#ifndef OPENSIPS_1_11_2_TLS_MSG_RETRY_H
#define OPENSIPS_1_11_2_TLS_MSG_RETRY_H

#include "../../locking.h"
#include "msilo.h"

#define MS_MSG_NULL	(0)
#define MS_MSG_SENT	(1<<0)
#define MS_MSG_DONE	(1<<2)
#define MS_MSG_ERRO (1<<3)
#define MS_MSG_TSND	(1<<4)

// Message was sent, but delivery transaction failed.
// Retry until MS_MSG_RETRY_LIMIT retry count is reached. Then switch to failed state.
#define MS_MSG_RETRY (1<<5)

// Message is queued in the sending list, do not delete it from the list.
// Sender thread takes care about it.
#define MS_MSG_QUEUED (1<<6)

#define MS_SEM_SENT	0
#define MS_SEM_DONE 1

#define MSG_LIST_OK		0
#define MSG_LIST_ERR	-1
#define MSG_LIST_EXIST	1

#define MS_MSG_RETRY_LIMIT 12

typedef struct _retry_list_el
{
    t_msg_mid msgid;
    int flag;

    int retry_ctr;
    time_t not_before;

    struct _retry_list_el * clone;
    struct _retry_list_el * prev;
    struct _retry_list_el * next;
} t_retry_list_el, *retry_list_el;

typedef struct _retry_list
{
    long nrretry;
    retry_list_el lretry_new;
    retry_list_el lretry_pop;
    gen_lock_t  sem_retry;
} t_retry_list, *retry_list;

retry_list_el retry_list_el_new();
void retry_list_el_free(retry_list_el);
void retry_list_el_free_all(retry_list_el);

retry_list retry_list_init();
void retry_list_free(retry_list);
int retry_add_element(retry_list ml, t_msg_mid mid, int retry_ctr, time_t not_before);
retry_list_el retry_peek_n(retry_list ml, size_t n, size_t * size);
int retry_is_empty(retry_list ml);
retry_list_el retry_list_reset(retry_list ml);

void retry_list_el_free_prev_all(retry_list_el mle);
void retry_clone_element(const retry_list_el src, retry_list_el dst);
retry_list_el retry_clone_elements_prev_local(retry_list_el p0);

//int retry_list_set_flag(retry_list, int, int);
//int retry_list_should_retry(retry_list ml, int mid, int limit, int * retryCnt, int fl);
//int retry_list_check(retry_list);



#endif //OPENSIPS_1_11_2_TLS_MSG_RETRY_H
