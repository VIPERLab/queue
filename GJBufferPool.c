//
//  GJBufferPool.c
//  GJQueue
//
//  Created by 未成年大叔 on 16/12/28.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//

#include "GJBufferPool.h"
#include <stdlib.h>
PoolBuffer* newPoolBuffer(int size){
    PoolBuffer* b = (PoolBuffer*)malloc(sizeof(PoolBuffer));
    b->data = malloc(size);
    if (!b->data) {
        free(b);
        return NULL;
    }
    b->size = b->capacity = size;
    return b;
}
void freePoolBuffer(PoolBuffer* b){
    if (b) {
        if (b->data) {
            free(b->data);
        }
        free(b);
    }
}



int GJBufferPoolCreate(GJBufferPool** pool){
    GJBufferPool* p = (GJBufferPool*)malloc(sizeof(GJBufferPool));
    if (!p || queueCreate(&p->queue, 5) < 0){
        return -1;
    }
    *pool = p;
    return 0;
};
GJBufferPool* defauleBufferPool(){
    static GJBufferPool* _defaultPool = NULL;
    if (_defaultPool == NULL) {
       _defaultPool = (GJBufferPool*)malloc(sizeof(GJBufferPool));
    }
    return _defaultPool;
}
int GJBufferPoolRelease(GJBufferPool** pool){
    if (!pool || !*pool) {
        return 0;
    }
    GJBufferPool* p = *pool;
    queueRelease(&p->queue);
    return 0;
};

void* GJBufferPoolGetData(GJBufferPool* p,int size){
    void* data;
    if (queuePop(p->queue, &data, 0)) {
        if ( *(int*)data < size) {
            free(data);
            data = malloc(size + sizeof(int));
            *(int*)data = size;
        }
    }else{
        data = malloc(size + sizeof(int));
        *(int*)data = size;
    }
    return (int*)data + 1;
}
int GJBufferPoolSetData(GJBufferPool* p,void* data){
    return queuePush(p->queue, (int*)data - 1, 0);
}
