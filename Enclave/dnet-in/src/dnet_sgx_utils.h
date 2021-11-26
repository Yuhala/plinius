/**
 * Author: xxx xxxx
 * SGX utils for darknet port
 */

//At the time of this porting Intel SGX does not support CUDA
//Therefore all GPU related functionality has been stripped off
//Peterson Yuhala Feb 2020

#ifndef DNET_SGX_UTILS_H
#define DNET_SGX_UTILS_H



#include "Enclave_t.h"
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define BACKUP_PATH ""
#define RAND_MAX 2147483647
#define INT_MAX RAND_MAX
#define MAX_CHAR 4096

//#define DNET_SGX_DEBUG //uncomment if you need more verbose output during training/debugging
//#define SGX_OMP //OMP not used yet 


#define PRINT_BLOCK()                    \
    {                                    \
        char buf[BUFSIZ] = {'\0'};       \
        va_list ap;                      \
        va_start(ap, fmt);               \
        vsnprintf(buf, BUFSIZ, fmt, ap); \
        va_end(ap);                      \
        ocall_print_string(buf);         \
    }

#define DEBUG_PRINT()                                 \
    {                                                 \
        printf("\n"); \
    }


//ensure C linkage..
#if defined(__cplusplus)
extern "C"
{
#endif
    void fread(void *ptr, size_t size, size_t nmemb, int fp);
    void fwrite(void *ptr, size_t size, size_t nmemb, int fp);
    void printf(const char *fmt, ...);
    //redefining srand and rand
    void srand(unsigned int seed);
    int rand(void);

#if defined(__cplusplus)
}
#endif

#endif //DNET_SGX_UTILS_H
