/**
 * Author: xxx xxxx
 * Based of RomulusLog code by: xxxx xxxx, xxxx xxxx, xxxx xxxx
 */

// romulus types

#ifndef ROM_TYPES_H
#define ROM_TYPES_H

#include <cstdint>
#include <atomic>

/* Log states */
#define STATE_0 0 /* IDLE */
#define STATE_1 1 /* MUTATING */
#define STATE_2 2 /* COPYING */

//this file will be included in the .edl file so must be pure C..
/* Typedefs */
typedef void *mspace;
typedef struct PersistentHeader
{
    uint64_t id{0};                  // Validates intialization
    std::atomic<int> state{STATE_0}; // Current state of consistency
    void **objects{};                // Objects directory
    mspace ms{};                     // Pointer to allocator's metadata
                                     // uint64_t used_size{0}; // It has to be the last, to calculate the used_size

    // Original romulus code tests for ESLOCO; They never use ESLOCO so I ignore it for now..
    uint64_t used_size{0};

} PersistentHeader;

#endif /* ROM_TYPES_H */