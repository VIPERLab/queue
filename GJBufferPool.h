//
//  GJBufferPool.h
//  GJQueue
//
//  Created by 未成年大叔 on 16/12/28.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//

#ifndef GJBufferPool_h
#define GJBufferPool_h
#include "GJQueue.h"
#include <stdio.h>
typedef struct _PoolBuffer{
    void* data;
    int size;
    int capacity;
}PoolBuffer;

PoolBuffer* newPoolBuffer(int size);
void freePoolBuffer(PoolBuffer* b);

typedef struct _GJBufferPool{
    GJQueue* queue;
}GJBufferPool;
int GJBufferPoolCreate(GJBufferPool** pool);
GJBufferPool* defauleBufferPool();
int GJBufferPoolRelease(GJBufferPool** pool);

void* GJBufferPoolGetData(GJBufferPool* p,int size);
int GJBufferPoolSetData(GJBufferPool* p,void* data);
#endif /* GJBufferPool_h */
