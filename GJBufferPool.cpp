//
//  GJBufferPool.c
//  GJQueue
//
//  Created by 未成年大叔 on 16/11/7.
//  Copyright © 2016年 MinorUncle. All rights reserved.
//

#include "GJBufferPool.h"
bool GJQueueBuffer::resizeCapture(long caputre){
    if(caputre<0 || caputre < _length)return false;
    
    uint8_t* temp = (uint8_t*)malloc(caputre);
    if(temp == NULL)return false;
    memcpy(temp, _data, _length);
    free(_data);
    _data=temp;
    _caputureSize=caputre;
    return true;
}
