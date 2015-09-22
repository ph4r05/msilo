//
// Created by Dusan Klinec on 18.09.15.
//

#include "MessageThreadManager.hpp"
#include "common.h"

// Hackery aparatus to include c++ incompatible c headers.
#define class xclass
#define delete xdelete
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-fpermissive"
#pragma GCC diagnostic warning "-pedantic"
#pragma GCC diagnostic warning "-w"
#include "../../../db/db.h"
#pragma GCC diagnostic pop
#undef class
#undef delete

using namespace std;
using namespace boost::interprocess;

MessageThreadManager::MessageThreadManager(const MessageThreadManager &src) {
    this->alloc = src.getAlloc();
}

int MessageThreadManager::dump(MessageThreadSender * sender, struct sip_msg *msg, char *owner, str uname, str host){
    // Testing purposes, handle messages only if uname starts with "test-"
    if (strncmp(uname.s, "test-", std::min((size_t)uname.len, (size_t)5)) != 0){
        return -1;
    }

    PH_INFO("uname starts with test- %.*s", uname.len, uname.s);
    SenderQueueJob * job = new SenderQueueJob();
    job->type = JOB_TYPE_SEND_RECEIVER;

    std::string user = ph4::Utils::getUsername(uname, host);
    ShmString receiver(user.c_str(), this->alloc);

    job->receiver = receiver;
    this->send1(job, sender);
    delete job;

    //TODO: load all non-loaded messages from database, create threads.
    //TODO: offer all loaded messages to the threads objects.
    //TODO: start new sending if thread object is in NONE state or does not exist.
    return 0;
}

/**
 * Cleaning task, periodically called by timer thread;
 */
int MessageThreadManager::clean(MessageThreadSender * sender){
    PH_INFO("Manager clean");
    // TODO: implement.
    // TODO: delete DB entries with finished flag in MID container.
    // TODO: prune MID container.
    return 0;
}

int MessageThreadManager::tsx_callback(MessageThreadSender *sender, int code, MessageThreadElement *mapElem, long mid) {
    // TODO: implement
    PH_INFO("Tsx callbach, code: %d, mid: %ld", code, mid);
    const bool sentOK = code < 300;

    // TODO: if code is not valid -> message.errorCount +=1. if reached threshold, disable sending, stop sender. Do nothing (user gone).
    // TODO: if code is valid -> 1. mark as sent in db register. (will be deleted in the next cleaning, cleaning prunes deleted/failed messages from register).
    // TODO:                     2. start a new job with send(receiver, sender).
    return 0;
}

/**
 * Callback from sender, send(receiver).
 */
