//
//  ETModelTextSTL.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETModelTextSTL.h"
#include "ETTriangle.h"
#include "ETVector.h"

#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


ETModel *ETModelTextSTL::Create(uint8_t *buf, size_t size)
{
    if (size<5)
        return 0L;
    if (strncmp((char*)buf, "solid", 5)==0)
        return new ETModelTextSTL;
    return 0L;
}


ETModelTextSTL::ETModelTextSTL() : ETModelSTL()
{
}


ETModelTextSTL::~ETModelTextSTL()
{
}


// ASCII FORMAT (ca. 140 characters per triangle)
//              solid name
//                facet normal ni nj nk
//                  outer loop
//                    vertex v1x v1y v1z
//                    vertex v2x v2y v2z
//                    vertex v3x v3y v3z
//                  endloop
//                endfacet
//              endsolid name
// Usually the line deliminator is 0x0A, but I found files that use 0x0D.
// Likely, there are files as well that use 0x0D followed by 0x0A.


/**
 * Text mode STL reader.
 *
 * The reader should be ablet to read some information even when the fil is
 * truncated or otherwise broken without crashing.
 *
 * \returns no error code, it fails silently, but creates an intact (possibly incomplete) dataset
 */
int ETModelTextSTL::Load()
{
    NTri = 2048;
    tri = (ETTriangle*)calloc(NTri, sizeof(ETTriangle));
    
    char buf[1024];
    
    // read "solid name" (we already verified that);
    for (;;) {
        if (FEof()) break;
        FGetS(buf, 1023);
        char *src = buf;
        if (Find(src, "solid")) break;
    }
    
    for (;;) {
        char *src = buf;
        for (;;) {
            if (FEof()) break;
            buf[0] = 0;
            FGetS(buf, 1023); src = buf;
            // read "facet normal ni nj nk"
            if (Find(src, "facet normal")) break;
        }
        if (NTri==nTri) {
            NTri += 2048;
            tri = (ETTriangle*)realloc(tri, NTri*sizeof(ETTriangle));
        }
        ETTriangle &t = tri[nTri];
        if (sscanf(src, "%f %f %f", &t.n.x, &t.n.y, &t.n.z)!=3) break;
        if (!t.n.isFinite()) continue;
        // read "outer loop"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "outer loop")) break;
        // read "vertex x y z"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "vertex")) break;
        if (sscanf(src, "%f %f %f", &t.p0.x, &t.p0.y, &t.p0.z)!=3) break;
        if (!t.p0.isFinite()) continue;
        // read "vertex x y z"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "vertex")) break;
        if (sscanf(src, "%f %f %f", &t.p1.x, &t.p1.y, &t.p1.z)!=3) break;
        if (!t.p1.isFinite()) continue;
        // read "vertex x y z"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "vertex")) break;
        if (sscanf(src, "%f %f %f", &t.p2.x, &t.p2.y, &t.p2.z)!=3) break;
        if (!t.p2.isFinite()) continue;
        // read "outer loop"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "endloop")) break;
        // read "outer loop"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "endfacet")) break;
        // don't increment the counter until we are sure that the triangle is read.
        nTri++;
    }
    return 1;
}

