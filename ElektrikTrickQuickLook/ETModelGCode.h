//
//  ETModelGCode.h
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Electrictrick__ETModelGCode__
#define __Electrictrick__ETModelGCode__

#include "ETModel.h"

class ETModelGCode : public ETModel
{
public:
    ETModelGCode();
    virtual ~ETModelGCode();
    virtual int Load(const char *filename);
    virtual void Draw(void*, int, int);
    virtual void PrepareDrawing();
    virtual void FindBoundingBox();
    virtual int FixupCoordinates();
    virtual int SimpleProjection();
};

#endif /* defined(__Electrictrick__ETModelGCode__) */
