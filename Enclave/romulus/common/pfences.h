/*
 * Created on Tue Feb 25 2020
 * Author:  2020 xxx xxxx, xxxx
 * Copyright Romulus by xxxx and xxxx
 */

#ifndef PFENCES_H
#define PFENCES_H

//#include <fcntl.h> header for file control
//#include <pthread.h> thread managment
//#include "stdatomic.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#define PWB_IS_CLFLUSHOPT

#define PWB_IS_CLFLUSH
#define CL_SIZE 64 // Size of one CPU cache line
#define FLUSH_ALIGN ((uintptr_t)64)
#define ENTRY_SIZE 4096 // One complete page at maximum


//#define PMEM_PATH "/dev/shm."


//------------------------------
#if defined(PWB_IS_CLFLUSH)

#define PWB(addr)                             \
  __asm__ volatile("clflush (%0)" ::"r"(addr) \
                   : "memory") // Broadwell only works with this.
#define PFENCE() \
  {              \
  } // No ordering fences needed for CLFLUSH (section 7.4.6 of Intel manual)
#define PSYNC()                                                             \
  {                                                                         \
  } // For durability it's not obvious, but CLFLUSH seems to be enough, and \
    // PMDK uses the same approach

#elif defined(PWB_IS_CLWB)
/* Use this for CPUs that support clwb, such as the SkyLake SP series (c5
 * compute intensive instances in AWS are an example of it) */
#define PWB(addr)               \
  __asm__ volatile(             \
      ".byte 0x66; xsaveopt %0" \
      : "+m"(*(volatile char *)(addr))) // clwb() only for Ice Lake onwards
#define PFENCE() __asm__ volatile("sfence" \
                                  :        \
                                  :        \
                                  : "memory")
#define PSYNC() __asm__ volatile("sfence" \
                                 :        \
                                 :        \
                                 : "memory")

#elif defined(PWB_IS_NOP)
/* pwbs are not needed for shared memory persistency (i.e. persistency across
 * process failure) */
#define PWB(addr) \
  {               \
  }
#define PFENCE() __asm__ volatile("sfence" \
                                  :        \
                                  :        \
                                  : "memory")
#define PSYNC() __asm__ volatile("sfence" \
                                 :        \
                                 :        \
                                 : "memory")

#elif defined(PWB_IS_CLFLUSHOPT)
/* Use this for CPUs that support clflushopt, which is most recent x86 */
#define PWB(addr)                           \
  __asm__ volatile(".byte 0x66; clflush %0" \
                   : "+m"(*(volatile char *)(addr))) // clflushopt (Kaby Lake)
#define PFENCE() __asm__ volatile("sfence" \
                                  :        \
                                  :        \
                                  : "memory")
#define PSYNC() __asm__ volatile("sfence" \
                                 :        \
                                 :        \
                                 : "memory")
#else
#error \
    "You must define what PWB is. Choose PWB_IS_CLFLUSHOPT if you don't know what your CPU is capable of"

#endif

// Flush each cache line in a range
// TODO: fix cache alignment
inline static void flushFromTo(void* from, void* to) noexcept {
    const int cache_line_size = 64;
    uint8_t* ptr = (uint8_t*)from;
    for (; ptr < (uint8_t*)to; ptr += cache_line_size) PWB(ptr);
}
//-----------------------------

#endif /* PFENCES_H */
