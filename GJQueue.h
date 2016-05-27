//
//  GJQueue.h
//  GJQueue
//
//  Created by tongguan on 16/3/15.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//

#ifdef DEBUG 
#define GJQueueLOG(format, ...) fprintf(stdout, format, ##__VA_ARGS__)
#else
#define GJQueueLOG(format, ...)
#endif

#ifndef GJQueue_h
#define GJQueue_h

#include <stdio.h>
#include <pthread.h>
#define ITEM_MAX_COUNT 10
typedef enum GJQueueType{
    queueAssignType,///直接赋值到列队
    queueCopyType,///支持深拷贝，自定义
}GJQueueType;

template <class T> class GJQueue{

private:
    T buffer[ITEM_MAX_COUNT];
    long _inPointer;  //尾
    long _outPointer; //头
    int _maxBufferSize;
    GJQueueType _queueType;
    
    pthread_mutex_t _mutex;
    pthread_cond_t _inCond;
    pthread_cond_t _outCond;
    pthread_mutex_t _uniqueLock;
    
    
    bool _mutexInit();
    bool _mutexDestory();
    bool _mutexWait(pthread_cond_t* _cond);
    bool _mutexSignal(pthread_cond_t* _cond);
    bool _lock(pthread_mutex_t* mutex);
    bool _unLock(pthread_mutex_t* mutex);
   
public:

    ~GJQueue(){
            _mutexDestory();
    };

#pragma mark DELEGATE
    bool shouldWait;  //没有数据时是否支持等待，需要多线程；
    /**
     *  //自定义深复制，比如需要复制结构体里面的指针需要复制，为空时则直接赋值指针；
     *dest 为目标地址，soc是赋值源
     */
    void (*popCopyBlock)(T* dest,T* soc);
    void (*pushCopyBlock)(T* dest,T* soc);
    
    bool queueCopyPop(T* temBuffer);
    bool queueCopyPush(T* temBuffer);
    bool queuePush(T temBuffer);
    T queuePop(bool* result);
    GJQueue(GJQueueType type);
};

#endif /* GJQueue_h */
