//
// Created by Dusan Klinec on 18.09.15.
//

#include "MessageThreadManager.hpp"
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
    ShmString * strReceiver = job->body.send.receiver;


}

/**
 * Callback from sender, send(receiver, sender).
 */
void MessageThreadManager::send2(SenderQueueJob * job, MessageThreadSender * sender){
    ShmString * strReceiver = job->body.send.receiver;
    ShmString * strSender = job->body.send.sender;
    if (strReceiver == NULL || strSender == NULL){
        PH_ERR("send2: null receiver || sender");
        return;
    }

    MessageThreadMapKey * mapKey = MessageThreadMapKeyFactory<decltype(this->alloc)>::build(*strReceiver, *strSender, this->alloc);
    MessageThreadMapElement elem = this->getThreadAndLock(*mapKey);

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
        MessageThreadMapElement elem = MessageThreadElementWrapper<decltype(this->alloc)>::build(key.receiver, key.sender, this->alloc);

        threadMap[key] = elem;
        return elem;

    } else {
        return iter->second;
    }
}
