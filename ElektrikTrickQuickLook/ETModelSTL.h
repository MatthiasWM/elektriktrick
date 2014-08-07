//
//  ETModelSTL.h
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Electrictrick__ETModelSTL__
#define __Electrictrick__ETModelSTL__

#include "ETModel.h"

class ETModelSTL : public ETModel
{
public:
    ETModelSTL();
    ~ETModelSTL();
    virtual int Load(const char *filename);
    virtual void Draw(void*, int, int);
    virtual void PrepareDrawing();
    virtual void FindBoundingBox();
    virtual int FixupCoordinates();
    virtual int SimpleProjection();
    
    void SimpleProjection(ETVector &v) { ETModel::SimpleProjection(v); }
    
    int LoadTextSTL();
    int LoadBinarySTL();
    int IsBinary();
    int DepthSort();
    static int CompareTriZ(const void *a, const void *b);
    int GenerateFaceNormals();
    
    ETTriangle *tri;
    uint32_t nTri;
    uint32_t NTri;
    ETTriangle **sortedTri;
    uint32_t nSortedTri;

};

#endif /* defined(__Electrictrick__ETModelSTL__) */
