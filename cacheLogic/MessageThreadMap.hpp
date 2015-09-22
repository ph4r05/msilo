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
class MessageThreadMapKey {
public:
    ShmString receiver;
    ShmString sender;

    // Equals operator.
    bool operator==(const MessageThreadMapKey &other) const {
        return (receiver == other.receiver && sender == other.sender);
    }

    MessageThreadMapKey() { }
    MessageThreadMapKey(const ShmString &receiver) : receiver(receiver) { }
    MessageThreadMapKey(const ShmString &receiver, const ShmString &sender) : receiver(receiver), sender(sender) { }
    MessageThreadMapKey(const MessageThreadMapKey & src) : receiver(src.receiver), sender(src.sender) { }
    MessageThreadMapKey(MessageThreadMapKey && src) noexcept : receiver(std::move(src.receiver)), sender(std::move(src.sender)){ }
};

// Factory, allocator bound.
template<class Alloc>
class MessageThreadMapKeyFactory {
public:
    typedef typename Alloc::template rebind<MessageThreadMapKey>::other Node_alloc;
    typedef typename Node_alloc::pointer Nodeptr;
    typedef typename Alloc::template rebind<MessageThreadMapKeyFactory>::other Wrapper_alloc;
    typedef typename Wrapper_alloc::pointer Wrapperptr;

    // Encapsulated element.
    MessageThreadMapKey elem;

    // Access operator goes directly to the element.
    MessageThreadMapKey * operator->() const {
        return &elem;
    }

    MessageThreadMapKeyFactory() { }
    MessageThreadMapKeyFactory(const MessageThreadMapKey &elem) : elem(elem) { }

    static MessageThreadMapKey * build(Alloc const& alloc);
    static MessageThreadMapKey * build(const ShmString &aReceiver, const ShmString &aSender, const Alloc &alloc);
    static void destroy(MessageThreadMapKey * elem, Alloc const& alloc);
};

// Hash function for message thread key. hashcode().
struct MessageThreadMapKeyHasher {
    std::size_t operator()(const MessageThreadMapKey& k) const {
        std::size_t seed = 0;
        boost::hash_combine(seed, boost::hash_range(k.receiver.begin(), k.receiver.end()));
        boost::hash_combine(seed, boost::hash_range(k.sender.begin(), k.sender.end()));
        return seed;
    }
};

// Hash function for message thread key. hashcode().
struct MessageThreadMapKeyEquals {
    bool operator()(const MessageThreadMapKey& k1, const MessageThreadMapKey& k2) const {
        return (k1.receiver == k2.receiver && k1.sender == k2.sender);
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
