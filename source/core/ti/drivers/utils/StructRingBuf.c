/*
 * Copyright (c) 2018, Texas Instruments Incorporated
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

#include <ti/drivers/utils/StructRingBuf.h>

#include <ti/drivers/dpl/HwiP.h>

/*
 *  ======== StructRingBuf_construct ========
 *  初始化结构体环形缓冲区，head 指向末尾（下次写入 head+1）
 */
void StructRingBuf_construct(StructRingBuf_Handle object, void *bufPtr,
    size_t bufSize, size_t structSize)
{
    object->buffer = bufPtr;
    object->length = bufSize;
    object->count = 0;          /* 初始无数据 */
    object->head = bufSize - 1; /* head 指向最后位置，首次写入时先 +1 */
    object->tail = 0;
    object->maxCount = 0;
    object->structSize = structSize; /* 记录单个结构体字节大小 */
}

/*
 *  ======== StructRingBuf_get ========
 *  从缓冲区头部取出一个结构体并移除，缓冲区为空返回 -1
 */
int StructRingBuf_get(StructRingBuf_Handle object, void *data)
{
    unsigned int key;

    key = HwiP_disable(); /* 关中断 */

    if (!object->count) {
        HwiP_restore(key);
        return -1;            /* 缓冲区为空 */
    }

    /* 从 tail 位置拷贝结构体数据到输出缓冲区 */
    memcpy(data, &object->buffer[object->structSize * object->tail], object->structSize);
    object->tail = (object->tail + 1) % object->length; /* tail 循环前进 */
    object->count--;

    HwiP_restore(key); /* 恢复中断 */

    return (object->count);
}

/*
 *  ======== StructRingBuf_getCount ========
 *  返回缓冲区当前存储的结构体个数
 */
int StructRingBuf_getCount(StructRingBuf_Handle object)
{
    return (object->count);
}

/*
 *  ======== StructRingBuf_isFull ========
 *  判断缓冲区是否已满
 */
bool StructRingBuf_isFull(StructRingBuf_Handle object)
{
    return (object->count == object->length);
}

/*
 *  ======== StructRingBuf_getMaxCount ========
 *  返回缓冲区历史最大使用量
 */
int StructRingBuf_getMaxCount(StructRingBuf_Handle object)
{
    return (object->maxCount);
}

/*
 *  ======== StructRingBuf_peek ========
 *  窥视缓冲区头部结构体但不移除，返回指向数据的指针
 */
int StructRingBuf_peek(StructRingBuf_Handle object, void **data)
{
    /* 直接返回 tail 位置的缓冲区地址，避免拷贝 */
    *data = &object->buffer[object->structSize * object->tail];

    return (object->count);
}

/*
 *  ======== StructRingBuf_put ========
 *  向缓冲区尾部写入一个结构体，缓冲区已满返回 -1
 */
int StructRingBuf_put(StructRingBuf_Handle object, const void *data)
{
    unsigned int key;
    unsigned int next;

    key = HwiP_disable(); /* 关中断 */

    if (object->count != object->length) {
        next = (object->head + 1) % object->length; /* 计算下一写入位置 */
        /* 将结构体数据拷贝到 next 位置 */
        memcpy(&object->buffer[object->structSize * next], data, object->structSize);
        object->head = next;
        object->count++;
        /* 更新高水位标记 */
        object->maxCount = (object->count > object->maxCount) ?
                            object->count :
                            object->maxCount;
    }
    else {

        HwiP_restore(key);
        return (-1);  /* 缓冲区已满 */
    }

    HwiP_restore(key); /* 恢复中断 */

    return (object->count);
}
