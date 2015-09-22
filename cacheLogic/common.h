//
// Created by Dusan Klinec on 21.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_COMMON1_H
#define OPENSIPS_1_11_2_TLS_COMMON1_H

#include "../../../db/db_con.h"
struct tm_binds;
struct db_func;
struct db_res;

// C-style wrapper for C++ thread manager & sender manager.
typedef struct {
    void *mgr;
    void *sender;
    struct db_func * msilo_dbf;
    struct tm_binds * tmb;
} thread_mgr;

// API function to load messages for given user.
typedef int (*load_messages_t)(str * uname, str * host, struct db_res** db_res);

// API for manager, provided by module.
typedef struct {
    struct db_func * msilo_dbf;
    struct tm_binds * tmb;
    db_con_t * db_con;
    load_messages_t load_messages;

} thread_mgr_api;

// Structure to be passed as a parameter to the transaction module.
typedef struct {
    void *mgr;
    void *sender;
    void *mapElement;
    long mid;
} thread_tsx_callback;

#endif //OPENSIPS_1_11_2_TLS_COMMON1_H
