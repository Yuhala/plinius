/**
 * Author: xxx xxxx
 * Based of RomulusLog code by: xxxx xxxx, xxxx xxxx, xxxx xxxx
 */

#ifndef TYPES_ROM_H
#define TYPES_ROM_H
#include <cstdint>
#include <atomic>
#include <functional>
//--------------------------------------------------------------
#define CHUNK_SIZE_H 1024
#define MAX_SIZE 400 * 1024 * 1024 //512mb
#define MAGIC_ID_H 0x1337BAB2
#define CLPAD_H (128 / sizeof(uintptr_t)) //
#define NUM_ROOTS 100
#define MAX_THREADS 1 //single threading for now

/* Log states */
#define STATE_0 0 /* IDLE */
#define STATE_1 1 /* MUTATING */
#define STATE_2 2 /* COPYING */

/* Typedefs */
typedef void *mspace;

// One instance of this is at the start of base_addr, in persistent memory

typedef struct LogEntry
{
    size_t offset;   // Pointer offset in bytes, relative to main_addr
    uint64_t length; // Range length of data at pointer offset

} LogEntry;

typedef struct LogChunk
{
    LogEntry entries[CHUNK_SIZE_H];
    uint64_t num_entries{0};
    LogChunk *next{nullptr};

} LogChunk;

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

#endif /*!ROMSGX_H*/