/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
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
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/utils/RingBuf.h>

/*
 *  ======== RingBuf_construct ========
 *  初始化环形缓冲区，head 指向末尾（下次写入 head+1）
 */
void RingBuf_construct(RingBuf_Handle object, unsigned char *bufPtr,
    size_t bufSize)
{
    object->buffer = bufPtr;
    object->length = bufSize;
    object->count = 0;          /* 初始无数据 */
    object->head = bufSize - 1; /* head 指向最后一个位置，首次写入时先 +1 */
    object->tail = 0;
    object->maxCount = 0;
}

/*
 *  ======== RingBuf_get ========
 *  从缓冲区头部取出一个字节并移除，缓冲区为空返回 -1
 */
int RingBuf_get(RingBuf_Handle object, unsigned char *data)
{
    unsigned int key;

    key = HwiP_disable(); /* 关中断 */

    if (!object->count) {
        HwiP_restore(key);
        return -1;            /* 缓冲区为空 */
    }

    *data = object->buffer[object->tail];        /* 读取 tail 位置数据 */
    object->tail = (object->tail + 1) % object->length; /* tail 循环前进 */
    object->count--;

    HwiP_restore(key); /* 恢复中断 */

    return (object->count);
}

/*
 *  ======== RingBuf_getConsume ========
 *  零拷贝消费：推进读取索引，跳过已由 getPointer 直接读取的数据
 */
int RingBuf_getConsume(RingBuf_Handle object, size_t size)
{
    unsigned int key;

    key = HwiP_disable(); /* 关中断 */

    /* 限制消费量不超过已有数据量 */
    if (object->count < size) {
        size = object->count;
    }
    object->count -= size;
    object->tail += size;
    object->tail %= object->length; /* tail 循环回绕 */

    HwiP_restore(key); /* 恢复中断 */

    return (size);
}

/*
 *  ======== RingBuf_getCount ========
 *  返回缓冲区当前存储的字节数
 */
int RingBuf_getCount(RingBuf_Handle object)
{
    return (object->count);
}

/*
 *  ======== RingBuf_getMaxCount ========
 *  返回缓冲区历史最大使用量
 */
int RingBuf_getMaxCount(RingBuf_Handle object)
{
    return (object->maxCount);
}

/*
 *  ======== RingBuf_getn ========
 *  批量取出 n 个字节，实际取出量可能小于 n
 */
int RingBuf_getn(RingBuf_Handle object, unsigned char *data, size_t n)
{
    unsigned int key;
    size_t       removed = 0;

    key = HwiP_disable(); /* 关中断 */
    /* 限制取出量不超过已有数据 */
    if (n > object->count) {
        n = object->count;
    }
    while (n) {
        *data++ = object->buffer[object->tail++]; /* 逐字节复制 */
        object->tail %= object->length;           /* tail 循环回绕 */
        --object->count;
        --n;
        ++removed;
    }
    HwiP_restore(key); /* 恢复中断 */

    return (removed);
}

/*
 *  ======== RingBuf_getPointer ========
 *  零拷贝读取：返回可读数据的线性内存指针和长度
 */
int RingBuf_getPointer(RingBuf_Handle object, unsigned char **data)
{
    size_t       result;
    unsigned int key;

    key = HwiP_disable(); /* 关中断 */

    /* 计算 tail 到缓冲区末尾的连续长度 */
    result = object->length - object->tail;
    /* 实际可读长度取连续长度与已有数据量的较小值 */
    if (object->count < result) {
        result = object->count;
    }
    *data = object->buffer + object->tail; /* 输出可读数据起始地址 */

    HwiP_restore(key); /* 恢复中断 */

    return (result);
}

/*
 *  ======== RingBuf_flush ========
 *  清空缓冲区，同时保留历史最大使用量记录
 */
