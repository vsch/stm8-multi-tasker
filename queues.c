//
// Created by Vladimir Schneider on 2018-01-28.
//

/*
 * Circular Link List Implementation
 */

#include "queues.h"

/*
 * X - node
 * call from asm
 *
 * return
 * Y == X
 * X unchanged
 */
void _InitQNodeInX()__naked
{
// @formatter:off
__asm
    ldw       y,x
    ldw       (x),y ; set one
    ldw       (2,x),y ; set the other
    ret
__endasm;
// @formatter:on
}

void InitQNode(QNode *node)__naked
{
    (void)node;
//    node->next = node;
//    node->prev = node;
// @formatter:off
__asm
    ldw     x,(2,sp)
    jra     __InitQNodeInX
__endasm;
// @formatter:on
}

/*
 * X - node
 * return Y = X, X unchanged
 */
void _QNodeUnlinkInX()__naked
{
//    __SAVE_DISABLE_INT
//    if (node->next != node)
//    {
//        node->next->prev = node->prev;
//        node->prev->next = node->next;
//        node->next = node->prev = node;
//    }
//    __RESTORE_INT
// @formatter:off
__asm
    pushw   x
    ldw     y,(QNEXT,y) ; y = node->next
    ldw     x,(QPREV,x)
    ldw     (QPREV,y),x ; node->next->prev = node->prev

    exgw    x,y       ; x = node->next
    ldw     y,(1,sp)  ; y = node
    ldw     y,(QPREV,y) ; y = node->prev
    ldw     (QPREV,y),x ; node->prev->next = node->next

    popw    x       ; x = node
    ldw     y,x
    ldw     (QPREV,x),y
    ldw     (QNEXT,x),y
    ret
__endasm;
// @formatter:on
}

void QNodeUnlink(QNode *node)__naked
{
    (void)node;
//    __SAVE_DISABLE_INT
//    if (node->next != node)
//    {
//        node->next->prev = node->prev;
//        node->prev->next = node->next;
//        node->next = node->prev = node;
//    }
//    __RESTORE_INT
// @formatter:off
__asm
    push    cc
    rim
    ldw     x,(2,sp)
    jra     __QNodeUnlinkInX
__endasm;
// @formatter:on
}

uint8_t QNodeTest(QNode *node)__naked
{
    (void)node;
//    uint8_t a;
//    __SAVE_DISABLE_INT
//    a = (uint8_t) (node->next == node);
//    __RESTORE_INT
//    return a;
// @formatter:off
__asm
    ldw     x,(2,sp)
    ldw     y,x
    clr     a
    cpw     y,(QNEXT,x)
    jreq    test.done
    inc     a
test.done:
    ret
__endasm;
// @formatter:on
}

/*
 * X - node
 * Y - other
 * return Y is first unlinked, then linked previous to X
 * interrupts are assumed to be disabled
 */
void _QNodeLinkPrevInXY()__naked
{
//    __SAVE_DISABLE_INT
//    QNodeUnlink(other);
//    other->prev = node->prev;
//    other->next = node;
//    node->prev = other;
//    __RESTORE_INT
// @formatter:off
__asm
    pushw   x
    exgw    x,y
    call    __QNodeUnlinkInX
    ; x & y both other
    ldw     x,(1,sp)    ; x = node
    ldw     (QNEXT,y),x ; other->next = node
    ldw     x,(QPREV,x)
    ldw     (QPREV,y),x ; other->prev = node->prev
    popw    x           ; x = node
    ldw     (QPREV,x),y ; node->prev = other
    ret
__endasm;
// @formatter:on
}

void QNodeLinkPrev(QNode *node, QNode *other)__naked
{
    (void)node;
    (void)other;
// @formatter:off
__asm
    push    cc
    rim
    ldw     y,(4,sp)
    ldw     x,(6,sp)
    call    __QNodeLinkPrevInXY
    pop     cc
    ret
__endasm;
// @formatter:on
}

void QNodeLinkTail(QNode *queue, QNode *node) __naked {
    (void) queue;
    (void) node;
    __SYNONYM_FOR(QNodeLinkPrev)
}

/*
 * X - node
 * Y - other
 * return Y is first unlinked, then linked after X
 * interrupts are assumed to be disabled
 */
void _QNodeLinkNextInXY()
{
//    __SAVE_DISABLE_INT
//    QNodeUnlink(other);
//    other->next = node->next;
//    other->prev = node;
//    node->next = other;
//    __RESTORE_INT
    // @formatter:off
__asm
    pushw   x
    exgw    x,y
    call    __QNodeUnlinkInX
    ; x & y both other
    ldw     x,(1,sp)    ; x = node
    ldw     (QPREV,y),x ; other->prev = node
    ldw     x,(QNEXT,x)
    ldw     (QNEXT,y),x ; other->next = node->next
    popw    x           ; x = node
    ldw     (QNEXT,x),y ; node->next = other
    ret
__endasm;
// @formatter:on

}

void QNodeLinkNext(QNode *node, QNode *other)
{
    (void)node;
    (void)other;
// @formatter:off
__asm
    push    cc
    rim
    ldw     y,(4,sp)
    ldw     x,(6,sp)
    call    __QNodeLinkNextInXY
    pop     cc
    ret
__endasm;
// @formatter:on
}

void QNodeLinkHead(QNode *queue, QNode *node) __naked {
    (void) queue;
    (void) node;
    __SYNONYM_FOR(QNodeLinkNext)
}

