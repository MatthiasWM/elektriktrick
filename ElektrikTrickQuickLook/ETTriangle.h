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


class ETTriangle
{
public:
  ETVector n;
  ETVector p0, p1, p2;
  uint16_t attr;
  float z;
};


#endif /* defined(__Electrictrick__ETTriangle__) */
