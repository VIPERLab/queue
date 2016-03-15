////
////  GJQueue.c
////  GJQueue
////
////  Created by tongguan on 16/3/15.
////  Copyright © 2016年 MinorUncle. All rights reserved.
////
//#ifdef DEBUG
//#define GJQueueLOG(format, ...) fprintf(stdout, format, ##__VA_ARGS__)
//#else
//#define GJQueueLOG(format, ...)
//#endif
//
//#import "GJQueue.h"
//
////template<class T>
////GJQueue<T>::GJQueue(){
////    _inPointer = 0;
////    _outPointer = 0;
////    _mutexInit();
////    
////    popCopyBlock = NULL;
////    pushCopyBlock = NULL;
////    dellocFreeCopyBlock = NULL;
////    
////}
//
//template<class T>
//bool GJQueue<T>::queueCopyPop(T* temBuffer){
//    pthread_mutex_lock(&_uniqueLock);
//    if (_inPointer <= _outPointer) {
//        pthread_mutex_unlock(&_uniqueLock);
//        GJQueueLOG("begin Wait in ----------\n");
//        _mutexWait(&_inCond);
//        pthread_mutex_lock(&_uniqueLock);
//        
//        GJQueueLOG("after Wait in.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
//    }
//    
//    long temPoint = _outPointer;
//    _outPointer++;
//    _mutexSignal(&_outCond);
//    GJQueueLOG("after signal out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
//    pthread_mutex_unlock(&_uniqueLock);
//    
//    if (popCopyBlock != NULL) {
//        popCopyBlock(&buffer[temPoint%ITEM_MAX_COUNT]);
//    }else{
//        *temBuffer = &buffer[temPoint%ITEM_MAX_COUNT];
//    }
//    return true;
//}
//
//template<class T>
//bool GJQueue<T>::queueCopyPush(T* temBuffer){
//  
//    pthread_mutex_lock(&_uniqueLock);
//    if ((_inPointer % ITEM_MAX_COUNT == _outPointer % ITEM_MAX_COUNT && _inPointer > _outPointer)) {
//        pthread_mutex_unlock(&_uniqueLock);
//
//        GJQueueLOG("begin Wait out ----------\n");
//        
//        _mutexWait(&_outCond);
//        pthread_mutex_lock(&_uniqueLock);
//        GJQueueLOG("after Wait out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
//    }
//    long temInPointer = _inPointer;
//    _inPointer++;
//    _mutexSignal(&_inCond);
//    GJQueueLOG("after signal in. incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
//    pthread_mutex_unlock(&_uniqueLock);
//    if (pushCopyBlock != NULL) {
//        pushCopyBlock(&buffer[temInPointer%ITEM_MAX_COUNT]);
//    }else{
//        buffer[temInPointer%ITEM_MAX_COUNT] = *temBuffer;
//    }
//    return true;
//}
//
//template<class T>
//bool  GJQueue<T>::queueRetainPop(T** temBuffer){
//    pthread_mutex_lock(&_uniqueLock);
//    if (_inPointer <= _outPointer) {
//        pthread_mutex_unlock(&_uniqueLock);
//        GJQueueLOG("begin Wait in ----------\n");
//        _mutexWait(&_inCond);
//        pthread_mutex_lock(&_uniqueLock);
//        
//        GJQueueLOG("after Wait in.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
//    }
//    
//    *temBuffer = &buffer[_outPointer%ITEM_MAX_COUNT];
//    _outPointer++;
//    _mutexSignal(&_outCond);
//    GJQueueLOG("after signal out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
//    pthread_mutex_unlock(&_uniqueLock);
//    return true;
//}
//
//template<class T>
//bool  GJQueue<T>::queueRetainPush(T* temBuffer){
//
//    pthread_mutex_lock(&_uniqueLock);
//    if ((_inPointer % ITEM_MAX_COUNT == _outPointer % ITEM_MAX_COUNT && _inPointer > _outPointer)) {
//        pthread_mutex_unlock(&_uniqueLock);
//        
//        GJQueueLOG("begin Wait out ----------\n");
//        
//        _mutexWait(&_outCond);
//        pthread_mutex_lock(&_uniqueLock);
//        GJQueueLOG("after Wait out.  incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
//    }
//    buffer[_inPointer%ITEM_MAX_COUNT] = temBuffer;
//    _inPointer++;
//    
//    _mutexSignal(&_inCond);
//    GJQueueLOG("after signal in. incount:%ld  outcount:%ld----------\n",_inPointer,_outPointer);
//    pthread_mutex_unlock(&_uniqueLock);
//    return true;
//}
//
////template<class T>
////void  GJQueue<T>::_mutexInit()
////{
////    pthread_mutex_init(&_mutex, NULL);
////    pthread_cond_init(&_inCond, NULL);
////    pthread_cond_init(&_outCond, NULL);
////    
////    pthread_mutex_init(&_uniqueLock, NULL);
////    return;
////}
//
////template<class T>
////void GJQueue<T>::_mutexDestory()
////{
////    pthread_mutex_destroy(&_mutex);
////    pthread_cond_destroy(&_inCond);
////    pthread_cond_destroy(&_outCond);
////    pthread_mutex_destroy(&_uniqueLock);
////    return;
////}
//
////template<class T>
////void GJQueue<T>::_mutexWait(pthread_cond_t* _cond)
////{
////    pthread_mutex_lock(&_mutex);
////    pthread_cond_wait(_cond, &_mutex);
////    pthread_mutex_unlock(&_mutex);
////    return;
////}
////
////template<class T>
////void GJQueue<T>::_mutexSignal(pthread_cond_t* _cond)
////{
////    pthread_mutex_lock(&_mutex);
////    pthread_cond_signal(_cond);
////    pthread_mutexunlock(&_mutex);
////    return;
////}
//
////template<class T>
////GJQueue<T>::~GJQueue(){
////    _mutexDestory();
////    dellocFreeCopyBlock(buffer,ITEM_MAX_COUNT);
////}