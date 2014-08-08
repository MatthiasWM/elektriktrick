//
//  ETModelTextSTL.h
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Electrictrick__ETModelTextSTL__
#define __Electrictrick__ETModelTextSTL__

#include "ETModelSTL.h"

class ETModelTextSTL : public ETModelSTL
{
public:
    static ETModel *Create(uint8_t *buf, size_t size);
public:
    ETModelTextSTL();
    virtual ~ETModelTextSTL();
    virtual int Load();
};

#endif /* defined(__Electrictrick__ETModelTextSTL__) */
