//
// Created by Dusan Klinec on 20.09.15.
//

#include "MessageThreadMap.hpp"

template<class Alloc>
MessageThreadMapKey *MessageThreadMapKeyFactory<Alloc>::build(const Alloc &alloc) {
    Node_alloc cAlloc(alloc);

    // Allocate
    MessageThreadMapKey * tmp = cAlloc.allocate(1, NULL);
    cAlloc.construct (tmp, (MessageThreadElement()));

    return tmp;
}

template<class Alloc>
MessageThreadMapKey *MessageThreadMapKeyFactory<Alloc>::build(const ShmString &aReceiver, const ShmString &aSender, const Alloc &alloc) {
    Node_alloc cAlloc(alloc);

    // Allocate
    MessageThreadMapKey * tmp = cAlloc.allocate(1, NULL);
    cAlloc.construct (tmp, (MessageThreadMapKey()));

    return tmp;
}

template<class Alloc>
void MessageThreadMapKeyFactory<Alloc>::destroy(MessageThreadMapKey *elem, const Alloc &alloc) {
    if (elem == nullptr){
        return;
    }

    Node_alloc cAlloc(alloc);
    cAlloc.destroy(elem);
    cAlloc.deallocate(elem, 1);
}
