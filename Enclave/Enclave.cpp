/**
 * Author: Peterson Yuhala
 * sgx-dnet-romulus/plinius: ML implem of our mirroring mechanism
 * using sgx-dnet and sgx-romulus
 */

#include "Enclave.h"
#include "sgx_trts.h"
#include "sgx_thread.h" //for thread manipulation
#include "Enclave_t.h"  /* print_string */
#include <stdarg.h>
#include <stdio.h>
#include "checks.h"
//#include <thread>
#include "romulus/datastructures/TMStack.hpp"

//#define PLINIUS_DEBUG

//romAttrib *romAttrib_out; //contains marshalled properties of outside romulus object
uint8_t *base_addr_in = NULL; //this will receive the value of the

int __cxa_thread_atexit(void (*dtor)(void *), void *obj, void *dso_symbol) {}

void printf(const char *fmt, ...)
{
    PRINT_BLOCK();
}

void sgx_printf(const char *fmt, ...)
{
#ifdef PLINIUS_DEBUG
    PRINT_BLOCK();
#endif
}

void fread(void *ptr, size_t size, size_t nmemb, int fp)
{
#ifdef ENABLE_CRYPTO //copy and decrypt temp into ptr
    char temp[BUFLEN];
    //sizeof(char) always 1 byte but for clarity of code we write sizeof(char) :-)
    ocall_fread(temp, sizeof(char), nmemb * size + ADD_ENC_DATA_SIZE);
    decryptData(temp, nmemb * size + ADD_ENC_DATA_SIZE, ptr, nmemb * size, GCM);

#else
    ocall_fread(ptr, size, nmemb);
#endif
}

void fwrite(void *ptr, size_t size, size_t nmemb, int fp)
{
#ifdef ENABLE_CRYPTO //encrypt ptr into temp and temp to file
    char temp[BUFLEN];
    encryptData(ptr, nmemb * size, temp, nmemb * size + ADD_ENC_DATA_SIZE, GCM);
    ocall_fwrite(temp, sizeof(char), nmemb * size + ADD_ENC_DATA_SIZE);

#else
    ocall_fwrite(ptr, size, nmemb);
#endif
}
void abort_h()
{
    sgx_printf("Aborting from sgx romulus..\n");
    abort();
}

void do_close()
{
    my_ocall_close();
}

void empty_ecall()
{
    sgx_printf("Inside empty ecall\n");
}

void ecall_init(void *per_out, uint8_t *base_addr_out)
{

    CHECK_REF_POINTER(per_out, sizeof(PersistentHeader));
    CHECK_REF_POINTER(base_addr_out, sizeof(uint8_t));
    /**
     * load fence after pointer checks ensures the checks are done 
     * before any assignment. 
     */
    printf("here\n");
    sgx_lfence();

    base_addr_in = base_addr_out;
    if (base_addr_in == NULL)
    {
        sgx_printf("Base addr null, exiting..\n");
        return;
    }
    romuluslog::gRomLog.per = (PersistentHeader *)per_out;
    sgx_printf("Pmem state: %d\n", romuluslog::gRomLog.per->id); //for testing

    romuluslog::gRomLog.ns_init(); //normally private..

    sgx_printf("Enclave init success..init base address for pmem\n");

    // Create an empty stack in PM and save the root pointer (index 0) to use in a later tx
    /* TM_WRITE_TRANSACTION([&]() {
        PStack *pstack = RomulusLog::get_object<PStack>(0);
        if (pstack == nullptr)
        {
            sgx_printf("Creating persistent stack...\n");
            PStack *pstack = (PStack *)TM_PMALLOC(sizeof(struct PStack));
            RomulusLog::put_object(0, pstack);
        }
        else
        {
            sgx_printf("Persistent stack exists...\n");
        }
    }); */
}

/* Worker: core data structure manipulations initialize from here */
void ecall_nvram_worker(int val, size_t tid)
{
    //start worker: worker pushes and pops values from the start
    do_work(val, tid);
}

void do_work(int val, size_t tid)
{

    //Pops a value from the persistent stack
    TM_WRITE_TRANSACTION([&]()
                         {
                             PStack *pstack = RomulusLog::get_object<PStack>(0);
                             // sgx_printf("Popped two items: %ld and %ld\n", pstack->pop(), pstack->pop());
                             // This one should be "EMTPY" which is 999999999
                             sgx_printf("Worker: %d Popped : %ld\n", tid, pstack->pop());
                         });

    // Add items to the persistent stack
    TM_WRITE_TRANSACTION([&]()
                         {
                             PStack *pstack = RomulusLog::get_object<PStack>(0);
                             pstack->push(val);
                             //pstack->push(44);
                         });

    // Pop two items from the persistent stack
    TM_WRITE_TRANSACTION([&]()
                         {
                             PStack *pstack = RomulusLog::get_object<PStack>(0);
                             // sgx_printf("Popped two items: %ld and %ld\n", pstack->pop(), pstack->pop());
                             // This one should be "EMTPY" which is 999999999
                             sgx_printf("Worker: %d Pushed and Popped : %ld\n", tid, pstack->pop());
                         });

    // Delete the persistent stack from persistent memory
    /*TM_WRITE_TRANSACTION([&]() {
        sgx_printf("Destroying persistent stack...\n");
        PStack *pstack = RomulusLog::get_object<PStack>(0);
        TM_PFREE(pstack);
        RomulusLog::put_object<PStack>(0, nullptr);
    });*/
}
