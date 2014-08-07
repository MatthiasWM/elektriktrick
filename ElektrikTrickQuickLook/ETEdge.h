//
//  ETEdge.h
//  Elektriktrick
//
//  Created by Matthias Melcher on 8/7/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Elektriktrick__ETEdge__
#define __Elektriktrick__ETEdge__

#include "ETVector.h"

class ETEdge
{
public:
    ETVector p0, p1;
    uint16_t attr;
    float z;
};

#endif /* defined(__Elektriktrick__ETEdge__) */
