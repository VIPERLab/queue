//
//  GJQueue.h
//  GJQueue
//
//  Created by tongguan on 16/3/15.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//



#ifndef GJQueue_h
#define GJQueue_h

#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#ifdef DEBUG
#define GJQueueLOG(format, ...) printf(format,##__VA_ARGS__)
#else
#define GJQueueLOG(format, ...)
#endif

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

template<class T>
GJQueue<T>::GJQueue(GJQueueType type)
{
    _queueType = type;
    shouldWait = false;
    _inPointer = 0;
    _outPointer = 0;
    _mutexInit();
    popCopyBlock = NULL;
    pushCopyBlock = NULL;
};

/**
 *  深拷贝
 *
 *  @param temBuffer 用来接收推出的数据
 *
 *  @return 结果
 */
template<class T>
bool GJQueue<T>::queueCopyPop(T* temBuffer){
    if (_queueType != queueCopyType) {
        GJQueueLOG("queue type wrong!!! ----------\n");
        return false;
        
    }
    _lock(&_uniqueLock);
    if (_inPointer <= _outPointer) {
        _unLock(&_uniqueLock);
        GJQueueLOG("begin Wait in ----------\n");
        if (!_mutexWait(&_inCond)) {
            return false;
        }
        _lock(&_uniqueLock);
        
        GJQueueLOG("after Wait in.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
    }
    
    if (popCopyBlock != NULL) {
        popCopyBlock(temBuffer, &buffer[_outPointer%ITEM_MAX_COUNT]);
    }else{
        *temBuffer = buffer[_outPointer%ITEM_MAX_COUNT];
    }
    _outPointer++;
    _mutexSignal(&_outCond);
    GJQueueLOG("after signal out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
    _unLock(&_uniqueLock);
    
    
    return true;
}
template<class T>
bool GJQueue<T>::queueCopyPush(T* temBuffer){
    if (_queueType != queueCopyType) {
        GJQueueLOG("queue type wrong!!! ----------\n");
        return false;
    }
    _lock(&_uniqueLock);
    if ((_inPointer % ITEM_MAX_COUNT == _outPointer % ITEM_MAX_COUNT && _inPointer > _outPointer)) {
        _unLock(&_uniqueLock);
        
        GJQueueLOG("begin Wait out ----------\n");
        if (!_mutexWait(&_outCond)) {
            return false;
        }
        
        _lock(&_uniqueLock);
        GJQueueLOG("after Wait out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
    }
    if (pushCopyBlock != NULL) {
        pushCopyBlock(&buffer[_inPointer%ITEM_MAX_COUNT],temBuffer);
    }else{
        buffer[_inPointer%ITEM_MAX_COUNT] = *temBuffer;
    }
    _inPointer++;
    _mutexSignal(&_inCond);
    GJQueueLOG("after signal in. incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
    _unLock(&_uniqueLock);
    
    return true;
}

template<class T>
bool GJQueue<T>::queuePush(T temBuffer){
    if (_queueType != queueAssignType) {
        GJQueueLOG("queue type wrong!!! ----------\n");
        return false;
    }
    _lock(&_uniqueLock);
    if ((_inPointer % ITEM_MAX_COUNT == _outPointer % ITEM_MAX_COUNT && _inPointer > _outPointer)) {
        _unLock(&_uniqueLock);
        
        GJQueueLOG("begin Wait out ----------\n");
        
        if (!_mutexWait(&_outCond)) {
            return false;
        }
        _lock(&_uniqueLock);
        GJQueueLOG("after Wait out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
    }
    buffer[_inPointer%ITEM_MAX_COUNT] = temBuffer;
    _inPointer++;
    
    _mutexSignal(&_inCond);
    GJQueueLOG("after signal in. incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
    _unLock(&_uniqueLock);
    return true;
}

template<class T>
T GJQueue<T>::queuePop(bool* result){
    if (_queueType != queueAssignType) {
        GJQueueLOG("queue type wrong!!! ----------\n");
        assert(0);
        return T();
    }
    _lock(&_uniqueLock);
    if (_inPointer <= _outPointer) {
        _unLock(&_uniqueLock);
        GJQueueLOG("begin Wait in ----------\n");
        if (!_mutexWait(&_inCond)) {
            if (result != NULL) {
                *result = false;
            }
            GJQueueLOG("信号等待出错")
            assert(0);
            return T();
        }
        _lock(&_uniqueLock);
        GJQueueLOG("after Wait in.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
    }
    
    long temPoint = _outPointer;
    _outPointer++;
    _mutexSignal(&_outCond);
    GJQueueLOG("after signal out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
    _unLock(&_uniqueLock);
    return buffer[temPoint%ITEM_MAX_COUNT];
    
}

template<class T>
bool GJQueue<T>::_mutexInit()
{
    //        if (!shouldWait) {
    //            return false;
    //        }
    pthread_mutex_init(&_mutex, NULL);
    pthread_cond_init(&_inCond, NULL);
    pthread_cond_init(&_outCond, NULL);
    
    pthread_mutex_init(&_uniqueLock, NULL);
    return true;
}

template<class T>
bool GJQueue<T>::_mutexDestory()
{
    if (!shouldWait) {
        return false;
    }
    pthread_mutex_destroy(&_mutex);
    pthread_cond_destroy(&_inCond);
    pthread_cond_destroy(&_outCond);
    pthread_mutex_destroy(&_uniqueLock);
    return true;
}
template<class T>
bool GJQueue<T>::_mutexWait(pthread_cond_t* _cond)
{
    if (!shouldWait) {
        return false;
    }
    pthread_mutex_lock(&_mutex);
    pthread_cond_wait(_cond, &_mutex);
    pthread_mutex_unlock(&_mutex);
    return true;
}
template<class T>
bool GJQueue<T>::_mutexSignal(pthread_cond_t* _cond)
{
    if (!shouldWait) {
        return false;
    }
    pthread_mutex_lock(&_mutex);
    pthread_cond_signal(_cond);
    pthread_mutex_unlock(&_mutex);
    return true;
}
template<class T>
bool GJQueue<T>::_lock(pthread_mutex_t* mutex){
    if (!shouldWait) {
        return false;
    }
    return !pthread_mutex_lock(mutex);
}
template<class T>
bool GJQueue<T>::_unLock(pthread_mutex_t* mutex){
    if (!shouldWait) {
        return false;
    }
    return !pthread_mutex_unlock(mutex);
}

#endif /* GJQueue_h */
