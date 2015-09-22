//
// Created by Dusan Klinec on 18.09.15.
//

#include "MessageThreadElement.hpp"

template<typename Alloc>
MessageThreadElement *MessageThreadElementWrapper<Alloc>::build(const Alloc &alloc) {
    Node_alloc cAlloc(alloc);

    // Allocate
    MessageThreadElement * elem = cAlloc.allocate(1, NULL);
    cAlloc.construct (elem, (MessageThreadElement()));

    return elem;
}

template<typename Alloc>
MessageThreadElement *MessageThreadElementWrapper<Alloc>::build(const ShmString &aReceiver, const ShmString &aSender, const Alloc &alloc) {
    Node_alloc cAlloc(alloc);

    // Allocate
    MessageThreadElement * elem = cAlloc.allocate(1, NULL);
    cAlloc.construct (elem, (MessageThreadElement()));
    elem->setSender(aSender);
    elem->setReceiver(aReceiver);

    return elem;
}

template<typename Alloc>
void MessageThreadElementWrapper<Alloc>::destroy(MessageThreadElement *elem, const Alloc &alloc) {
    if (elem == nullptr){
        return;
    }

    Node_alloc cAlloc(alloc);
    cAlloc.destroy(elem);
    cAlloc.deallocate(elem, 1);
}

template<typename Alloc>
MessageThreadElementWrapper<Alloc> *MessageThreadElementWrapper<Alloc>::buildWrapper(const Alloc &alloc) {
    Wrapper_alloc cAlloc(alloc);

    // Allocate
    MessageThreadElementWrapper<Alloc> * elem = cAlloc.allocate(1, NULL);
    cAlloc.construct (elem, (MessageThreadElementWrapper()));

    return elem;
}

template<typename Alloc>
MessageThreadElementWrapper<Alloc> *MessageThreadElementWrapper<Alloc>::buildWrapper(const ShmString &aReceiver, const ShmString &aSender, const Alloc &alloc) {
    Wrapper_alloc cAlloc(alloc);

    // Allocate
    MessageThreadElementWrapper<Alloc> * elem = cAlloc.allocate(1, NULL);
    cAlloc.construct (elem, (MessageThreadElementWrapper()));
    elem->elem.setReceiver(aReceiver);
    elem->elem.setSender(aSender);

    return elem;
}

template<typename Alloc>
void MessageThreadElementWrapper<Alloc>::destroyWrapper(MessageThreadElementWrapper<Alloc> *elem, const Alloc &alloc) {
    if (elem == nullptr){
        return;
    }

    Wrapper_alloc cAlloc(alloc);
    cAlloc.destroy(elem);
    cAlloc.deallocate(elem, 1);
}
