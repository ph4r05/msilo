//
// Created by Dusan Klinec on 20.09.15.
//

#include "MessageThreadWrapper.hpp"
#include "MessageThreadManager.hpp"
#include "MessageThreadSender.hpp"
#include "SipsSHMAllocator.hpp"
#include "SipsHeapAllocator.hpp"


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
        return -1;
    }

    SipsSHMAllocator<thread_mgr> structAllocator;

    // Dealloc manager.
    if (holder->mgr != NULL){
        SipsSHMAllocator<MessageThreadManager> mgrAllocator;
        mgrAllocator.destroy((MessageThreadManager*) holder->mgr)
        mgrAllocator.deallocate((MessageThreadManager*) holder->mgr, 1);
        holder->mgr = NULL;
    }

    // Dealloc struct.
    structAllocator.deallocate(holder, 1);
    return 0;
}

int thread_mgr_init_sender(thread_mgr *holder){
    // TODO: use HEAP allocator to allocate sender.
    if (holder == NULL){
        return -1;
    }

    MessageThreadManager * mgr = (MessageThreadManager*) holder->mgr;

    SipsHeapAllocator<MessageThreadSender> hAlloc;
    holder->sender = (void*) hAlloc.allocate(1, NULL);
    hAlloc.construct((MessageThreadSender*) holder->sender, MessageThreadSender(&(mgr->jobQueue)));
    return 0;
}

int thread_mgr_destroy_sender(thread_mgr *holder){
    // TODO: implement.
    if (holder == NULL || holder->sender == NULL){
        return -1;
    }

    SipsHeapAllocator<MessageThreadSender> hAlloc;
    hAlloc.destroy((MessageThreadSender*)holder->sender);
    hAlloc.deallocate((MessageThreadSender*)holder->sender, 1);
    holder->sender = NULL;
    return 0;
}





