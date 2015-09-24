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
#include "logic.hpp"
#include "MessageListElement.hpp"
#include "MessageThreadElement.hpp"
#include "MessageThreadMap.hpp"
#include "MessageThreadSender.hpp"
#include "SenderJobQueue.hpp"
#include "SipsHeapAllocator.hpp"
#include "SipsSHMAllocator.hpp"
#include "ApiHelper.hpp"
#include "common.h"
#include "../ms_msg_list.h"

// Main allocator, will be used after rebind call to allocate list elements.
typedef SipsSHMAllocator<MessageListElement> MainAllocator;

// Forward declaration for sender.
class MessageThreadSender;

// Main mesage thread manager.
class MessageThreadManager {
private:
    // Global interprocess mutex, structure wide for fetching records.
    boost::interprocess::interprocess_mutex mutex;

    // Main allocator to be used in construction of a message list elements and message thread elements.
    MainAllocator alloc;

    // Linked list of all records with NONE status => cached in LRU fashion, if cache is too big
    // these records are recycled in LRU policy.
    // Linked list is intended to server as LRU record. Most recent is in the head.
    boost::interprocess::interprocess_mutex thread_lru_mutex;
    MessageThreadPool * thread_lru_head;
    MessageThreadPool * thread_lru_tail;

    // Unordered hash map of the message thread elements.
    // Main structure for organizing message threads.
    // (receiver,sender) -> * MessageThreadElement
    MessageThreadMap threadMap;

    // Precessed msg list - used for dumping the messages.
    // Holds list of all processed messages from DB, not to process them again.
    msg_list ml;

    // MessageThread pool.
    // Intended to cache old unused message thread objects for quick reuse without allocation.
    // Not used at the moment.
    MessageThreadPool * thread_pool_head;
    MessageThreadPool * thread_pool_tail;

    // Sender job queue, HSM allocated.
    // Executor (sender) processes this jobQueue. Queue has to be locked when processing.
    SenderJobQueue jobQueue;

    // Hooks, API wrapper.
    thread_mgr_api api;
    ApiHelper apiHelper;

    /**
     * Locks the main mutex, loads particular thread object
     */
    MessageThreadElement * getThreadAndLock(const MessageThreadMapKey & key);

    /**
     * Sends message via TM interface.
     */
    int sendMessage(str * to, str * from, str * headers, str * body, thread_tsx_callback * cb);

public:
    // Take an allocator, rebind it to desired type in order to allocate memory in SHM.
    MessageThreadManager(const MainAllocator &alloc) :
            alloc{alloc},
            thread_lru_head{NULL},
            thread_lru_tail{NULL}, //SenderQueueJob
            ml{NULL},
            jobQueue{MainAllocator::rebind<SenderQueueJob>::other(alloc)}
    {

    }

    MessageThreadManager() : MessageThreadManager((MainAllocator())) { }

    // Copy constructor.
    MessageThreadManager(const MessageThreadManager& src);

    /**
     * Dump messages. User was registered.
     */
    int dump(MessageThreadSender * sender, struct sip_msg *msg, char *owner, str uname, str host);

    /**
     * Cleaning task, periodically called by timer thread;
     */
    int clean(MessageThreadSender * sender);

    /**
     * Tsx callback.
     */
    int tsx_callback(MessageThreadSender * sender, int code, MessageThreadElement * mapElem, long mid);

    /**
     * Callback from sender, send(receiver).
     */
    void send1(SenderQueueJob * job, MessageThreadSender * sender);

    /**
     * Callback from sender, send(receiver, sender).
     */
    void send2(SenderQueueJob * job, MessageThreadSender * sender);

    const SenderJobQueue &getJobQueue() const {
        return jobQueue;
    }

    SenderJobQueue * getJobQueuePtr() {
        return &jobQueue;
    }

    const MainAllocator &getAlloc() const {
        return alloc;
    }

    void updateApi(thread_mgr_api const * xapi){
        this->api.db_con = xapi->db_con;
        this->api.msilo_dbf = xapi->msilo_dbf;
        this->api.tmb = xapi->tmb;
        this->api.load_messages = xapi->load_messages;
        this->apiHelper.setApi(&api);
    }
};


#endif //OPENSIPS_1_11_2_TLS_MESSAGETHREADMANAGER_H
