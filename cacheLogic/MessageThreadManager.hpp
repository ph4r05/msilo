//
// Created by Dusan Klinec on 18.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_MESSAGETHREADMANAGER_H
#define OPENSIPS_1_11_2_TLS_MESSAGETHREADMANAGER_H

#include <memory>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <unordered_map>
#include "logic.h"
#include "MessageThreadElement.hpp"
#include "MessageThreadMap.hpp"
#include "SenderJobQueue.hpp"

namespace bip = boost::interprocess;

// Main allocator, will be used after rebind call to allocate list elements.
typedef SipsSHMAllocator<MessageListElement> MainAllocator;

// Main mesage thread manager.
class MessageThreadManager {
private:
    // Global interprocess mutex, structure wide for fetching records.
    boost::interprocess::interprocess_mutex mutex;

    // Main allocator to be used in construction of a message list elements and message thread elements.
    MainAllocator alloc;

    // Linked list of all records with NONE status => cached in LRU fashion, if cache is too big
    // these records are recycled in LRU policy.
    boost::interprocess::interprocess_mutex thread_lru_mutex;
    MessageThreadLRU * thread_lru_head;
    MessageThreadLRU * thread_lru_tail;

    // Unordered hash map of the message thread elements.
    // Main structure for organizing message threads.
    MessageThreadMap threadMap;

    // Message thread pool.
    MessageThreadPool * thread_pool_head;
    MessageThreadPool * thread_pool_tail;

    // Sender job queue, HSM allocated.
    SenderJobQueue jobQueue;

public:

    // Take an allocator, rebind it to desired type in order to allocate memory in SHM.
    MessageThreadManager(const MainAllocator &alloc) :
            alloc{alloc},
            thread_lru_head{NULL},
            thread_lru_tail{NULL},
            jobQueue{std::allocator_traits<decltype(alloc)>::rebind_alloc<SenderQueueJob> allocator}
    {


    }

    /**
     * Dump messages. User was registered.
     */
    int dump(struct sip_msg *msg, char *owner, str uname, str host);

    /**
     * Cleaning task, periodically called by timer thread;
     */
    int clean();

};


#endif //OPENSIPS_1_11_2_TLS_MESSAGETHREADMANAGER_H
