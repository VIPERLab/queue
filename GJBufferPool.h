//
//  GJBufferPool.h
//  GJQueue
//
//  Created by 未成年大叔 on 16/11/7.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//

#ifndef GJBufferPool_h
#define GJBufferPool_h

#include "GJQueue.h"
#include <string>


typedef struct GJBuffer{
public:
    uint8_t* data(){return _data;};
    long length(){return _length;}
    GJBuffer(long length){if(length<0){_length=0;_data=NULL;return;}; _data=(uint8_t*)malloc(length);_length=length;}
    GJBuffer(){_data=NULL;_length=0;}
protected:
    uint8_t* _data;
    long _length;
    ~GJBuffer(){if(_data)free(_data);}
}GJBuffer;

struct GJPoolBuffer:GJBuffer{
public:
    GJPoolBuffer(long length):GJBuffer(length){_caputureSize=_length;}
    long caputreSize(){return _caputureSize;}
    bool setLength(long length){if(length<=_caputureSize){_length=length;return true;}else return false;}
    bool resizeCapture(long caputre);
private:
    long _caputureSize;//申请空间的大小
};

typedef struct GJPoolBuffer GJPoolBuffer;

typedef struct GJQueuePool{
public:
    GJQueuePool(long suitableBufferSize, int size);
    GJPoolBuffer* get(long size);
    void put(GJPoolBuffer* buffer);
    ~GJQueuePool();
private:
    GJQueue<GJPoolBuffer*> _queue;
    int _numElem;
}GJQueuePool;
#define DEFAULT_POOL_BUFFER_SIZE 10
GJQueuePool::GJQueuePool(long suitableBufferSize, int size){
    _queue.autoResize=true;
    _queue.shouldWait=false;
    _queue.shouldNonatomic=true;
    if (size <= 0) {
        _numElem=0;
        return;
    }
    if (suitableBufferSize<=0) {   suitableBufferSize=10;    }
    for (int i =0; i<size; i++) {
        GJPoolBuffer* buffer = new GJPoolBuffer(suitableBufferSize);
        _queue.queuePush(buffer);
    }
    
};
GJPoolBuffer* GJQueuePool::get(long size){
    GJPoolBuffer* buffer=NULL;
    if (_queue.currentLenth()==0) {
        buffer = new GJPoolBuffer(size);
    }else{
        _queue.queuePop(&buffer);
        if (buffer->caputreSize()<size) {
            buffer->resizeCapture(size);
        }
        buffer->setLength(size);
    }
    return buffer;
};
void GJQueuePool::put(GJPoolBuffer* buffer){
    _queue.queuePush(buffer);
};
GJQueuePool::~GJQueuePool(){
    GJPoolBuffer* buffer;
    while (_queue.queuePop(&buffer)) {
        free(buffer);
    }
}


#endif /* GJBufferPool_h */
