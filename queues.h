//
// Created by Vladimir Schneider on 2018-01-28.
//
#include "stdint.h"

#ifndef MULTI_TASKER_QUEUES_H

typedef struct QNode QNode, QList;

struct QNode
{
    union
    {
        QNode *prev;
        QNode *next;
    };
    union
    {
        QNode *head;
        QNode *tail;
    };
};

// for asm code, offsets to structure's fields
#define QPREV 0
#define QNEXT 2
#define QTAIL QPREV
#define QHEAD QNEXT

// synonyms for asm code
#define __QNodeLinkTailInXY __QNodeLinkPrevInXY
#define __QNodeLinkHeadInXY __QNodeLinkNextInXY

#pragma callee_saves QInitNodes, QNodeUnlink, QNodeIsEmpty, QNodeLinkPrev, QNodeLinkTail, QNodeLinkNext, QNodeLinkHead

extern void QInitNode(QNode *node);  // initialize node
extern void QNodeUnlink(QNode *node);  // unlink node, if has prev then node.prev->next = node.next, if has next then node.next->prev = node.prev
extern uint8_t QNodeIsEmpty(QNode *node);  // return 0 if empty
extern void QNodeLinkPrev(QNode *node, QNode *other);  // link Y before X
extern void QNodeLinkTail(QList *queue, QNode *node);  // link Y at tail of queue (same as link before)
extern void QNodeLinkNext(QNode *node, QNode *other);  // link Y after X
extern void QNodeLinkHead(QList *queue, QNode *node);  // link Y at head of queue (same as link after0

#define MULTI_TASKER_QUEUES_H

#endif //MULTI_TASKER_QUEUES_H
