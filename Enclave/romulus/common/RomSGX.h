/**
 * Author: xxx xxxx
 * Based of RomulusLog code by: xxxx xxxx, xxxx xxxx, xxxx xxxx
 */

#ifndef ROMSGX_H
#define ROMSGX_H

#include <cstdint>  //for uint64_t
#include <stdlib.h> //for size_t
#include <atomic> //for atomic vars




#define CHUNK_SIZE_H 1024
#define MAX_SIZE 10 * 1024 * 1024 //100mb
#define MAGIC_ID_H 0x1337BAB2
#define CLPAD_H (128 / sizeof(uintptr_t)) //
#define NUM_ROOTS 100
#define MAX_THREADS 128 //single threading for now
//---------------------------------------------------------------

/**
 * The following macros are used to replace unsupported routines
 * and types in the malloc.c file
 */

#define SGX_MALLOC_FAILURE_ACTION abort_h()//TODO
#define SGX_MMAP abort_h() //TODO
#define SGX_MUNMAP(a, s) (0) //TODO i.e. do munmap via an ocall and return 
#define SGX_ABORT abort_h()
#define SGX_MAP_DEFAULT(s) (0)

#define SGX_PTHREAD_T
#define SGX_MAP_PROT //TODO
#define SGX_MAP_FLAGS

#define THREAD_LIB //TODO
#define SCHED_LIB //TODO
#define PTHREAD_LIB //TODO  
#define SYS_PARAM //TODO
//---------------------------------------------------------------


//--------------------------------------------------------------
extern uint8_t *base_addr_in;

#if defined( __sparc )
#define Pause() __asm__ __volatile__ ( "rd %ccr,%g0" )
#elif defined( __i386 ) || defined( __x86_64 )
#define Pause() __asm__ __volatile__ ( "pause" : : : )
#else
//#define Pause() std::this_thread::yield(); //does not work with sgx
#endif


//------------------------------------------------------------------------------------

/* Fxn prototypes */
//void destruct_out();
//LogChunk* get_chunk_out();

#if defined(__cplusplus)
extern "C" {
#endif
int __cxa_thread_atexit(void (*dtor)(void *), void *obj, void *dso_symbol);
void do_mmap();
void do_close();
void abort_h();
void sgx_printf(const char* fmt, ...);


#if defined(__cplusplus)
}
#endif

#endif /*!ROMSGX_H*/

