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
    virtual ~ETModelSTL();
    virtual void Prepare2DDrawing();
    virtual void Prepare3DDrawing();
    
    virtual void FindBoundingBox();
    virtual int FixupCoordinates();
    virtual int SimpleProjection();
    
    void SimpleProjection(ETVector &v) { ETModel::SimpleProjection(v); }
    
    int DepthSort();
    static int CompareTriZ(const void *a, const void *b);
    int GenerateFaceNormals();

#if ET_USE_CG
    virtual void CGDraw2D(void*, int, int);
#endif
#if ET_USE_GL
    virtual void GLDraw3D();
    virtual void GLDraw2D(int, int);
#endif

    ETTriangle *tri;
    uint32_t nTri;
    uint32_t NTri;
    ETTriangle **sortedTri;
    uint32_t nSortedTri;

};

#endif /* defined(__Electrictrick__ETModelSTL__) */
