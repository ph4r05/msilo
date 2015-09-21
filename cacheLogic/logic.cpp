//
// Created by Dusan Klinec on 21.09.15.
//
#include "logic.hpp"

std::size_t hash_value(ShmString const& b) {
    return boost::hash_range(b.begin(), b.end());
}
