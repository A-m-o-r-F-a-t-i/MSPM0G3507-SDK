/*
 * Copyright (c) 2015, 2017 Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ======== List.c ========
 *  双向链表实现，提供线程安全的元素增删查操作
 */
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/utils/List.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/*
 *  ======== List_clearList ========
 *  清空链表，将头尾指针置 NULL
 */
void List_clearList(List_List *list)
{
    uintptr_t key;

    key = HwiP_disable();          /* 关中断，保证原子操作 */

    list->head = list->tail = NULL; /* 头尾均置空 */

    HwiP_restore(key);             /* 恢复中断 */
}



/*
 *  ======== List_get ========
 *  原子取出链表头部元素并移除
 */
List_Elem *List_get(List_List *list)
{
    List_Elem *elem;
    uintptr_t key;

    key = HwiP_disable();   /* 关中断 */

    elem = list->head;

    /* 链表非空时摘除头部节点 */
    if (elem != NULL) {
        list->head = elem->next;
        if (elem->next != NULL) {
            elem->next->prev = NULL; /* 新头节点的 prev 置空 */
        }
        else {
            list->tail = NULL;       /* 链表变空，tail 也置空 */
        }
    }

    HwiP_restore(key);       /* 恢复中断 */

    return (elem);
}


/*
 *  ======== List_insert ========
 *  在 curElem 之前插入新元素 newElem
 */
void List_insert(List_List *list, List_Elem *newElem, List_Elem *curElem)
{
    uintptr_t key;

    key = HwiP_disable(); /* 关中断 */

    newElem->next = curElem;
    newElem->prev = curElem->prev;
    if (curElem->prev != NULL) {
        curElem->prev->next = newElem; /* 将前驱节点的 next 指向新节点 */
    }
    else {
        list->head = newElem;          /* curElem 原为头节点，新节点成为新头 */
    }
    curElem->prev = newElem;           /* curElem 的 prev 指向新节点 */

    HwiP_restore(key); /* 恢复中断 */
}


/*
 *  ======== List_put ========
 *  原子将元素追加到链表尾部（FIFO 入队）
 */
void List_put(List_List *list, List_Elem *elem)
{
    uintptr_t key;

    key = HwiP_disable(); /* 关中断 */

    elem->next = NULL;
    elem->prev = list->tail;
    if (list->tail != NULL) {
        list->tail->next = elem; /* 原尾节点的 next 指向新节点 */
    }
    else {
        list->head = elem;       /* 链表原为空，新节点同时成为头节点 */
    }

    list->tail = elem;           /* 更新尾指针 */

    HwiP_restore(key); /* 恢复中断 */
}

/*
 *  ======== List_putHead ========
 *  原子将元素插入到链表头部（LIFO 入栈）
 */
void List_putHead(List_List *list, List_Elem *elem)
{
    uintptr_t key;

    key = HwiP_disable(); /* 关中断 */

    elem->next = list->head;
    elem->prev = NULL;
    if (list->head != NULL) {
        list->head->prev = elem; /* 原头节点的 prev 指向新节点 */
    }
    else {
        list->tail = elem;       /* 链表原为空，新节点同时成为尾节点 */
    }

    list->head = elem;           /* 更新头指针 */

    HwiP_restore(key); /* 恢复中断 */
}

/*
 *  ======== List_remove ========
 *  从链表中移除指定元素
 */
void List_remove(List_List *list, List_Elem *elem)
{
    uintptr_t key;

    key = HwiP_disable(); /* 关中断 */

    /* 若待删元素是尾节点，更新 tail 指针 */
    if (elem->next == NULL) {
        list->tail = elem->prev;
    }
    else {
        elem->next->prev = elem->prev; /* 后继节点的 prev 跳过待删元素 */
    }

    /* 若待删元素是头节点，更新 head 指针 */
    if (elem->prev == NULL) {
        list->head = elem->next;
    }
    else {
        elem->prev->next = elem->next; /* 前驱节点的 next 跳过待删元素 */
    }

    HwiP_restore(key); /* 恢复中断 */
}
