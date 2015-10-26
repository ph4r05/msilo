//
// Created by Dusan Klinec on 26.10.15.
//

#include "msg_retry.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../dprint.h"

/**
 * create a new element
 */
retry_list_el retry_list_el_new(void)
{
    retry_list_el mle = NULL;
    mle = (retry_list_el)shm_malloc(sizeof(t_retry_list_el));
    if(mle == NULL)
        return NULL;

    mle->next = NULL;
    mle->prev = NULL;
    mle->msgid = 0;
    mle->retry_ctr = 0;
    mle->not_before = 0;
    mle->flag = MS_MSG_NULL;

    return mle;
}

/**
 * free an element
 */
void retry_list_el_free(retry_list_el mle)
{
    if(mle)
        shm_free(mle);
}

/**
 * free a list of elements
 */
void retry_list_el_free_all(retry_list_el mle)
{
    retry_list_el p0, p1;

    if(!mle)
        return;

    p0 = mle;
    while(p0)
    {
        p1 = p0;
        p0 = p0->next;
        retry_list_el_free(p1);
    }
}

/**
 * init a list
 */
retry_list retry_list_init(void)
{
    retry_list ml = NULL;

    ml = (retry_list)shm_malloc(sizeof(t_retry_list));
    if(ml == NULL)
        return NULL;
    /* init locks */
    if (lock_init(&ml->sem_retry)==0){
        LM_CRIT("could not initialize a lock\n");
        goto clean;
    };
    ml->nrretry = 0;
    ml->lretry_new = NULL;
    ml->lretry_pop = NULL;

    return ml;

    clean:
    shm_free(ml);
    return NULL;
}

/**
 * free a list
 */
void retry_list_free(retry_list ml)
{
    retry_list_el p0, p1;

    if(!ml)
        return;

    lock_destroy(&ml->sem_retry);

    if(ml->nrretry>0 && ml->lretry_new)
    { // free sent list
        p0 = ml->lretry_new;
        ml->lretry_new = NULL;
        ml->nrretry = 0;
        while(p0)
        {
            p1 = p0->next;
            retry_list_el_free(p0);
            p0 = p1;
        }
    }

    shm_free(ml);
}

/**
 * adds given entry to the retry list.
 */
int retry_add_element(retry_list ml, int mid, int retry_ctr, time_t not_before)
{
    retry_list_el p0;

    if(!ml || mid==0)
    {
        goto errorx;
    }

    LM_DBG("adding msgid=%d\n", mid);

    lock_get(&ml->sem_retry);

    // When calling this we should be pretty sure record is noy already in the queue
    // thus skipping O(n) scanning.
    p0 = retry_list_el_new();
    if(!p0)
    {
        LM_ERR("failed to create new msg elem.\n");
        goto error;
    }

    p0->msgid = mid;
    p0->flag |= MS_MSG_SENT;
    p0->retry_ctr = retry_ctr;
    p0->not_before = not_before;
    p0->next = ml->lretry_new;
    p0->prev = NULL;

    if(ml->lretry_new)
    {
        ml->lretry_new->prev = p0;
    }

    if (ml->lretry_pop == NULL){
        ml->lretry_pop = p0;
    }

    ml->lretry_new = p0;
    ml->nrretry++;

    lock_release(&ml->sem_retry);
    LM_DBG("msg added to sent list.\n");
    return MSG_LIST_OK;
error:
    lock_release(&ml->sem_retry);
errorx:
    return MSG_LIST_ERR;
}

/**
 * Removes first N elements from the queue pop end.
 */
retry_list_el retry_peek_n(retry_list ml, size_t n, size_t * size){
    retry_list_el p0 = NULL, p1 = NULL, p_ret = NULL;
    size_t cur_ctr = 0;

    if(!ml) {
        goto errorx;
    }

    lock_get(&ml->sem_retry);

    p1 = p0 = ml->lretry_pop;
    while(p0) {
        p1 = p0;
        cur_ctr += 1;
        if (cur_ctr >= n){
            break;
        }

        p0 = p0->prev;
    }

    // Nothing to provide
    if (ml->lretry_pop == NULL){
        *size = 0;
        goto nulz;
    }

    // User iterates over prev pointers. First one is on the top.
    p_ret = ml->lretry_pop;
    p_ret->next = NULL;
    *size = cur_ctr;
    ml->nrretry -= cur_ctr;

    // New queue start is p1->prev as p1 is the last element in the chain.
    ml->lretry_pop = p1->prev;
    p1->prev = NULL;
    if (ml->lretry_pop){
        ml->lretry_pop->next = NULL;
    } else {
        ml->lretry_new = NULL;
        if (ml->nrretry != 0){
            LM_CRIT("Num of nodes does not match: %d!", ml->nrretry);
        }
    }

    LM_INFO("Items removed (peek): %d, remaining: %d, new: %p, pop: %p, ret: %p, p1: %p",
            (int)cur_ctr, (int)ml->nrretry, ml->lretry_new, ml->lretry_pop, p_ret, p1);

    lock_release(&ml->sem_retry);
    return p_ret;
nulz:
    lock_release(&ml->sem_retry);
    return p_ret;
errorx:
    return p_ret;
}

int retry_is_empty(retry_list ml) {
    int ret = 1;
    if(!ml) {
        goto errorx;
    }

    lock_get(&ml->sem_retry);
    ret = ml->nrretry > 0;
    lock_release(&ml->sem_retry);

    return ret;
errorx:
    return ret;
}

/**
 * reset a list
 * return old list
 */
retry_list_el retry_list_reset(retry_list ml)
{
    retry_list_el p0;

    if(!ml)
        return NULL;

    lock_get(&ml->sem_retry);
    p0 = ml->lretry_pop;
    ml->lretry_pop = NULL;
    ml->lretry_new = NULL;
    ml->nrretry = 0;
    lock_release(&ml->sem_retry);

    return p0;
}
