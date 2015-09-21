//
// Created by Dusan Klinec on 18.09.15.
//

#include "MessageThreadManager.hpp"
using namespace std;
using namespace boost::interprocess;

int MessageThreadManager::dump(struct sip_msg *msg, char *owner, str uname, str host){
    // TODO: implement.
    return 0;
}

/**
 * Cleaning task, periodically called by timer thread;
 */
int MessageThreadManager::clean(){
    // TODO: implement.
    return 0;
}