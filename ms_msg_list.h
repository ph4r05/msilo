/**
 * $Id$
 *
 * MSILO module
 *
 * Copyright (C) 2001-2003 FhG Fokus
 *
 * This file is part of opensips, a free SIP server.
 *
 * opensips is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * opensips is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * History:
 * --------
 *  2003-03-11  major locking changes, now it uses locking.h (andrei)
 */

#ifndef _MS_MSG_LIST_H_
#define _MS_MSG_LIST_H_


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

typedef struct _msg_list_el
{
	t_msg_mid msgid;
	int flag;
	int retry_ctr; // Retry counter for failed message.
	struct _msg_list_el * prev;
	struct _msg_list_el * next;
} t_msg_list_el, *msg_list_el;

typedef struct _msg_list
{
	long nrsent;
	long nrdone;
	msg_list_el lsent;
	msg_list_el ldone;
	gen_lock_t  sem_sent;
	gen_lock_t  sem_done;
} t_msg_list, *msg_list;

msg_list_el msg_list_el_new();
void msg_list_el_free(msg_list_el);
void msg_list_el_free_all(msg_list_el);

msg_list msg_list_init();
void msg_list_free(msg_list);
int msg_list_check_msg(msg_list, t_msg_mid, int *, int *);
int msg_list_set_flag(msg_list, t_msg_mid, int);
int msg_list_check(msg_list);
msg_list_el msg_list_reset(msg_list);

#endif

