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
#include <pthread.h>
#include <assert.h>
#ifdef DEBUG
#define GJQueueLOG(format, ...) printf(format,##__VA_ARGS__)
#else
#define GJQueueLOG(format, ...)
#endif

#define DEFAULT_MAX_COUNT 10


template <class T> class GJQueue{

private:
    T *buffer;
    long _inPointer;  //尾
    long _outPointer; //头
    int _capacity;
    int _allocSize;
    
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
    void _init();
    void _resize();
public:

    ~GJQueue(){
        _mutexDestory();
        free(buffer);
    };

#pragma mark DELEGATE
    bool shouldWait;  //没有数据时是否支持等待，当为autoResize 为YES时，push永远不会等待
    bool shouldNonatomic; //是否多线程，
    bool autoResize; //当为YES时，push永远不会等待
    /**
     *  //自定义深复制，比如需要复制结构体里面的指针需要复制，为空时则直接赋值指针；
     *dest 为目标地址，soc是赋值源
     */
    void (*popCopyBlock)(T* dest,T* soc);//出栈时调用，释放压栈时的内存
    void (*pushCopyBlock)(T* dest,T* soc);//压栈时调用，用于自定义深复制，必要时需要申请内存，
    
    bool queuePop(T* temBuffer);
    bool queuePush(T* temBuffer);
    int getCurrentCount();
    GJQueue(int capacity);
    GJQueue();

};

template<class T>
int GJQueue<T>::getCurrentCount(){
    return _outPointer - _inPointer;
}
template<class T>
GJQueue<T>::GJQueue()
{
    _capacity = DEFAULT_MAX_COUNT;
    _init();
}
template<class T>
GJQueue<T>::GJQueue(int capacity)
{
    _capacity = capacity;
    if (capacity <=0) {
        _capacity = DEFAULT_MAX_COUNT;
    }
    _init();
};

template<class T>void
GJQueue<T>::_init()
{
    buffer = (T*)malloc(sizeof(T)*_capacity);
    _allocSize = _capacity;
    autoResize = true;
    shouldWait = false;
    shouldNonatomic = false;
    _inPointer = 0;
    _outPointer = 0;
    _mutexInit();
    popCopyBlock = NULL;
    pushCopyBlock = NULL;
}

/**
 *  深拷贝
 *
 *  @param temBuffer 用来接收推出的数据
 *
 *  @return 结果
 */
template<class T>
bool GJQueue<T>::queuePop(T* temBuffer){
    
    _lock(&_uniqueLock);
    if (_inPointer <= _outPointer) {
        _unLock(&_uniqueLock);
        GJQueueLOG("begin Wait in ----------\n");
        if (!_mutexWait(&_inCond)) {
            GJQueueLOG("fail Wait in ----------\n");
            return false;
        }
        _lock(&_uniqueLock);
        GJQueueLOG("after Wait in.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
    }
    
    if (popCopyBlock != NULL) {
        popCopyBlock(temBuffer, &buffer[_outPointer%_allocSize]);
    }else{
        *temBuffer = buffer[_outPointer%_allocSize];
    }
    _outPointer++;
    _mutexSignal(&_outCond);
    GJQueueLOG("after signal out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
    _unLock(&_uniqueLock);
    return true;
}
template<class T>
bool GJQueue<T>::queuePush(T* temBuffer){
    
    _lock(&_uniqueLock);
    if ((_inPointer % _allocSize == _outPointer % _allocSize && _inPointer > _outPointer)) {
        if (autoResize) {
            _resize();
        }else{
            _unLock(&_uniqueLock);
            
            GJQueueLOG("begin Wait out ----------\n");
            if (!_mutexWait(&_outCond)) {
                GJQueueLOG("fail begin Wait out ----------\n");
                return false;
            }
            
            _lock(&_uniqueLock);
            GJQueueLOG("after Wait out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
        }
    }
    if (pushCopyBlock != NULL) {
        pushCopyBlock(&buffer[_inPointer%_allocSize],temBuffer);
    }else{
        buffer[_inPointer%_allocSize] = *temBuffer;
    }
    _inPointer++;
    _mutexSignal(&_inCond);
    GJQueueLOG("after signal in. incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
    _unLock(&_uniqueLock);
    
    return true;
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
    //    if (!shouldWait) {
    //        return false;
    //    }
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
    if (!shouldNonatomic) {
        return false;
    }
    return !pthread_mutex_lock(mutex);
}
template<class T>
bool GJQueue<T>::_unLock(pthread_mutex_t* mutex){
    if (!shouldNonatomic) {
        return false;
    }
    return !pthread_mutex_unlock(mutex);
}
template<class T>
void GJQueue<T>::_resize(){

    T* temBuffer = (T*)malloc(sizeof(T)*(_allocSize + _capacity));
    for (long i = _outPointer,j =0; i<_inPointer; i++,j++) {
        temBuffer[j] = buffer[i%_allocSize];
    }
    free(buffer);
    buffer = temBuffer;
    _inPointer = _allocSize;
    _outPointer = 0;
    _allocSize += _capacity;

}
#endif /* GJQueue_h */
