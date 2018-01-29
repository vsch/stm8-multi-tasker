//
// Created by Vladimir Schneider on 2018-01-28.
//

/*
 * Circular Link List Implementation
 */

#include "queues.h"

void InitQNode(QNode *node)
{
    node->next = node;
    node->prev = node;
}

void QNodeUnlink(QNode *node)
{
    __SAVE_DISABLE_INT
    if (node->next != node)
    {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        node->next = node->prev = node;
    }
    __RESTORE_INT
}

uint8_t QNodeTest(QNode *node)
{
    uint8_t a;
    __SAVE_DISABLE_INT
    a = (uint8_t) (node->next == node);
    __RESTORE_INT
    return a;
}

void QNodeLinkPrev(QNode *node, QNode *other)
{
    __SAVE_DISABLE_INT
    QNodeUnlink(other);
    other->prev = node->prev;
    other->next = node;
    node->prev = other;
    __RESTORE_INT
}

void QNodeLinkTail(QNode *queue, QNode *node) __naked {
    (void) queue;
    (void) node;
    __SYNONYM_FOR(QNodeLinkPrev)
}

void QNodeLinkNext(QNode *node, QNode *other)
{
    __SAVE_DISABLE_INT
    QNodeUnlink(other);
    other->next = node->next;
    other->prev = node;
    node->next = other;
    __RESTORE_INT
}

void QNodeLinkHead(QNode *queue, QNode *node) __naked {
    (void) queue;
    (void) node;
    __SYNONYM_FOR(QNodeLinkNext)
}

