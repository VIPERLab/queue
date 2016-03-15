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
#define MAX_FRAME_SIZE 7000
#define ITEM_MAX_COUNT 10


template <class T> class GJQueue{

private:
    T buffer[ITEM_MAX_COUNT];
    T* retainBuffer[ITEM_MAX_COUNT];
    long _inPointer;  //尾
    long _outPointer; //头
    int _maxBufferSize;
    
   

    
    pthread_mutex_t _mutex;
    pthread_cond_t _inCond;
    pthread_cond_t _outCond;
    
    
    pthread_mutex_t _uniqueLock;
    
    bool _mutexInit()
    {
        if (!shouldWait) {
            return false;
        }
        pthread_mutex_init(&_mutex, NULL);
        pthread_cond_init(&_inCond, NULL);
        pthread_cond_init(&_outCond, NULL);
        
        pthread_mutex_init(&_uniqueLock, NULL);
        return true;
    }
    
    bool _mutexDestory()
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
    bool _mutexWait(pthread_cond_t* _cond)
    {
        if (!shouldWait) {
            return false;
        }
        pthread_mutex_lock(&_mutex);
        pthread_cond_wait(_cond, &_mutex);
        pthread_mutex_unlock(&_mutex);
        return true;
    }
    bool _mutexSignal(pthread_cond_t* _cond)
    {
        if (!shouldWait) {
            return false;
        }
        pthread_mutex_lock(&_mutex);
        pthread_cond_signal(_cond);
        pthread_mutex_unlock(&_mutex);
        return true;
    }
    
    bool _lock(pthread_mutex_t* mutex){
        if (!shouldWait) {
            return false;
        }
        return !pthread_mutex_lock(mutex);
    }
    bool _unLock(pthread_mutex_t* mutex){
        if (!shouldWait) {
            return false;
        }
       return !pthread_mutex_unlock(mutex);
    }
public:

    ~GJQueue(){
            _mutexDestory();
            dellocFreeCopyBlock(buffer,ITEM_MAX_COUNT);
    };

#pragma mark DELEGATE
    bool shouldWait;  //没有数据时是否支持等待，需要多线程；
    
    void (*popCopyBlock)(T* );  //deep copy with this
    
    void (*pushCopyBlock)(T* );
    
    void (*dellocFreeCopyBlock)(T* buffer,int lenth);
    
    GJQueue()
    {
        shouldWait = false;
        _inPointer = 0;
        _outPointer = 0;
        _mutexInit();
        popCopyBlock = NULL;
        pushCopyBlock = NULL;
        dellocFreeCopyBlock = NULL;
    };
    bool queueCopyPop(T* temBuffer){
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
        
        long temPoint = _outPointer;
        _outPointer++;
        _mutexSignal(&_outCond);
        GJQueueLOG("after signal out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
        _unLock(&_uniqueLock);
        
        if (popCopyBlock != NULL) {
            popCopyBlock(&buffer[temPoint%ITEM_MAX_COUNT]);
        }else{
            *temBuffer = buffer[temPoint%ITEM_MAX_COUNT];
        }
        return true;
    }
    bool queueCopyPush(T* temBuffer){
        
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
        long temInPointer = _inPointer;
        _inPointer++;
        _mutexSignal(&_inCond);
        GJQueueLOG("after signal in. incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
        _unLock(&_uniqueLock);
        if (pushCopyBlock != NULL) {
            pushCopyBlock(&buffer[temInPointer%ITEM_MAX_COUNT]);
        }else{
            buffer[temInPointer%ITEM_MAX_COUNT] = *temBuffer;
        }
        return true;
    }
    
    bool queueRetainPop(T** temBuffer){
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
        
        *temBuffer = retainBuffer[_outPointer%ITEM_MAX_COUNT];
        _outPointer++;
        _mutexSignal(&_outCond);
        GJQueueLOG("after signal out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
        _unLock(&_uniqueLock);
        return true;
    }
    
    bool queueRetainPush(T* temBuffer){
        
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
        retainBuffer[_inPointer%ITEM_MAX_COUNT] = temBuffer;
        _inPointer++;
        
        _mutexSignal(&_inCond);
        GJQueueLOG("after signal in. incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
        _unLock(&_uniqueLock);
        return true;
    }
    
    bool queuePush(T temBuffer){
        
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
    
    T queuePop(bool* result){
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
        
        long temPoint = _outPointer;
        _outPointer++;
        _mutexSignal(&_outCond);
        GJQueueLOG("after signal out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
        _unLock(&_uniqueLock);
        return buffer[temPoint%ITEM_MAX_COUNT];

    }
};

#endif /* GJQueue_h */
