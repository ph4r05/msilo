//
// Created by Dusan Klinec on 22.09.15.
//

#include "MessageListElementWrapper.hpp"

template<class Alloc>
MessageListElement *MessageListElementWrapper<Alloc>::build(const Alloc &alloc) {
    Node_alloc cAlloc(alloc);

    // Allocate
    MessageListElement * elem = cAlloc.allocate(1, NULL);
    if (elem == nullptr){
        throw std::bad_alloc();
    }

    cAlloc.construct (elem, (MessageListElement()));
    return elem;
}

template<class Alloc>
void MessageListElementWrapper<Alloc>::destroy(MessageListElement *elem, const Alloc &alloc) {
    if (elem == nullptr){
        return;
    }

    Node_alloc cAlloc(alloc);
    cAlloc.destroy(elem);
    cAlloc.deallocate(elem, 1);
}

template<class Alloc>
MessageListElementWrapper<Alloc> *MessageListElementWrapper<Alloc>::buildWrapper(const Alloc &alloc) {
    Wrapper_alloc cAlloc(alloc);

    // Allocate
    MessageListElementWrapper<Alloc> * elem = cAlloc.allocate(1, NULL);
    if (elem == nullptr){
        throw std::bad_alloc();
    }

    cAlloc.construct (elem, (MessageListElementWrapper()));
    return elem;
}

template<class Alloc>
void MessageListElementWrapper<Alloc>::destroyWrapper(MessageListElementWrapper<Alloc> *wrapper, const Alloc &alloc) {
    if (wrapper == nullptr){
        return;
    }

    Wrapper_alloc cAlloc(alloc);
    cAlloc.destroy(wrapper);
    cAlloc.deallocate(wrapper, 1);
}
