

#ifndef OPENSIPS_1_11_2_TLS_SIPSSHMALLOCATOR_H
#define OPENSIPS_1_11_2_TLS_SIPSSHMALLOCATOR_H

#include <limits>
#include <iostream>
#include "../../../mem/mem.h"
#include "../../../mem/shm_mem.h"
#include "SipsAllocator.hpp"

/**
 * Allocator uses OpenSIPS facilities to allocate chunks on shared memory (SHM).
 */
template <class T>
class SipsSHMAllocator {
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
        typedef SipsSHMAllocator<U> other;
    };

    // return address of values
    pointer address (reference value) const {
        return &value;
    }

    const_pointer address (const_reference value) const {
        return &value;
    }

    /* constructors and destructor
     * - nothing to do because the allocator has no state
     */
    SipsSHMAllocator() throw() {
    }

    SipsSHMAllocator(const SipsSHMAllocator&) throw() {
    }

    template <class U>
    SipsSHMAllocator (const SipsSHMAllocator<U>&) throw() {
    }

    ~SipsSHMAllocator() throw() {
    }

    // return maximum number of elements that can be allocated
    size_type max_size () const throw() {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    // allocate but don't initialize num elements of type T
    pointer allocate (size_type num, const void* = 0) {
        // print message and allocate memory with global new
        std::cerr << "allocate " << num << " element(s)" << " of size " << sizeof(T) << std::endl;

        // Use shared memory interface provided by OpenSIPS.
        pointer ret = (pointer)shm_malloc(num*sizeof(T));
        if (ret == NULL){
            throw std::bad_alloc();
        }

        // Old version: heap allocation.
        //pointer ret = (pointer)(::operator new(num*sizeof(T)));

        std::cerr << " allocated at: " << (void*)ret << std::endl;
        return ret;
    }

    // initialize elements of allocated storage p with value value
    void construct (pointer p, const T& value) {
        // initialize memory with placement new
        new((void*)p)T(value);
    }

    // destroy elements of initialized storage p
    void destroy (pointer p) {
        // destroy objects by calling their destructor
        p->~T();
    }

    // deallocate storage p of deleted elements
    void deallocate (pointer p, size_type num) {
        // print message and deallocate memory with global delete
        std::cerr << "deallocate " << num << " element(s)" << " of size " << sizeof(T) << " at: " << (void*)p << std::endl;

        // Use shared memory interface provided by OpenSIPS.
        shm_free((void*)p);

        // Old version: heap deallocation, delete operator.
        //::operator delete((void*)p);
    }

    template <typename U>
    static ph4::unique_ptr<U> unique_ptr(U * p, SipsSHMAllocator<U> * alloc = nullptr, size_type num=1, bool destroy=true){
        return ph4::unique_ptr<U>(p, [&](U* f) {
            if (destroy) {
                alloc->destroy(f);
            }

            alloc->deallocate(f, num);
        });
    }

    template <typename U>
    static ph4::shared_ptr<U> shared_ptr(U * p, SipsSHMAllocator<U> * alloc = nullptr, size_type num=1, bool destroy=true){
        return ph4::shared_ptr<U>(p, [&](U* f) {
            if (destroy) {
                alloc->destroy(f);
            }

            alloc->deallocate(f, num);
        }, alloc);
    }

    ph4::unique_ptr<T> unique_ptr(T * p, size_type num=1, bool destroy=true){
        return SipsSHMAllocator<T>::unique_ptr(p, this, num, destroy);
    }

    ph4::shared_ptr<T> shared_ptr(T * p, size_type num=1, bool destroy=true){
        return SipsSHMAllocator<T>::shared_ptr(p, this, num, destroy);
    }

};

// return that all specializations of this allocator are interchangeable
template <class T1, class T2>
bool operator== (const SipsSHMAllocator<T1>&,
                 const SipsSHMAllocator<T2>&) throw() {
    return true;
}
template <class T1, class T2>
bool operator!= (const SipsSHMAllocator<T1>&,
                 const SipsSHMAllocator<T2>&) throw() {
    return false;
}


#endif //OPENSIPS_1_11_2_TLS_SIPSSHMALLOCATOR_H