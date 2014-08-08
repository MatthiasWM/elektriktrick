//
//  ETModelBinarySTL.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETModelBinarySTL.h"
#include "ETTriangle.h"
#include "ETVector.h"

#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


ETModel *ETModelBinarySTL::Create(uint8_t *buf, size_t size)
{
    if (size<84)
        return 0L;
    int i;
    for (i=0; i<84; i++) {
        if (buf[i]==0 || buf[i]>127)
            return new ETModelBinarySTL();
    }
    return 0L;
}


ETModelBinarySTL::ETModelBinarySTL() : ETModelSTL()
{
}


ETModelBinarySTL::~ETModelBinarySTL()
{
}


/**
 * Load a binary STL file.
 *
 * FIXME: file may be truncated! Make sure we don;t read beyond EOF.
 * FIXME: file may conatin NaN's. Make sure to weed those out as they may cause exceptions and slow us down tremendously.
 * FIXME: make sure that bytes are swapped everywhere for big endian machines (file format is little endian):
 *          CFSwapInt32LittleToHost(), CFConvertFloat64SwappedToHost()
 */
int ETModelBinarySTL::Load()
{
    fseek(pFile, 80, SEEK_SET);
    
    // number of triangles
    uint32_t i, nTriFileSize=0, nTriFileInfo=0;
    // get the number of triangles in the file, as claimed by the numTri fied
    fread(&nTriFileInfo, 1, 4, pFile);
    // get the maximum number of readble triangles based on the file size
    nTriFileSize = ((uint32_t)pFilesize-84) / 50;
    if (nTriFileSize < nTriFileInfo) {
        // the file may have been truncated
        nTri = nTriFileSize;
    } else {
        nTri = nTriFileInfo;
    }
    
    //  fprintf(stderr, "We have %d triangles\n", nTri);
    tri = (ETTriangle*)calloc(nTri, sizeof(ETTriangle));
    if (!tri) return -1;
    
    for (i=0; i<nTri; ++i) {
        ETTriangle *t = tri+i;
        fread(t, 50, 1, pFile);
        t->n.fixFinite();
        t->p0.fixFinite();
        t->p1.fixFinite();
        t->p2.fixFinite();
    }
    return 1;
}

