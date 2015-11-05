//
// Created by Dusan Klinec on 05.11.15.
//

#ifndef OPENSIPS_1_11_2_TLS_RABBIT_H
#define OPENSIPS_1_11_2_TLS_RABBIT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#define RABBIT_QUEUE_NAME 255
typedef struct t_msilo_rabbit
{
    amqp_connection_state_t     conn;
    amqp_socket_t               *socket;
    amqp_channel_t              channel;
    amqp_channel_open_ok_t     *channel_ptr;
    int init_ok;
} t_msilo_rabbit;

int msilo_rabbit_init(t_msilo_rabbit *rabbit, const char *host, int port, char const *vhost, char const *username, char const *password);
int msilo_rabbit_deinit(t_msilo_rabbit *rabbit);
int msilo_rabbit_started(t_msilo_rabbit *rabbit);
int msilo_rabbit_send(t_msilo_rabbit *rabbit, char const *queue, void *buff, size_t size);


#endif //OPENSIPS_1_11_2_TLS_RABBIT_H
