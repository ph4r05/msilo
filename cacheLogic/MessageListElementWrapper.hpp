//
// Created by Dusan Klinec on 22.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_MESSAGELISTELEMENTWRAPPER_HPP
#define OPENSIPS_1_11_2_TLS_MESSAGELISTELEMENTWRAPPER_HPP

#include "logic.hpp"
#include "MessageListElement.hpp"

template<class Alloc>
class MessageListElementWrapper {
public:
    typedef typename Alloc::template rebind<MessageListElement>::other Node_alloc;
    typedef typename Node_alloc::pointer Nodeptr;
    typedef typename Alloc::template rebind<MessageListElementWrapper>::other Wrapper_alloc;
    typedef typename Wrapper_alloc::pointer Wrapperptr;

    // Encapsulated element.
    MessageListElement elem;

    // Access operator goes directly to the element.
    MessageListElement * operator->() const {
        return &elem;
    }

    MessageListElementWrapper() { }
    MessageListElementWrapper(const MessageListElement &elem) : elem(elem) { }

    static MessageListElement * build(Alloc const& alloc);
    static void destroy(MessageListElement * elem, Alloc const& alloc);

    static MessageListElementWrapper<Alloc> * buildWrapper(Alloc const& alloc);
    static void destroyWrapper(MessageListElementWrapper<Alloc> *wrapper, const Alloc &alloc);

};


#endif //OPENSIPS_1_11_2_TLS_MESSAGELISTELEMENTWRAPPER_HPP
