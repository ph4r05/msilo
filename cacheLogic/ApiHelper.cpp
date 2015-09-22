//
// Created by Dusan Klinec on 22.09.15.
//

#include "ApiHelper.hpp"
#include "common.h"
#include "apiDeps.h"

#define class xclass
#define delete xdelete
//#define lock xlock
//#define unlock xunlock
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-fpermissive"
#pragma GCC diagnostic warning "-pedantic"
#pragma GCC diagnostic warning "-w"

#include "../../tm/tm_load.h"

#pragma GCC diagnostic pop
#undef class
#undef delete
#undef lock
#undef unlock

int ApiHelper::loadMessages(str *uname, str *host, struct db_res **db_res) {
    if (api == NULL || api->load_messages == NULL){
        PH_ERR("Could not use API call.\n");
        return -1;
    }

    return api->load_messages(uname, host, db_res);
}

int ApiHelper::freeDbResult(struct db_res *res) {
    if (api == NULL || api->db_con == NULL || api->msilo_dbf == NULL){
        PH_ERR("Could not use API call.\n");
        return -1;
    }

    return api->msilo_dbf->free_result(api->db_con, res);
}


int ApiHelper::tmRequest(str *m, str *ru, str *t, str *f, str *h, str *b, str *obu, transaction_cb c, void *cp,
                         release_tmcb_param release_func) {

    if (api == NULL || api->tmb == NULL){
        PH_ERR("Could not use API call.\n");
        return -1;
    }

    return api->tmb->t_request(m, ru, t, f, h, b, obu, c, cp, release_func);
}