void MessageThreadManager::send1(SenderQueueJob * job, MessageThreadSender * sender){
    // TODO: implement.
    // TODO: load data from database
    ShmString & strReceiver = job->receiver;

    // Executed in connection thread or worker thread.
    //TODO: load all non-loaded messages from database, create threads.
    // TODO: split on "@"
    str uname;
    str host;
    struct db_res * db_res = NULL;
    int loadRes = api.load_messages(&uname, &host, &db_res);
    if (loadRes < 0){
        PH_WARN("DB loading error, result: %d", loadRes);
    } else {
        // TODO: implement message loading.
        LM_INFO("x: dumping [%d] messages for <%s>\n", RES_ROW_N(db_res), strReceiver.c_str());

//        for(i = 0; i < RES_ROW_N(db_res); i++)
//        {
//            mid =  RES_ROWS(db_res)[i].values[0].val.int_val;
//            if(msg_list_check_msg(ml, mid))
//            {
//                LM_INFO("message[%d] mid=%d already sent.\n", i, mid);
//                continue;
//            }
//
//            memset(str_vals, 0, 4*sizeof(str));
//            SET_STR_VAL(str_vals[0], db_res, i, 1); /* from */
//            SET_STR_VAL(str_vals[1], db_res, i, 2); /* to */
//            SET_STR_VAL(str_vals[2], db_res, i, 3); /* body */
//            SET_STR_VAL(str_vals[3], db_res, i, 4); /* ctype */
//            rtime =
//                    (time_t)RES_ROWS(db_res)[i].values[5/*inc time*/].val.int_val;
//
//            hdr_str.len = 1024;
//            if(m_build_headers(&hdr_str, str_vals[3] /*ctype*/,
//                               str_vals[0]/*from*/, rtime /*Date*/, (long) (dumpId * 1000l)) < 0)
//            {
//                LM_ERR("headers building failed [%d]\n", mid);
//                if (msilo_dbf.free_result(db_con, db_res) < 0)
//                    LM_ERR("failed to free the query result\n");
//                msg_list_set_flag(ml, mid, MS_MSG_ERRO);
//                goto error;
//            }
//
//            LM_DBG("msg [%d-%d] for: %.*s\n", i+1, mid,	pto->uri.len, pto->uri.s);
//
//            /** sending using TM function: t_uac */
//            body_str.len = 1024;
//            n = m_build_body(&body_str, rtime, str_vals[2/*body*/], 0);
//            if(n<0)
//                LM_DBG("sending simple body\n");
//            else
//                LM_DBG("sending composed body\n");
//
//            int res = tmb.t_request(&msg_type,  /* Type of the message */
//                                    &str_vals[1],     /* Request-URI (To) */
//                                    &str_vals[1],     /* To */
//                                    &str_vals[0],     /* From */
//                                    &hdr_str,         /* Optional headers including CRLF */
//                                    (n<0)?&str_vals[2]:&body_str, /* Message body */
//                                    (ms_outbound_proxy.s)?&ms_outbound_proxy:0,
//                    /* outbound uri */
//                                    m_tm_callback,    /* Callback function */
//                                    (void*)(long)mid, /* Callback parameter */
//                                    NULL
//            );
//
//            if (res < 0){
//                LM_WARN("message sending failed [%d], res=%d messages for <%.*s>!\n",
//                        mid, res, pto->uri.len, pto->uri.s);
//            }
//        }
    }

    //TODO: offer all loaded messages to the threads objects.
    //TODO: start new sending if thread object is in NONE state or does not exist.

}

/**
 * Callback from sender, send(receiver, sender).
 */
void MessageThreadManager::send2(SenderQueueJob * job, MessageThreadSender * sender){
    ShmString & strReceiver = job->receiver;
    ShmString & strSender = job->sender;
    if (strReceiver.empty() || strSender.empty()){
        PH_ERR("send2: null receiver || sender");
        return;
    }

    MessageThreadMapKey * mapKey = MessageThreadMapKeyFactory<decltype(this->alloc)>::build(strReceiver, strSender, this->alloc);
    MessageThreadMapElement elem = this->getThreadAndLock(*mapKey);
    if (elem == nullptr){
        PH_WARN("Could not allocate cache entry");
        return;
    }

    // Lock a record mutex, going to operate.
    bip::scoped_lock<bip::interprocess_mutex> lock(elem->getMutex());
    if (elem->getMsg_cache_size() > 0){
        // TODO: if message cache contains data: pop one message, mark as sending, send. Wait for transaction callback.
    } else {
        // TODO: if empty cache: load new messages from database.
    }

    // TODO: implement.
    // TODO: empty cache & empty database -> set to NONE, add to pool, dealloc, stop.
}


MessageThreadElement * MessageThreadManager::getThreadAndLock(const MessageThreadMapKey & key) {
    // Lock global manager mutex, map manipulation in place.
    bip::scoped_lock<bip::interprocess_mutex> lock(this->mutex);

    MessageThreadMap::const_iterator iter = threadMap.find(key);
    if(iter == threadMap.end()){
        // Allocating a new one, on SHM.
        // TODO: add LRU/POOLing.
        MessageThreadMapElement elem = NULL;
        try {
            elem = MessageThreadElementWrapper<decltype(this->alloc)>::build(key.receiver, key.sender, this->alloc);
            threadMap[key] = elem;

        } catch(std::bad_alloc){
            PH_WARN("Bad allocation in message thread element alloc");
        }

        return elem;

    } else {
        return iter->second;
    }
}
