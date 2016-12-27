//
//  GJQueue.c
//  GJQueue
//
//  Created by 未成年大叔 on 16/12/27.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//

#include "GJQueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#ifdef DEBUG
#define GJQueueLOG(format, ...) printf(format,##__VA_ARGS__)
#else
#define GJQueueLOG(format, ...)
#endif

#define DEFAULT_MAX_COUNT 3

#define DEFAULT_TIME 100000000

int _condWait(pthread_cond_t* _cond,pthread_mutex_t* mutex,int ms);
int _condBroadcast(pthread_cond_t* _cond);
int _condSignal(pthread_cond_t* _cond);

inline int _condWait(pthread_cond_t* _cond,pthread_mutex_t* mutex,int ms)
{
    struct timespec ts;
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    ms += tv.tv_usec / 1000;
    ts.tv_sec = tv.tv_sec + ms / 1000;
    ts.tv_nsec = ms % 1000 * 1000000;
    int ret = pthread_cond_timedwait(_cond, mutex, &ts);
    printf("ret:%d,,%d\n",ret,!ret);
    return !ret;
}

inline int _condSignal(pthread_cond_t* _cond)
{
    return !pthread_cond_signal(_cond);
}

inline int _condBroadcast(pthread_cond_t* _cond)
{
    return !pthread_cond_broadcast(_cond);
}
inline int queueUnLockPop(GJQueue* q){
    return !pthread_mutex_unlock(&q->popLock);
}
inline int queueLockPush(GJQueue* q){
    return !pthread_mutex_lock(&q->pushLock);
}
inline int queueUnLockPush(GJQueue* q){
    return !pthread_mutex_unlock(&q->pushLock);
}

inline int queueLockPop(GJQueue* q){
    return !pthread_mutex_lock(&q->popLock);
}

#pragma mark DELEGATE

    
  

long queueGetLength(GJQueue* q){
    return (q->outPointer -  q->inPointer);
}

int queuePeekValue(GJQueue* q, const long index,void** value){
    if (index < q->outPointer || index >= q->inPointer) {
        return -1;
    }
    long current = index%q->allocSize;
    *value = q->queue[current];
    return 0;
}
int queuePeekTopOutValue(GJQueue* q,void** value){
    void* retV = NULL;
    int ret = -1;
    queueLockPop(q);
    if (  q->outPointer == q->inPointer) {
        retV = NULL;
    }else{
        retV = q->queue[q->outPointer % q->allocSize];
        ret = 0;
    }
    queueUnLockPop(q);
    *value = retV;
    return ret;
}
/**
 *  深拷贝
 *
 *  @param temBuffer 用来接收推出的数据
 *
 *  @return 结果
 */
int queuePop(GJQueue* q,void** temBuffer,int ms){
    queueLockPop(q);
    if (q->inPointer <= q->outPointer) {
        GJQueueLOG("begin Wait in ----------\n");
        if (ms <= 0 || !_condWait(&q->outCond,&q->popLock,ms)) {
            GJQueueLOG("fail Wait in ----------\n");
            queueUnLockPop(q);
            return -1;
        }
        GJQueueLOG("after Wait in.  incount:%ld  outcount:%ld----------\n",q->inPointer,q->outPointer);
    }
    int index = q->outPointer%q->allocSize;
    *temBuffer = q->queue[index];
    memset(&q->queue[index], 0, sizeof(void*));//防止在oc里的引用一直不释放；
    
    q->outPointer++;
    _condSignal(&q->inCond);
    GJQueueLOG("after signal out.  incount:%ld  outcount:%ld----------\n",q->inPointer,q->outPointer);
    queueUnLockPop(q);
    assert(*temBuffer);
    return 0;
}
int queuePush(GJQueue* q,void* temBuffer,int ms){
    queueLockPush(q);
    if ((q->inPointer % q->allocSize == q->outPointer % q->allocSize && q->inPointer > q->outPointer)) {
        if (q->autoResize) {
            //resize
            void** temBuffer = (void**)malloc(sizeof(void*)*(q->allocSize * 2));
            for (long i = q->outPointer,j =0; i<q->inPointer; i++,j++) {
                temBuffer[j] = q->queue[i%q->allocSize];
            }
            free(q->queue);
            q->queue = temBuffer;
            q->inPointer = q->allocSize;
            q->outPointer = 0;
            q->allocSize += q->capacity;
        }else{
            GJQueueLOG("begin Wait out ----------\n");
            if (ms <= 0 || !_condWait(&q->inCond,&q->pushLock,ms)) {
                GJQueueLOG("fail begin Wait out ----------\n");
                queueUnLockPush(q);
                return -1;
            }
            GJQueueLOG("after Wait out.  incount:%ld  outcount:%ld----------\n",q->inPointer,q->outPointer);
        }
    }
    q->queue[q->inPointer%q->allocSize] = temBuffer;
    q->inPointer++;
    _condSignal(&q->outCond);
    GJQueueLOG("after signal in. incount:%ld  outcount:%ld----------\n",q->inPointer,q->outPointer);
    queueUnLockPush(q);
    assert(temBuffer);
    
    return 0;
}

void queueClean(GJQueue* q){
    queueLockPop(q);
    _condBroadcast(&q->inCond);//确保可以锁住下一个,避免循环锁
    queueLockPush(q);
    while (q->outPointer<q->inPointer) {
        memset(&q->queue[q->outPointer++%q->allocSize], 0, sizeof(void*));//防止在oc里的引用一直不释放；
    }
    q->inPointer=q->outPointer=0;
    _condBroadcast(&q->inCond);
    queueUnLockPush(q);
    queueUnLockPop(q);
}

int queueCreate(GJQueue** outQ,int capacity){
    GJQueue* q = (GJQueue*)malloc(sizeof(GJQueue));
    if (!q) {
        return -1;
    }
    memset(q, 0, sizeof(GJQueue));
    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_cond_init(&q->inCond, &cond_attr);
    pthread_cond_init(&q->outCond, &cond_attr);
    pthread_mutex_init(&q->popLock, NULL);
    pthread_mutex_init(&q->pushLock, NULL);
    if (capacity<=0) {capacity = DEFAULT_MAX_COUNT;}
    q->queue = (void**)malloc(sizeof(void*) * q->capacity);
    if (!q->queue) {
        free(q);
        return -1;
    }
    *outQ = q;
    return 0;
}

int queueRelease(GJQueue** inQ){
    GJQueue* q = *inQ;
    if (!q) {
        return 0;
    }
    queueClean(q);
    free(q->queue);
    pthread_cond_destroy(&q->inCond);
    pthread_cond_destroy(&q->outCond);
    pthread_mutex_destroy(&q->popLock);
    pthread_mutex_destroy(&q->pushLock);
    *inQ = NULL;
    return 0;
}


