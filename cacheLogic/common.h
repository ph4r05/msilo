//
// Created by Dusan Klinec on 21.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_COMMON_H
#define OPENSIPS_1_11_2_TLS_COMMON_H

struct tm_binds;
struct db_func;

// C-style wrapper for C++ thread manager & sender manager.
typedef struct {
    void *mgr;
    void *sender;
    struct db_func * msilo_dbf;
    struct tm_binds * tmb;
} thread_mgr;

// Structure to be passed as a parameter to the transaction module.
typedef struct {
    void *mgr;
    void *sender;
    void *mapElement;
    long mid;
} thread_tsx_callback;

#endif //OPENSIPS_1_11_2_TLS_COMMON_H
