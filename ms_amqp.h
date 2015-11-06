//
// Created by Dusan Klinec on 05.11.15.
//

#ifndef OPENSIPS_1_11_2_TLS_RABBIT_H
#define OPENSIPS_1_11_2_TLS_RABBIT_H

#include "msilo.h"

#ifdef MS_AMQP
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

//
// Using library:
// https://github.com/alanxz/rabbitmq-c
//

typedef struct t_msilo_amqp
{
    amqp_connection_state_t     conn;
    amqp_socket_t               *socket;
    amqp_channel_t              channel;
    amqp_channel_open_ok_t     *channel_ptr;
    int init_ok;
    int retry_count;

    // For eventual reconnection.
    char *host;
    int port;
    char *vhost;
    char *username;
    char *password;
} t_msilo_amqp;

int msilo_amqp_init(t_msilo_amqp *amqp, const char *host, int port, char const *vhost, char const *username, char const *password);
int msilo_amqp_deinit(t_msilo_amqp *amqp);
int msilo_amqp_started(t_msilo_amqp *amqp);
int msilo_amqp_send(t_msilo_amqp *amqp, char const *queue, void *buff, size_t size);

#endif
#endif //OPENSIPS_1_11_2_TLS_RABBIT_H
