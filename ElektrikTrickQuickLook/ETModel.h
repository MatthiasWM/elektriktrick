//
//  ETModel.h
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Electrictrick__ETModel__
#define __Electrictrick__ETModel__

#include <QuickLook/QuickLook.h>
#include <stdlib.h>
#include <stdio.h>

#include "ETVector.h"

class ETTriangle;

typedef char *ETString;

class ETModel
{
public:
    static ETModel *ModelForFileType(const char *filename);
public:
    ETModel();
    virtual ~ETModel();
    virtual int Load() = 0;
    virtual void Draw(void*, int, int) = 0;
    virtual void PrepareDrawing() = 0;
    virtual void FindBoundingBox() = 0;
    virtual int FixupCoordinates() = 0;
    virtual int SimpleProjection() = 0;
    
    void SimpleProjection(ETVector &v);
    int Find(ETString &src, const char *key);
    char* FGetS(char *dst, int size);
    int FEof();
    
private:
    static bool FileIsBinarySTL(uint8_t *buf, size_t size);
    static bool FileIsTextSTL(uint8_t *buf, size_t size);
    
public: //private:
    float cx, cy, cz, dx, dy, dz;
    // Posix File Interface
    char *pFilename;
    FILE *pFile;
    off_t pFilesize;
    ETVector pBBoxMin;
    ETVector pBBoxMax;
};


#endif /* defined(__Electrictrick__ETModel__) */
