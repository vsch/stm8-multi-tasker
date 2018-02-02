//
// Created by Vladimir Schneider on 2018-01-28.
//

/*
 * Circular Link List Implementation
 */

#include "multitasker.h"

const uint16_t queueDescriptionSize = sizeof(QueueDescription);

void QInitNode(QNode *node)__naked
{
    (void)node;
//    node->next = node;
//    node->prev = node;
// @formatter:off
    __asm
    ldw     x,(1,sp)
    ; fallthrough to next function, assume function order is fixed by source
    ;jra     __InitQNodeInX
__endasm;
// @formatter:on
}

/*
 * X - node
 * call from asm
 *
 * return
 * Y = X, X unchanged
 */
void _InitQNodeInX()__naked
{
// @formatter:off
__asm
    ldw     y,x
    ldw     (QNEXT,x),y
    ldw     (QPREV,x),y
    ret
__endasm;
// @formatter:on
}

/*
 * X - node
 * interrupts disabled
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
    ldw     y,x
    cpw     x,(QPREV,y) ; see if already unlinked
    jreq    unlink.done

    pushw   x           ; save node
    ldw     y,(QNEXT,y) ; y = node->next
    ldw     x,(QPREV,x) ; x = node->prevn
    ldw     (QPREV,y),x ; node->next->prev = node->prev

    exgw    x,y       ; x = node->next
    ldw     y,(1,sp)  ; y = node
    ldw     y,(QPREV,y) ; y = node->prev
    ldw     (QNEXT,y),x ; node->prev->next = node->next

    popw    x       ; x = node
    ldw     y,x
    ldw     (QPREV,x),y
    ldw     (QNEXT,x),y

unlink.done:
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
    sim
    ldw     x,(2,sp) ; get node sp: cc, x.h, x.l
    callr    __QNodeUnlinkInX
    pop     cc
    ret
__endasm;
// @formatter:on
}

uint8_t QNodeIsEmpty(QNode *node)__naked
{
    (void)node;
//    uint8_t a;
//    __SAVE_DISABLE_INT
//    a = (uint8_t) (node->next == node);
//    __RESTORE_INT
//    return a;
// @formatter:off
__asm
    push    cc
    sim
    ldw     x,(2,sp)  ; get node sp: cc, x.h, x.l
    ldw     y,x
    clr     a
    cpw     y,(QNEXT,x)
    jreq    test.done
    inc     a
test.done:
    pop     cc
    ret
__endasm;
// @formatter:on
}

/*
 * X - node
 * Y - other
 * interrupts are disabled
 * return Y is first unlinked, then linked previous to X
 */
void _QNodeLinkPrevInXY()__naked
{
//    __SAVE_DISABLE_INT
//    QNodeUnlink(other);
//    other->prev = node->prev;
//    node->prev->next = other
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
    ldw     x,(QPREV,x) ; x = node->prev
    ldw     (QPREV,y),x ; other->prev = node->prev
    ldw     (QNEXT,x),y ; node->prev->next = other
    popw    x           ; x = node
    ldw     (QPREV,x),y ; node->prev = other
    ret
__endasm;
// @formatter:on
}

void QNodeLinkTail(QNode *queue, QNode *node) __naked {
    (void) queue;
    (void) node;
//    __SYNONYM_FOR(QNodeLinkPrev)
// fallthrough to next function, assume function order is fixed by source
}

void QNodeLinkPrev(QNode *node, QNode *other)__naked
{
    (void)node;
    (void)other;
// @formatter:off
__asm
    push    cc
    sim
    ldw     y,(4,sp)  ; get other sp: cc, pc.h, pc.l, oth.h, oth.l, n.h, n.l
    ldw     x,(6,sp)  ; get node
    callr    __QNodeLinkPrevInXY
    pop     cc
    ret
__endasm;
// @formatter:on
}

/*
 * X - node
 * Y - other
 * interrupts disabled
 * return Y is first unlinked, then linked after X
 */
void _QNodeLinkNextInXY()
{
//    __SAVE_DISABLE_INT
//    QNodeUnlink(other);
//    other->next = node->next;
//    node->next->prev = other
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
    ldw     x,(QNEXT,x) ; x = node->next
    ldw     (QNEXT,y),x ; other->next = node->next
    ldw     (QPREV,x),y ; node->next->prev = other
    popw    x           ; x = node
    ldw     (QNEXT,x),y ; node->next = other
    ret
__endasm;
// @formatter:on

}

void QNodeLinkHead(QNode *queue, QNode *node) __naked {
    (void) queue;
    (void) node;
//    __SYNONYM_FOR(QNodeLinkNext)
    // fallthrough to next function, assume function order is fixed by source
}

void QNodeLinkNext(QNode *node, QNode *other)
{
    (void)node;
    (void)other;
// @formatter:off
__asm
    push    cc
    sim
    ldw     y,(4,sp) ; get other sp: cc, pc.h, pc.l, oth.h, oth.l, n.h, n.l
    ldw     x,(6,sp) ; get node
    call    __QNodeLinkNextInXY
    pop     cc
    ret
__endasm;
// @formatter:on
}

