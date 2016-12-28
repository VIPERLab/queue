//
//  GJQueue.h
//  GJQueue
//
//  Created by 未成年大叔 on 16/12/27.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//

#ifndef GJQueue_h
#define GJQueue_h
#include <pthread.h>
#include <stdio.h>

typedef struct _GJData{
    void* data;
    unsigned int size;
}GJData;

typedef struct _GJQueue{
    long inPointer;  //尾
    long outPointer; //头
    int capacity;
    int allocSize;
    
    void** queue;
    pthread_cond_t inCond;
    pthread_cond_t outCond;
    pthread_mutex_t pushLock;
    pthread_mutex_t popLock;
    //没有数据时是否支持等待，当为autoResize 为YES时，push永远不会等待
    int autoResize;//是否支持自动增长，当为YES时，push永远不会等待，只会重新申请内存,默认为false

}GJQueue;

int queueCreate(GJQueue** outQ,int capacity);
int queueRelease(GJQueue** inQ);

int queuePop(GJQueue* q,void** temBuffer,int ms);
int queuePush(GJQueue* q,void* temBuffer,int ms);
long queueGetLength(GJQueue* q);

//根据index获得vause,当超过inPointer和outPointer范围则失败，用于遍历数组，不会产生压出队列作用
int queuePeekValue(GJQueue* q,const long index,void** value);
int queuePeekTopOutValue(GJQueue* q,void** value, int ms);//没有数据则等待ms 时长

int queueUnLockPop(GJQueue* q);
int queueLockPush(GJQueue* q);
int queueUnLockPush(GJQueue* q);
int queueLockPop(GJQueue* q);


#endif /* GJQueue_h */
