//
// Created by Vladimir Schneider on 2018-01-28.
//
#include "stdint.h"
#include "helpers.h"

#ifndef SDCC_TEST_QUEUES_H

struct QNode
{
    struct QNode *prev;
    struct QNode *next;
};

// for asm code, offsets to structure's fields
#define QPREV 0
#define QNEXT 2
#define QTAIL QPREV
#define QHEAD QNEXT

typedef struct QNode QNode;

extern void InitQNode(QNode *node);  // sets the two word pointers to QPREV
extern void QNodeUnlink(QNode *node);  // unlink node, if has prev then node.prev->next = node.next, if has next then node.next->prev = node.prev
extern uint8_t QNodeTest(QNode *node);  // return 0 if empty
extern void QNodeLinkPrev(QNode *node, QNode *other);  // link Y before X
extern void QNodeLinkTail(QNode *queue, QNode *node);  // link Y at tail of queue (same as link before)
extern void QNodeLinkNext(QNode *node, QNode *other);  // link Y after X
extern void QNodeLinkHead(QNode *queue, QNode *node);  // link Y at head of queue (same as link after0

#define SDCC_TEST_QUEUES_H

#endif //SDCC_TEST_QUEUES_H
