//
// Created by Dusan Klinec on 22.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_APIHELPER_HPP
#define OPENSIPS_1_11_2_TLS_APIHELPER_HPP

#include "common.h"
#include "logic.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

// Hackery aparatus to include c++ incompatible c headers.
#define class xclass
#define delete xdelete
//#define lock xlock
//#define unlock xunlock
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-fpermissive"
#pragma GCC diagnostic warning "-pedantic"
#pragma GCC diagnostic warning "-w"

#include "../../tm/t_hooks.h"

#pragma GCC diagnostic pop
#undef class
#undef delete
#undef lock
#undef unlock

class ApiHelper {
private:
    thread_mgr_api * api;
public:

    int loadMessages(str * uname, str * host, struct db_res** db_res);
    int freeDbResult(struct db_res * res);

    int tmRequest(str* m, str* ru, str* t, str* f, str* h, str* b, str *obu,
                  transaction_cb c, void* cp, release_tmcb_param release_func);

    thread_mgr_api *getApi() const {
        return api;
    }

    void setApi(thread_mgr_api *api) {
        ApiHelper::api = api;
    }
};


#endif //OPENSIPS_1_11_2_TLS_APIHELPER_HPP
