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
    // TODO: implement.
    return 0;
}

/**
 * Cleaning task, periodically called by timer thread;
 */
int MessageThreadManager::clean(MessageThreadSender * sender){
    // TODO: implement.
    return 0;
}

/**
 * Callback from sender, send(receiver).
 */
void MessageThreadManager::send1(SenderQueueJob * job, MessageThreadSender * sender){
    // TODO: implement.
}

/**
 * Callback from sender, send(receiver, sender).
 */
void MessageThreadManager::send2(SenderQueueJob * job, MessageThreadSender * sender){
    // TODO: implement.
}