void RingBuf_flush(RingBuf_Handle object)
{
    uintptr_t key;
    size_t    maxCount;

    key = HwiP_disable(); /* 关中断 */
    maxCount = object->maxCount;          /* 保存高水位 */
    RingBuf_construct(object, object->buffer, object->length); /* 重新初始化 */
    object->maxCount = maxCount;          /* 恢复高水位 */
    HwiP_restore(key); /* 恢复中断 */
}

/*
 *  ======== RingBuf_isFull ========
 *  判断缓冲区是否已满
 */
bool RingBuf_isFull(RingBuf_Handle object)
{
    return (object->count == object->length);
}

/*
 *  ======== RingBuf_peek ========
 *  窥视缓冲区头部字节但不移除
 */
int RingBuf_peek(RingBuf_Handle object, unsigned char *data)
{
    unsigned int key;
    int          retCount;

    key = HwiP_disable(); /* 关中断 */

    *data = object->buffer[object->tail]; /* 读取但不移动 tail */
    retCount = object->count;

    HwiP_restore(key); /* 恢复中断 */

    return (retCount);
}

/*
 *  ======== RingBuf_put ========
 *  向缓冲区尾部写入一个字节，缓冲区已满返回 -1
 */
int RingBuf_put(RingBuf_Handle object, unsigned char data)
{
    unsigned int key;
    unsigned int next;

    key = HwiP_disable(); /* 关中断 */

    if (object->count != object->length) {
        next = (object->head + 1) % object->length; /* 计算下一个写入位置 */
        object->buffer[next] = data;                 /* 写入数据 */
        object->head = next;                         /* 更新 head */
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

/*
 *  ======== RingBuf_putAdvance ========
 *  零拷贝提交：推进写入索引，配合 putPointer 使用
 */
int RingBuf_putAdvance(RingBuf_Handle object, size_t size)
{
    unsigned int key;

    key = HwiP_disable(); /* 关中断 */

    /* 限制提交量不超过剩余空间 */
    if (RingBuf_space(object) < size) {
        size = RingBuf_space(object);
    }
    object->count += size;
    object->head += size;
    object->head %= object->length; /* head 循环回绕 */

    /* 更新高水位标记 */
    object->maxCount = (object->count > object->maxCount) ?
            object->count : object->maxCount;

    HwiP_restore(key); /* 恢复中断 */

    return (size);
}

/*
 *  ======== RingBuf_putn ========
 *  批量写入 n 个字节，实际写入量可能小于 n
 */
int RingBuf_putn(RingBuf_Handle object, unsigned char *data, size_t n)
{
    unsigned int key;
    size_t       next;
    size_t       added = 0;

    key = HwiP_disable(); /* 关中断 */
    /* 限制写入量不超过剩余空间 */
    if (n > RingBuf_space(object)) {
        n = RingBuf_space(object);
    }
    while (n) {
        next = (object->head + 1) % object->length; /* 计算下一写入位置 */
        object->buffer[next] = *data++;              /* 逐字节写入 */
        object->head = next;
        ++object->count;
        --n;
        ++added;
    }
    /* 更新高水位标记 */
    if (object->maxCount < object->count) {
        object->maxCount = object->count;
    }
    HwiP_restore(key); /* 恢复中断 */

    return (added);
}

/*
 *  ======== RingBuf_putPointer ========
 *  零拷贝写入：返回可写入区域的线性内存指针和长度
 */
int RingBuf_putPointer(RingBuf_Handle object, unsigned char **data)
{
    size_t       result;
    size_t       next;
    unsigned int key;

    key = HwiP_disable(); /* 关中断 */

    next = (object->head + 1) % object->length; /* 下一写入位置 */
    /* 计算从 next 到缓冲区末尾的连续可写长度 */
    result = object->length - next;
    /* 实际可写长度取连续长度与剩余空间的较小值 */
    if (RingBuf_space(object) < result) {
        result = RingBuf_space(object);
    }
    *data = object->buffer + next; /* 输出可写区域起始地址 */

    HwiP_restore(key); /* 恢复中断 */

    return (result);
}
