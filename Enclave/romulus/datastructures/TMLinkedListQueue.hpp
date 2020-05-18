/*
 * Copyright 2017-2018
 *   xxxx xxxx <xxxx.xxxx@xxxx>
 *   xxxx xxxx <xxxx.xxxx@xxxx>
 *   xxxx xxxx <xxxx@gmail.com>
 *
 * This work is published under the MIT license. See LICENSE.TXT
 */
#ifndef _TM_LINKED_LIST_QUEUE_H_
#define _TM_LINKED_LIST_QUEUE_H_


#include <string>

#include "../common/tm.h"

/**
 * <h1> A Linked List queue (memory unbounded) for usage with STMs and PTMs </h1>
 *
 */
template<typename T>
class TMLinkedListQueue {

private:
    struct Node {
        TM_TYPE<T*>    item;
        TM_TYPE<Node*> next;
        Node(T* userItem) : item{userItem}, next{nullptr} { }
    };

    alignas(128) TM_TYPE<Node*>  head {nullptr};
    alignas(128) TM_TYPE<Node*>  tail {nullptr};


public:
    TMLinkedListQueue() {
		Node* sentinelNode = TM_ALLOC<Node>(nullptr);
		head = sentinelNode;
		tail = sentinelNode;
    }


    ~TMLinkedListQueue() {
		while (dequeue() != nullptr); // Drain the queue
		Node* lhead = head;
		TM_FREE(lhead);
    }


    static std::string className() { return TM_NAME() + "-LinkedListQueue"; }


    bool enqueue(T* item) {
        TM_WRITE_TRANSACTION([&] () {
            Node* newNode = TM_ALLOC<Node>(item);
            tail->next = newNode;
            tail = newNode;
        });
        return true;
    }


    T* dequeue() {
        T* item = nullptr;
        TM_WRITE_TRANSACTION([&] () {
            Node* lhead = head;
            if (lhead == tail) return; // item = nullptr;
            head = lhead->next;
            TM_FREE(lhead);
            item = head->item;
        });
        return item;
    }
};

#endif /* _TM_LINKED_LIST_QUEUE_H_ */
