//
// Created by Dusan Klinec on 22.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_SIPSALLOCATOR_H
#define OPENSIPS_1_11_2_TLS_SIPSALLOCATOR_H

#include <limits>
#include <memory>
#include <iostream>

// Smart pointers with custom deleter function, that will be provided by custom allocator.
namespace ph4 {
    template<class U>
    using deleter = std::function<void(U *)>;

    template<class U>
    using unique_ptr = std::unique_ptr<U, deleter<U> >;

    template<class U>
    using shared_ptr = std::shared_ptr<U>;
};

template <class T>
class SipsAllocator {
public:
    // type definitions
    typedef T        value_type;
    typedef T*       pointer;
    typedef const T* const_pointer;
    typedef T&       reference;
    typedef const T& const_reference;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    // rebind allocator to type U
    template <class U>
    struct rebind {
        typedef SipsAllocator<U> other;
    };

    /* constructors and destructor
     * - nothing to do because the allocator has no state
     */
    SipsAllocator() throw() {
    }

    SipsAllocator(const SipsAllocator&) throw() {
    }

    template <class U>
    SipsAllocator (const SipsAllocator<U>&) throw() {
    }

    ~SipsAllocator() throw() {
    }

    // return address of values
    virtual pointer address (reference value) const =0;

    virtual const_pointer address (const_reference value) const =0;

    // return maximum number of elements that can be allocated
    virtual size_type max_size () const throw() =0;

    // allocate but don't initialize num elements of type T
    virtual pointer allocate (size_type num, const void* = 0) =0;

    // initialize elements of allocated storage p with value value
    virtual void construct (pointer p, const T& value) =0;

    // destroy elements of initialized storage p
    virtual void destroy (pointer p) =0;

    // deallocate storage p of deleted elements
    virtual void deallocate (pointer p, size_type num) =0;
};

// return that all specializations of this allocator are interchangeable
template <class T1, class T2>
bool operator== (const SipsAllocator<T1>&,
                 const SipsAllocator<T2>&) throw() {
    return true;
}
template <class T1, class T2>
bool operator!= (const SipsAllocator<T1>&,
                 const SipsAllocator<T2>&) throw() {
    return false;
}


#endif //OPENSIPS_1_11_2_TLS_SIPSALLOCATOR_H
