//
//  ETModelDXF.h
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Electrictrick__ETModelDXF__
#define __Electrictrick__ETModelDXF__

#include "ETWireframeModel.h"

class ETEdge;

class ETModelDXF : public ETWireframeModel
{
public:
    static ETModel *Create(uint8_t *buf, size_t size);
public:
    ETModelDXF();
    virtual ~ETModelDXF();
    virtual int Load();
    
    int NextCode(int &code, char *data);
};

#endif /* defined(__Electrictrick__ETModelDXF__) */
