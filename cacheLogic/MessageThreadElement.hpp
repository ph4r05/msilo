//
// Created by Dusan Klinec on 18.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_MESSAGETHREADELEMENT_H
#define OPENSIPS_1_11_2_TLS_MESSAGETHREADELEMENT_H

#include <iostream>
#include <string>
#include <functional>                  //std::equal_to
#include <limits>                      //std::numeric_limits
#include <boost/unordered_map.hpp>     //boost::unordered_map
#include <boost/unordered_set.hpp>     //boost::unordered_set
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include "logic.hpp"
#include "MessageListElement.hpp"

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
 * Message thread pool.
 * Linked list of all unused records.
 */
class MessageThreadPool {
public:
    MessageThreadElement * tElem;

    MessageThreadPool * next;
    MessageThreadPool * prev;

    MessageThreadPool() :
            tElem{NULL},
            next{NULL},
            prev{NULL}
    { }
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
    MessageListElement * msg_cache_head;
    MessageListElement * msg_cache_tail;

    // Pointer to pool entry.
    MessageThreadPool * cache_entry;

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


    ShmString getReceiver() const {
        return receiver;
    }

    void setReceiver(ShmString receiver) {
        MessageThreadElement::receiver = receiver;
    }

    ShmString getSender() const {
        return sender;
    }

    void setSender(ShmString sender) {
        MessageThreadElement::sender = sender;
    }

    const MessageThreadState &getSendState() const {
        return sendState;
    }

    void setSendState(const MessageThreadState &sendState) {
        MessageThreadElement::sendState = sendState;
    }

    boost::interprocess::interprocess_mutex &getMutex() {
        return mutex;
    }

    boost::interprocess::interprocess_mutex &getMsg_cache_lock() {
        return msg_cache_lock;
    }

    MessageIDSet& getMsg_cache_id_set() {
        return msg_cache_id_set;
    }

    unsigned long getMsg_cache_size() const {
        return msg_cache_size;
    }

    void setMsg_cache_size(unsigned long msg_cache_size) {
        MessageThreadElement::msg_cache_size = msg_cache_size;
    }

    MessageIDType getMsg_cache_max_mid() const {
        return msg_cache_max_mid;
    }

    void setMsg_cache_max_mid(MessageIDType msg_cache_max_mid) {
        MessageThreadElement::msg_cache_max_mid = msg_cache_max_mid;
    }

    MessageIDType getMsg_cache_min_mid() const {
        return msg_cache_min_mid;
    }

    void setMsg_cache_min_mid(MessageIDType msg_cache_min_mid) {
        MessageThreadElement::msg_cache_min_mid = msg_cache_min_mid;
    }

    MessageListElement *getMsg_cache_head() const {
        return msg_cache_head;
    }

    void setMsg_cache_head(MessageListElement *msg_cache_head) {
        MessageThreadElement::msg_cache_head = msg_cache_head;
    }

    MessageListElement *getMsg_cache_tail() const {
        return msg_cache_tail;
    }

    void setMsg_cache_tail(MessageListElement *msg_cache_tail) {
        MessageThreadElement::msg_cache_tail = msg_cache_tail;
    }

    MessageThreadPool *getCache_entry() const {
        return cache_entry;
    }

    void setCache_entry(MessageThreadPool *cache_entry) {
        MessageThreadElement::cache_entry = cache_entry;
    }
};

#endif //OPENSIPS_1_11_2_TLS_MESSAGETHREADELEMENT_H
