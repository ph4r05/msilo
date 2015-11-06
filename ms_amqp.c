//
// Created by Dusan Klinec on 05.11.15.
//


#include "ms_amqp.h"
#ifdef MS_AMQP

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <amqp_ssl_socket.h>
#include <amqp_tcp_socket.h>
#include <amqp_framing.h>

#include "../../mem/mem.h"
#include "../../mem/shm_mem.h"
#include "../../dprint.h"

static int check_amqp_status(int x, char const *context);
static int check_amqp_error(amqp_rpc_reply_t x, char const *context);
static int amqp_free_config(t_msilo_amqp *amqp);

static int ms_amqp_connect(t_msilo_amqp *amqp);
static int ms_amqp_disconnect(t_msilo_amqp *amqp);

int msilo_amqp_init(t_msilo_amqp *amqp, const char *host, int port, char const *vhost, char const *username, char const *password)
{
    int status;

    amqp->conn = NULL;
    amqp->socket = NULL;
    amqp->channel = 0;
    amqp->retry_count = 3;

    // Copy configuration.
    amqp->port = port;
    amqp->host = strdup(host);
    amqp->vhost = strdup(vhost);
    amqp->username = strdup(username);
    amqp->password = strdup(password);

    // Allocate new connection.
    status = ms_amqp_connect(amqp);

    amqp->init_ok = 1;
    return status;
}

int msilo_amqp_deinit(t_msilo_amqp *amqp)
{
    LM_INFO("Deinitializing RabbitMQ connection");
    ms_amqp_disconnect(amqp);
    amqp->init_ok = 0;
    amqp_free_config(amqp);
    return 0;
}

int msilo_amqp_started(t_msilo_amqp *amqp)
{
    return amqp != NULL && amqp->conn != NULL && amqp->init_ok != 0;
}

int msilo_amqp_send(t_msilo_amqp *amqp, char const *queue, void *buff, size_t size)
{
    if (amqp == NULL || amqp->init_ok != 1)
    {
        LM_ERR("Could not send RabbitMQ message, initialization failed");
        return -1;
    }

    int status = -1;
    int retry = 0;
    int try_reconnect = 0;
    amqp_bytes_t message_bytes;
    message_bytes.len = size;
    message_bytes.bytes = buff;

    for(retry = 0; retry <= amqp->retry_count && status != 0; retry ++)
    {
        if (try_reconnect)
        {
            ms_amqp_disconnect(amqp);
            int recon_status = ms_amqp_connect(amqp);
            LM_INFO("Reconnecting AMQP, status: %d, %d/%d", recon_status, retry, amqp->retry_count);

            if (recon_status != 0)
            {
                continue;
            }
            else
            {
                try_reconnect = 0;
            }
        }

        errno = 0;
        status = check_amqp_status(amqp_basic_publish(amqp->conn,
                                                      amqp->channel,
                                                      amqp_cstring_bytes(""),
                                                      amqp_cstring_bytes(queue),
                                                      0,
                                                      0,
                                                      NULL,
                                                      message_bytes), "Publishing");

        if (status < 0)
        {
            LM_ERR("AMQP publication failed [%d/%d], status: %d, errno: %d, queue: %s, message: %.*s",
                   retry, amqp->retry_count, status, errno, queue,
                   (int) size, (char *) buff);
            try_reconnect = 1;
        }
    }

    return status;
}

static int ms_amqp_connect(t_msilo_amqp *amqp)
{
    amqp_rpc_reply_t reply;
    int status = 0;

    // Allocate new connection.
    amqp->conn = amqp_new_connection();
    if (amqp->conn == NULL)
    {
        LM_CRIT("Rabbit connection could not be created");
        return -1;
    }

    // Create a new SSL socket.
    amqp->socket = amqp_tcp_socket_new(amqp->conn);
    if (amqp->socket == NULL) {
        LM_CRIT("Rabbit TCP connection could not be created");
        check_amqp_status(amqp_destroy_connection(amqp->conn), "Connection destroy");
        amqp->conn = NULL;
        return -2;
    }

    // Open the SSL socket.
    status = amqp_socket_open(amqp->socket, amqp->host, amqp->port);
    if (status) {
        LM_CRIT("Rabbit TCP connection could not be opened at %s:%d, status: %d", amqp->host, amqp->port, status);
        check_amqp_error(amqp_connection_close(amqp->conn, AMQP_REPLY_SUCCESS), "Connection close");
        check_amqp_status(amqp_destroy_connection(amqp->conn), "Connection destroy");
        amqp->socket = NULL;
        amqp->conn = NULL;
        return -3;
    }

    // First channel.
    amqp->channel = 1;

    // Login attempt.
    reply = amqp_login(amqp->conn, amqp->vhost, 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, amqp->username, amqp->password);
    status = check_amqp_error(reply, "Logging in");
    if (status < 0){
        LM_CRIT("Rabbit login failed, vhost: %s, user: %s, status: %d", amqp->vhost, amqp->username, status);
        check_amqp_error(amqp_connection_close(amqp->conn, AMQP_REPLY_SUCCESS), "Connection close");
        check_amqp_status(amqp_destroy_connection(amqp->conn), "Connection destroy");
        amqp->socket = NULL;
        amqp->conn = NULL;
        return status;
    }

    // Open channel
    amqp->channel_ptr = amqp_channel_open(amqp->conn, amqp->channel);

    // Check reply
    reply = amqp_get_rpc_reply(amqp->conn);
    status = check_amqp_error(reply, "Opening channel");
    if (status < 0){
        LM_CRIT("Rabbit opening channel failed, status: %d", status);
        check_amqp_error(amqp_channel_close(amqp->conn, amqp->channel, AMQP_REPLY_SUCCESS), "Channel close");
        check_amqp_error(amqp_connection_close(amqp->conn, AMQP_REPLY_SUCCESS), "Connection close");
        check_amqp_status(amqp_destroy_connection(amqp->conn), "Connection destroy");
        amqp->socket = NULL;
        amqp->conn = NULL;
        amqp->channel_ptr = NULL;
        return status;
    }

    return 0;
}

static int ms_amqp_disconnect(t_msilo_amqp *amqp)
{
    if (amqp->conn != NULL)
    {
        check_amqp_error(amqp_channel_close(amqp->conn, amqp->channel, AMQP_REPLY_SUCCESS), "Channel close");
        check_amqp_error(amqp_connection_close(amqp->conn, AMQP_REPLY_SUCCESS), "Connection close");
        check_amqp_status(amqp_destroy_connection(amqp->conn), "Connection destroy");
    }

    amqp->socket = NULL;
    amqp->conn = NULL;
    amqp->channel_ptr = NULL;
    return 0;
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

static void helper_free(void ** ptr)
{
    if (ptr == NULL || *ptr == NULL)
    {
        return;
    }

    free(*ptr);
    *ptr = NULL;
}

static int amqp_free_config(t_msilo_amqp *amqp)
{
    if (amqp == NULL)
    {
        return -1;
    }

    helper_free((void**)&(amqp->host));
    helper_free((void**)&(amqp->vhost));
    helper_free((void**)&(amqp->username));
    helper_free((void**)&(amqp->password));
    return 0;
}

#endif
