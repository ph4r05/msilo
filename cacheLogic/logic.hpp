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
#include <cstring>

#include "SipsSHMAllocator.hpp"
#include "SipsHeapAllocator.hpp"
#include "../../../dprint.h"
#include "../../../str.h"
#include "../../../mem/mem.h"

// Namespace shortcut.
namespace bip = boost::interprocess;

// Message ID.
typedef long MessageIDType;

// Allocator for strings.
typedef SipsSHMAllocator<char> CharShmAllocator;

// String with allocator on shared memory
typedef bip::basic_string <char, std::char_traits<char>, CharShmAllocator> ShmString;

// Simple hasher for ShmString.
std::size_t hash_value(ShmString const& b);

#define PH_DBG( fmt, args...)                   		\
				do {                            		\
                    LM_DBG(fmt, ##args);        		\
				}while(0)

#define PH_INFO( fmt, args...) 							\
				do { 									\
                    LM_INFO(fmt, ##args); 				\
				}while(0)

#define PH_WARN( fmt, args...)  						\
				do { 									\
                    LM_WARN(fmt, ##args); 				\
				}while(0)

#define PH_ERR( fmt, args...)                            \
				do {                                     \
                    LM_ERR(fmt, ##args);                 \
				}while(0)

#endif //OPENSIPS_1_11_2_TLS_LOGIC_H
