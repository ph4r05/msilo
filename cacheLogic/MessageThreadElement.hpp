//
// Created by Dusan Klinec on 18.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_MESSAGETHREADELEMENT_H
#define OPENSIPS_1_11_2_TLS_MESSAGETHREADELEMENT_H

#include <iostream>
#include <string>
#include <functional>                  //std::equal_to
#include <unordered_set>
#include <limits>                      // std::numeric_limits
#include <boost/unordered_map.hpp>     //boost::unordered_map
#include <boost/unordered_set.hpp>     //boost::unordered_set
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include "MessageListElement.h"
#include "logic.h"

// Message send record state.
typedef enum MessageThreadState {
    MSG_REC_STATE_NONE = 0, // Not used, can be cached out if desired.
    MSG_REC_STATE_QUEUED,
    MSG_REC_STATE_PROCESSING,
    MSG_REC_STATE_WAITING
} MessageThreadState;

// Set on shared memory for message IDs.
namespace bip = boost::interprocess;

// Allocator for pair value, allocating in shared memory.
typedef SipsSHMAllocator<MessageIDType> MessageIDAllocator;

// Message ID set.
typedef boost::unordered_set<
                    MessageIDType,                  // Map key type.
                    boost::hash<MessageIDType>,     // hashcode()
                    std::equal_to<MessageIDType>,   // equals
                    MessageIDAllocator>             // Shared memory allocator.
        MessageIDSet;

// Forward declaration.
class MessageThreadElement;

/**
 * Linked list of all unused records.
 */
class MessageThreadCache {
public:
    MessageThreadElement * tElem;

    MessageThreadCache * next;
    MessageThreadCache * prev;

};

/**
 * Structure for one per user msilo management.
 * "receiver:sender" -> MessageThreadElement
 */
class MessageThreadElement {
private:
    ShmString receiver;
    ShmString sender;

    // If the current record is being processed (waiting for tx callback).
    // if this record is being queued for processing.
    MessageThreadState sendState;

    // receiver:sender record-wide lock, locking access to the whole user record.
    boost::interprocess::interprocess_mutex mutex;

    // Preloaded messages for this receiver:sender key from database.
    // Linked list of messages.
    boost::interprocess::interprocess_mutex msg_cache_lock;

    // Set of all message IDs stored in the linked list.
    MessageIDSet msg_cache_id_set;

    // Size of the linked list.
    unsigned long msg_cache_size;

    // Maximal message ID in the message queue (ordering is strictly enforced, used in insert checks)
    MessageIDType msg_cache_max_mid;

    // Minimal message ID in the message queue.
    MessageIDType msg_cache_min_mid;

    // Linked list of cached messages in this thread to send.
    MessageElement * msg_cache_head;
    MessageElement * msg_cache_tail;

    // Pointer to cache entry. LRU cache entry listing.
    MessageThreadCache * cache_entry;

    // Constructor with allocator.
public:
    template <typename Alloc2>
    MessageThreadElement(Alloc2 const& alloc = {})
            : msg_cache_head{NULL},
              msg_cache_tail{NULL},
              msg_cache_min_mid{std::numeric_limits<MessageIDType>::max()},
              msg_cache_max_mid{std::numeric_limits<MessageIDType>::min()},
              msg_cache_size{0},
              msg_cache_id_set{alloc},
              sendState{MSG_REC_STATE_NONE},
              cache_entry{NULL}
    {

    }

};

#endif //OPENSIPS_1_11_2_TLS_MESSAGETHREADELEMENT_H
