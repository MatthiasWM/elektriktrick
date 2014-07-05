//
//  ETTriangle.h
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Electrictrick__ETTriangle__
#define __Electrictrick__ETTriangle__

#include "ETVector.h"

class ETVertex;
class ETEdge;


class ETTriangle
{
public:
    ETVector n;
    ETVertex *v0, *v1, *v2;
    ETEdge *e0, *e1, *e2;
    uint16_t attr;
    float z;
};


#endif /* defined(__Electrictrick__ETTriangle__) */
