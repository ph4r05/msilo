//
// Created by Dusan Klinec on 18.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_MESSAGE_CACHE2_H
#define OPENSIPS_1_11_2_TLS_MESSAGE_CACHE2_H

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <iostream>
#include <unordered_map>
using namespace boost::interprocess;

// Message send record state.
typedef enum MessageSendCtlState {
    MSG_REC_STATE_NONE = 0, // Not used, can be cached out if desired.
    MSG_REC_STATE_QUEUED,
    MSG_REC_STATE_PROCESSING,
    MSG_REC_STATE_WAITING
} MessageSendCtlState;

/**
 * Structure for one per user msilo management.
 * "receiver:sender" -> _msilo_ctl_el
 */
class MessageSendCtl {
private:
    std::string receiver;
    std::string sender;

    // If the current record is being processed (waiting for tx callback).
    // if this record is being queued for processing.
    MessageSendCtlState sendState;

    // receiver:sender record-wide lock, locking access to the whole user record.
    boost::interprocess::interprocess_mutex mutex;

    // Preloaded messages for this receiver:sender key from database.
    // Linked list of messages.
    boost::interprocess::interprocess_mutex msg_cache_lock;
    unsigned long msg_cache_size;
    // Maximal message ID in the message queue (ordering is strictly enforced, used in insert checks)
    long msg_cache_max_mid;
    // Minimal message ID in the message queue.
    long msg_cache_min_mid;
    // Linked list.
    MessageElement * msg_cache_head;
    MessageElement * msg_cache_tail;
};

class MessageCache {
private:
    // Global interprocess mutex, structure wide for fetching records.
    boost::interprocess::interprocess_mutex mutex;
};

using namespace std;

// List nodeclass
class Node {
public:
    int data;
    Node* next;
    Node* prev;
};

// Linked list class
class DLList {
public:
    DLList() { head = NULL; tail = NULL; count = 0;}
    ~DLList() {}
    Node* addNode(int val);
    void print();
    void removeTail();
    void moveToHead(Node* node);
    int size() { return count; }

private:
    Node* head;
    Node* tail;
    int count;
};

// Function to add a node to the list
Node* DLList::addNode(int val)
{
    Node* temp = new Node();
    temp->data = val;
    temp->next = NULL;
    temp->prev = NULL;

    if ( tail == NULL ) {
        tail = temp;
        head = temp;
    }
    else {
        head->prev = temp;
        temp->next = head;
        head = temp;
    }
    count++;
    return temp;
}

void DLList::moveToHead(Node* node)
{
    if (head == node)
        return;
    node->prev->next = node->next;

    if (node->next != NULL){
        node->next->prev = node->prev;
    } else {
        tail = node->prev;
    }
    node->next = head;
    node->prev = NULL;
    head->prev = node;
    head = node;
}

void DLList::removeTail()
{
    count--;
    if (head == tail) {
        delete head;
        head = NULL;
        tail = NULL;
    } else {
        Node* del = tail;
        tail = del->prev;
        tail->next = NULL;
        delete del;
    }
}

void DLList::print()
{
    Node* temp = head;
    int ctr = 0;
    while ( (temp != NULL) && (ctr++ != 25)  ) {
        cout << temp->data << " ";
        temp = temp->next;
    }
    cout << endl;
}

class LRUCache {
public:
    LRUCache(int aCacheSize);
    void fetchPage(int pageNumber);

private:
    int cacheSize;
    DLList lruOrder;
    unordered_map<int,Node*> directAccess;

    // Whole cache lock. read write process wide mutex.

};

LRUCache::LRUCache(int aCacheSize):cacheSize(aCacheSize)
{
}

void LRUCache::fetchPage(int pageNumber)
{
    unordered_map<int,Node*>::const_iterator it = directAccess.find(pageNumber);

    if (it != directAccess.end()) {
        lruOrder.moveToHead( (Node*)it->second);
    } else {
        if (lruOrder.size() == cacheSize-1)
            lruOrder.removeTail();
        Node* node = lruOrder.addNode(pageNumber);
        directAccess.insert(make_pair<int,Node*>(pageNumber,node));
    }
    lruOrder.print();
}


#endif //OPENSIPS_1_11_2_TLS_MESSAGE_CACHE2_H
