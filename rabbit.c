//
// Created by Dusan Klinec on 05.11.15.
//

#include "rabbit.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <amqp_ssl_socket.h>
#include <amqp_framing.h>

#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../dprint.h"

static int check_amqp_status(int x, char const *context);
static int check_amqp_error(amqp_rpc_reply_t x, char const *context);

int msilo_rabbit_init(t_msilo_rabbit *rabbit, const char *host, int port, char const *vhost, char const *username, char const *password)
{
    int status;
    amqp_rpc_reply_t reply;

    rabbit->conn = NULL;
    rabbit->socket = NULL;
    rabbit->channel = 0;

    // Allocate new connection.
    rabbit->conn = amqp_new_connection();
    if (rabbit->conn == NULL)
    {
        LM_CRIT("Rabbit connection could not be created");
        return -1;
    }

    // Create a new SSL socket.
    rabbit->socket = amqp_ssl_socket_new(rabbit->conn);
    if (rabbit->socket == NULL) {
        LM_CRIT("Rabbit SSL connection could not be created");
        check_amqp_status(amqp_destroy_connection(rabbit->conn), "Destroy connection");
        rabbit->conn = NULL;
        return -2;
    }

    // Open the SSL socket.
    status = amqp_socket_open(rabbit->socket, host, port);
    if (status) {
        LM_CRIT("Rabbit SSL connection could not be opened");
        check_amqp_error(amqp_connection_close(rabbit->conn, AMQP_REPLY_SUCCESS), "Connection close");
        check_amqp_status(amqp_destroy_connection(rabbit->conn), "Destroy connection");
        rabbit->socket = NULL;
        rabbit->conn = NULL;
        return -3;
    }

    // First channel.
    rabbit->channel = 1;

    // Login attempt.
    reply = amqp_login(rabbit->conn, vhost, 0, 131072, 30, AMQP_SASL_METHOD_PLAIN, username, password);
    status = check_amqp_error(reply, "Logging in");
    if (status < 0){
        check_amqp_error(amqp_connection_close(rabbit->conn, AMQP_REPLY_SUCCESS), "Connection close");
        check_amqp_status(amqp_destroy_connection(rabbit->conn), "Destroy connection");
        rabbit->socket = NULL;
        rabbit->conn = NULL;
        return status;
    }

    // Open channel
    rabbit->channel_ptr = amqp_channel_open(rabbit->conn, rabbit->channel);

    // Check reply
    reply = amqp_get_rpc_reply(rabbit->conn);
    status = check_amqp_error(reply, "Opening channel");
    if (status < 0){
        check_amqp_error(amqp_channel_close(rabbit->conn, rabbit->channel, AMQP_REPLY_SUCCESS), "Channel close");
        check_amqp_error(amqp_connection_close(rabbit->conn, AMQP_REPLY_SUCCESS), "Connection close");
        check_amqp_status(amqp_destroy_connection(rabbit->conn), "Destroy connection");
        rabbit->socket = NULL;
        rabbit->conn = NULL;
        rabbit->channel_ptr = NULL;
        return status;
    }

    rabbit->init_ok = 1;
    return 0;
}

int msilo_rabbit_deinit(t_msilo_rabbit *rabbit)
{
    LM_INFO("Deinitializing RabbitMQ connection");
    check_amqp_error(amqp_channel_close(rabbit->conn, rabbit->channel, AMQP_REPLY_SUCCESS), "Channel close");
    check_amqp_error(amqp_connection_close(rabbit->conn, AMQP_REPLY_SUCCESS), "Connection close");
    check_amqp_status(amqp_destroy_connection(rabbit->conn), "Destroy connection");
    rabbit->socket = NULL;
    rabbit->conn = NULL;
    rabbit->channel_ptr = NULL;
    rabbit->init_ok = 0;
    return 0;
}

int msilo_rabbit_started(t_msilo_rabbit *rabbit)
{
    return rabbit != NULL && rabbit->conn != NULL && rabbit->init_ok != 0;
}

int msilo_rabbit_send(t_msilo_rabbit *rabbit, char const *queue, void *buff, size_t size)
{
    if (rabbit == NULL || rabbit->init_ok != 1)
    {
        LM_ERR("Could not send RabbitMQ message, initialization failed");
        return -1;
    }

    int status = 0;
    amqp_bytes_t message_bytes;
    message_bytes.len = size;
    message_bytes.bytes = buff;


    status = check_amqp_status(amqp_basic_publish(rabbit->conn,
                                    rabbit->channel,
                                    amqp_cstring_bytes("amq.direct"),
                                    amqp_cstring_bytes(queue),
                                    0,
                                    0,
                                    NULL,
                                    message_bytes),
                 "Publishing");

    return status;
}

static int check_amqp_status(int x, char const *context)
{
    if (x < 0) {
        LM_ERR("%s: %s\n", context, amqp_error_string2(x));
    }

    return x;
}

static int check_amqp_error(amqp_rpc_reply_t x, char const *context)
{
    switch (x.reply_type) {
        case AMQP_RESPONSE_NORMAL:
            return 0;

        case AMQP_RESPONSE_NONE:
            LM_ERR("%s: missing RPC reply type!\n", context);
            return -10;

        case AMQP_RESPONSE_LIBRARY_EXCEPTION:
            LM_ERR("%s: %s\n", context, amqp_error_string2(x.library_error));
            return -11;

        case AMQP_RESPONSE_SERVER_EXCEPTION:
            switch (x.reply.id) {
                case AMQP_CONNECTION_CLOSE_METHOD: {
                    amqp_connection_close_t *m = (amqp_connection_close_t *) x.reply.decoded;
                    LM_ERR("%s: server connection error %d, message: %.*s\n",
                           context,
                           m->reply_code,
                           (int) m->reply_text.len, (char *) m->reply_text.bytes);

                    return -20;
                }
                case AMQP_CHANNEL_CLOSE_METHOD: {
                    amqp_channel_close_t *m = (amqp_channel_close_t *) x.reply.decoded;
                    LM_ERR("%s: server channel error %d, message: %.*s\n",
                           context,
                           m->reply_code,
                           (int) m->reply_text.len, (char *) m->reply_text.bytes);

                    return -21;
                }
                default:
                    LM_ERR("%s: unknown server error, method id 0x%08X\n", context, x.reply.id);
                    return -22;
            }
    }

    return -30;
}