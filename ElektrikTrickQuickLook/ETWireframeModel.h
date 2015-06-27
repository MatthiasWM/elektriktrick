//
//  ETWireframeModel.h
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Electrictrick__ETWireframeModel__
#define __Electrictrick__ETWireframeModel__

#include "ETModel.h"

class ETEdge;

class ETWireframeModel : public ETModel
{
public:
    ETWireframeModel();
    virtual ~ETWireframeModel();
    virtual void Draw(void*, int, int);
    virtual void PrepareDrawing();
    virtual void FindBoundingBox();
    virtual int FixupCoordinates();
    virtual int SimpleProjection();
    
    void SimpleProjection(ETVector &v) { ETModel::SimpleProjection(v); }
    int DepthSort();
    static int CompareEdgeZ(const void *a, const void *b);
    
    ETEdge *NewEdge();
    void NewArc(ETVector &p, float r, float a0, float a1);
    void NewSegment(ETVector &p0, ETVector &p1, float bulge);
    void NewSplineSegment(ETVector &p0, ETVector &p1);
    void NewLastSplineSegment(ETVector &p0, ETVector &p1);
    
    ETEdge *edge;
    uint32_t nEdge;
    uint32_t NEdge;
    ETEdge **sortedEdge;
};

#endif /* defined(__Electrictrick__ETWireframeModel__) */
