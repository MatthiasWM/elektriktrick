//
//  ETModelBinarySTL.h
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Electrictrick__ETModelBinarySTL__
#define __Electrictrick__ETModelBinarySTL__

#include "ETModelSTL.h"

class ETModelBinarySTL : public ETModelSTL
{
public:
    static ETModel *Create(uint8_t *buf, size_t size);
public:
    ETModelBinarySTL();
    virtual ~ETModelBinarySTL();
    virtual int Load();
};

#endif /* defined(__Electrictrick__ETModelBinarySTL__) */
