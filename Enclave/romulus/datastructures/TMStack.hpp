#ifndef _TM_STACK
#define _TM_STACK

#define ROMULUS_LOG_PTM
//All your pmem objects could use this as a template
//Include this template file inside enclave routine to use the pmem object

//#include "../romuluslog/RomulusLogSGX.hpp"
#include "../common/tm.h"
using namespace romuluslog;

// A simple persistent stack of integers (one integer per node)
class PStack {
    struct Node {
        TM_TYPE<uint64_t> key;
        TM_TYPE<Node*>    next;
    };

    TM_TYPE<Node*> head {nullptr};

public:
    TM_TYPE<uint64_t> EMPTY {999999999};

    void push(uint64_t key) {
         TM_WRITE_TRANSACTION([&] () {
           Node* node = (Node*) TM_PMALLOC(sizeof(struct Node));
            node->next = head;
            node->key = key;
            head = node;
        });
    }

    // Returns EMPTY if the stack has no keys
    uint64_t pop(void) {
        uint64_t key = 999999;//EMPTY;
        TM_WRITE_TRANSACTION([&] () {
           if (head == nullptr){
               sgx_printf("Stack empty...\n");
               return;
           };
           Node* node = head;
           key = node->key;
           head = node->next;           
           TM_PFREE(node);
           
        });        
        return key;
    }
};



#endif /* _TM_STACK */