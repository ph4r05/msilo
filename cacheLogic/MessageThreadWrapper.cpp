//
// Created by Dusan Klinec on 20.09.15.
//

#include "MessageThreadWrapper.h"

#include "logic.hpp"
#include "MessageThreadManager.hpp"
#include "MessageThreadSender.hpp"
#include "SipsSHMAllocator.hpp"
#include "SipsHeapAllocator.hpp"
#include "../../../str.h"
#include "../../../dprint.h"
#include "../../../mem/mem.h"
#include "../../tm/t_hooks.h"

thread_mgr* thread_mgr_init(){
    // use SHM allocator to create manager (sender queue, maps, ...).
    SipsSHMAllocator<thread_mgr> structAllocator;
    SipsSHMAllocator<MessageThreadManager> mgrAllocator;
    MainAllocator mAllocator;

    thread_mgr * holder = structAllocator.allocate(1, NULL);
    holder->sender = NULL;

    holder->mgr = (void*) mgrAllocator.allocate(1, NULL);
    mgrAllocator.construct((MessageThreadManager *) holder->mgr, MessageThreadManager(mAllocator));
    PH_INFO("Thread manager constructed\n");

    return holder;
}

int thread_mgr_destroy(thread_mgr *holder){
    if (holder == NULL){
        PH_ERR("Holder is already null in thread_mgr_destroy\n");
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
    PH_INFO("Thread manager destroyed\n");
    return 0;
}

int thread_mgr_init_sender(thread_mgr *holder){
    if (holder == NULL || holder->mgr == NULL){
        PH_ERR("Holder or manager is null in thread_mgr_init_sender\n");
        return -1;
    }

    MessageThreadManager * mgr = (MessageThreadManager*) holder->mgr;

    // Use HEAP allocator to allocate sender.
    SipsHeapAllocator<MessageThreadSender> hAlloc;
    holder->sender = (void*) hAlloc.allocate(1, NULL);
    hAlloc.construct((MessageThreadSender*) holder->sender, MessageThreadSender(mgr, mgr->getJobQueuePtr()));
    PH_INFO("Thread sender constructed\n");

    return 0;
}

int thread_mgr_destroy_sender(thread_mgr *holder){
    if (holder == NULL){
        PH_ERR("Holder is null\n");
        return -1;
    }

    if (holder->sender == NULL){
        PH_DBG("Sender is already destroyed\n");
        return 0;
    }

    SipsHeapAllocator<MessageThreadSender> hAlloc;
    hAlloc.destroy((MessageThreadSender*)holder->sender);
    hAlloc.deallocate((MessageThreadSender*)holder->sender, 1);
    holder->sender = NULL;
    PH_INFO("Thread sender destroyed\n");

    return 0;
}

int thread_mgr_dump(thread_mgr *mgr, struct sip_msg *msg, char *owner, str uname, str host) {
    if (mgr == NULL || mgr->mgr == NULL){
        return -1;
    }

    MessageThreadManager * manager = (MessageThreadManager*) mgr->mgr;
    return manager->dump((MessageThreadSender*)mgr->sender, msg, owner, uname, host);
}

int thread_mgr_clean(thread_mgr *mgr) {
    if (mgr == NULL || mgr->mgr == NULL){
        return -1;
    }

    MessageThreadManager * manager = (MessageThreadManager*) mgr->mgr;
    return manager->clean((MessageThreadSender*)mgr->sender);
}

void thread_mgr_tm_callback(struct cell *t, int type, struct tmcb_params *ps){

    // TODO implement callback.
}
