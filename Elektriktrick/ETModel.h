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

#include <vector>
#include <map>

#include <stdlib.h>
#include <stdio.h>

#include "ETVector.h"

class ETTriangle;
class ETVertex;
class ETEdge;

typedef char *ETString;

typedef std::vector<ETTriangle*> ETTriangleList;
typedef std::multimap<float, ETVertex*> ETVertexList;
typedef std::multimap<float, ETVertex*>::iterator ETVertextIterator;
typedef std::multimap<float, ETEdge*> ETEdgeList;
typedef std::multimap<float, ETEdge*>::iterator ETEdgeIterator;

class ETModel
{
public:
    ETModel();
    ~ETModel();
    void FindBoundingBox();
    int FixupCoordinates();
    int GenerateFaceNormals();
    int GenerateVertexNormals();
    void Draw(void*, int, int);
    void PrepareDrawing();
    ETVertex *findOrAddVertex(const ETVector &pIn);
    void createEdgeList();
    ETEdge *findOrAddEdge(ETVertex *v0, ETVertex *v1, ETTriangle *t);
    int verifyIntegrity();
    int SaveAs();

    void calibrate(float xGrow, float yGrow, float zGrow, float xScale=1.0, float yScale=1.0, float zScale=1.0);

//private:
    ETTriangleList pTriList;
    ETVertexList pVertexList;
    ETEdgeList pEdgeList;

    float cx, cy, cz, dx, dy, dz;
    // Posix File Interface
    ETVector pBBoxMin;
    ETVector pBBoxMax;
};


#endif /* defined(__Electrictrick__ETModel__) */
