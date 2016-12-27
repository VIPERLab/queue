//
//  GJBufferPool.c
//  GJQueue
//
//  Created by 未成年大叔 on 16/11/7.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//

#include "GJBufferPool+cplus.h"
GJBuffer::GJBuffer(int8_t* bufferData,int bufferSize){
    static int i = 0;
    printf("GJBuffer count:%d\n",++i);
    data = bufferData;
    size = _capacity = bufferSize;
}
GJBuffer::GJBuffer(){
    data = NULL;
    size = _capacity = 0;
}
int GJBuffer::capacity(){
    return _capacity;
}

GJBufferPool::GJBufferPool(){
    _cacheQueue.autoResize = true;
}
GJBufferPool::~GJBufferPool(){
    cleanBuffer();
}
GJBufferPool* GJBufferPool::defaultBufferPool()
{
    static GJBufferPool* _defaultPool = new GJBufferPool();
    return _defaultPool;
}

GJBuffer* GJBufferPool::getBuffer(int size){
    static int mc = 0;
    GJBuffer* buffer = NULL;
    if(_cacheQueue.queuePop(&buffer,0)) {
        if (buffer->_capacity < size) {
            free(buffer->data);
            buffer->data = (int8_t*)malloc(size);
            printf("malloc GJBuffer0 count:%d\n",++mc);
            
            buffer->size = buffer->_capacity = size;
        }else{
            buffer->size = size;
        }
    }
    if (!buffer) {
        buffer = new GJBuffer((int8_t*)malloc(size),size);
        printf("malloc GJBuffer count:%d\n",++mc);
    }
    return buffer;
}

void GJBufferPool::setBuffer(GJBuffer* buffer){
    _cacheQueue.queuePush(buffer,0);
}
void GJBufferPool::cleanBuffer(){
    GJBuffer* buffer;
    while (_cacheQueue.queuePop(&buffer)) {
        free(buffer->data);
        free(buffer);
    }
    _cacheQueue.clean();
}

