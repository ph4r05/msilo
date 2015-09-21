//
// Created by Dusan Klinec on 20.09.15.
//

#include "MessageThreadWrapper.h"
#include "MessageThreadManager.hpp"
#include "MessageThreadSender.hpp"
#include "SipsSHMAllocator.hpp"
#include "SipsHeapAllocator.hpp"
#include "../../../str.h"
#include "../../../dprint.h"
#include "../../../mem/mem.h"

thread_mgr* thread_mgr_init(){
    // use SHM allocator to create manager (sender queue, maps, ...).
    SipsSHMAllocator<thread_mgr> structAllocator;
    SipsSHMAllocator<MessageThreadManager> mgrAllocator;
    MainAllocator mAllocator;

    thread_mgr * holder = structAllocator.allocate(1, NULL);
    holder->sender = NULL;

    holder->mgr = (void*) mgrAllocator.allocate(1, NULL);
    mgrAllocator.construct((MessageThreadManager *) holder->mgr, MessageThreadManager(mAllocator));
    return holder;
}

int thread_mgr_destroy(thread_mgr *holder){
    if (holder == NULL){
        LM_ERR("Holder is already null in thread_mgr_destroy");
        return -1;
    }

    SipsSHMAllocator<thread_mgr> structAllocator;

    // Dealloc manager.
    if (holder->mgr != NULL){
        SipsSHMAllocator<MessageThreadManager> mgrAllocator;
        mgrAllocator.destroy((MessageThreadManager*) holder->mgr);
        mgrAllocator.deallocate((MessageThreadManager*) holder->mgr, 1);
        holder->mgr = NULL;
    }

    // Dealloc struct.
    structAllocator.deallocate(holder, 1);
    return 0;
}

int thread_mgr_init_sender(thread_mgr *holder){
    if (holder == NULL || holder->mgr == NULL){
        LM_ERR("Holder or manager is null in thread_mgr_init_sender");
        return -1;
    }

    MessageThreadManager * mgr = (MessageThreadManager*) holder->mgr;

    // Use HEAP allocator to allocate sender.
    SipsHeapAllocator<MessageThreadSender> hAlloc;
    holder->sender = (void*) hAlloc.allocate(1, NULL);
    hAlloc.construct((MessageThreadSender*) holder->sender, MessageThreadSender(&(mgr->jobQueue)));
    return 0;
}

int thread_mgr_destroy_sender(thread_mgr *holder){
    if (holder == NULL){
        LM_ERR("Holder is null");
        return -1;
    }

    if (holder->sender == NULL){
        LM_DBG("Sender is already destroyed");
        return 0;
    }

    SipsHeapAllocator<MessageThreadSender> hAlloc;
    hAlloc.destroy((MessageThreadSender*)holder->sender);
    hAlloc.deallocate((MessageThreadSender*)holder->sender, 1);
    holder->sender = NULL;
    return 0;
}

int thread_mgr_dump(thread_mgr *mgr, struct sip_msg *msg, char *owner, str uname, str host) {
    if (mgr == NULL || mgr->mgr == NULL){
        return -1;
    }

    MessageThreadManager * manager = (MessageThreadManager*) holder->mgr;
    return manager->dump(msg, owner, uname, host);
}

int thread_mgr_clean(thread_mgr *mgr) {
    if (mgr == NULL || mgr->mgr == NULL){
        return -1;
    }

    MessageThreadManager * manager = (MessageThreadManager*) holder->mgr;
    return manager->clean();
}
