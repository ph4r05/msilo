//
// Created by Dusan Klinec on 18.09.15.
//

#ifndef OPENSIPS_1_11_2_TLS_LOGIC_H
#define OPENSIPS_1_11_2_TLS_LOGIC_H

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#include <memory>
#include <thread>
#include <string>

#include "SipsSHMAllocator.hpp"
#include "SipsHeapAllocator.hpp"
#include "../../../dprint.h"

// Namespace shortcut.
namespace bip = boost::interprocess;

// Message ID.
typedef long MessageIDType;

// Allocator for strings.
typedef SipsSHMAllocator<char> CharShmAllocator;

// String with allocator on shared memory
typedef bip::basic_string <char, std::char_traits<char>, CharShmAllocator> ShmString;

#endif //OPENSIPS_1_11_2_TLS_LOGIC_H
