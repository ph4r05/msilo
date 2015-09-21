//
// Created by Dusan Klinec on 20.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_MESSAGETHREADMAP_H
#define OPENSIPS_1_11_2_TLS_MESSAGETHREADMAP_H

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <unordered_map>
#include "logic.hpp"
#include "MessageThreadElement.hpp"

namespace bip = boost::interprocess;

// Message thread key
// (receiver, sender) string tuple.
typedef struct MessageThreadMapKey {
    ShmString receiver;
    ShmString sender;

    // Equals operator.
    bool operator==(const MessageThreadMapKey &other) const {
        return (receiver == other.receiver && sender == other.sender);
    }
} MessageThreadMapKey;

// Hash function for message thread key. hashcode().
struct MessageThreadMapKeyHasher {
    std::size_t operator()(const MessageThreadMapKey& k) const {
        std::size_t seed = 0;
        boost::hash_combine(seed,boost::hash_value(k.receiver));
        boost::hash_combine(seed,boost::hash_value(k.sender));
        return seed;
    }
};

// Hash function for message thread key. hashcode().
struct MessageThreadMapKeyEquals {
    bool operator()(const MessageThreadMapKey& k1, const MessageThreadMapKey& k2) const {
        return (k1.receiver == k2.receiver && k1.sender == k2.sender) ? 1 : 0;
    }

    bool operator==(const MessageThreadMapKey& k1, const MessageThreadMapKey& k2) const {
        return (k1.receiver == k2.receiver && k1sender == k2.sender);
    }
};

// Type for the map value
typedef MessageThreadElement * MessageThreadMapElement;

// (Key, value) pair for hash map.
typedef std::pair<const MessageThreadMapKey, MessageThreadMapElement> MessageThreadMapValueType;

// Allocator for pair value, allocating in shared memory.
typedef SipsSHMAllocator<MessageThreadMapValueType> MessageThreadMapAllocator;

// Message thread map.
typedef boost::unordered_map<   MessageThreadMapKey,            // Map key type.
                                MessageThreadMapElement,        // Map value type.
                                MessageThreadMapKeyHasher,      // Hashcode functor.
                                MessageThreadMapKeyEquals,      // Equals functor.
                                MessageThreadMapAllocator>      // Shared memory allocator.
                        MessageThreadMap;

#endif //OPENSIPS_1_11_2_TLS_MESSAGETHREADMAP_H
