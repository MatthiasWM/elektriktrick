//
//  ETModelGCode.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETModelGCode.h"
#include "ETTriangle.h"
#include "ETVector.h"


ETModelGCode::ETModelGCode() : ETModel()
{
}


ETModelGCode::~ETModelGCode()
{
}


int ETModelGCode::Load(const char *filename)
{
//    G1 X-0.55 Y20.0 Z0.2 F840.0
    fprintf(stderr, "int ETModelGCode::Load(const char *filename)\n");
    return 1;
}


/**
 * Draw all triangles in the model starting at the furthest triangle all the way to the closest one.
 */
void ETModelGCode::Draw(void* ctx, int width, int height)
{
//    CGContextRef cgContext = (CGContextRef)ctx;
//    int xoff = width/2;
//    int yoff = height/2;
//    int xscl = height*0.42; // yes, that is "height", assuming that height is smaller than width
//    int yscl = height*0.42;
//    uint32_t i;
//    for (i=0; i<nSortedTri; ++i) {
//        ETTriangle *t = sortedTri[i];
//        float lum = (sinf((t->n.x+1.2f)*0.5f*M_PI)*0.5f+0.5f) * (sinf((t->n.y+1.4f)*0.5f*M_PI)*0.5f+0.5f);
//        float lumStroke = lum * 0.75f;
//        CGContextSetRGBFillColor(cgContext, lum, lum, lum, 0.8);
//        CGContextSetRGBStrokeColor(cgContext, lumStroke, lumStroke, lumStroke, 0.9);
//        CGContextBeginPath(cgContext);
//        CGContextMoveToPoint(cgContext, t->p0.x*xscl+xoff, t->p0.y*yscl+yoff);
//        CGContextAddLineToPoint(cgContext, t->p1.x*xscl+xoff, t->p1.y*yscl+yoff);
//        CGContextAddLineToPoint(cgContext, t->p2.x*xscl+xoff, t->p2.y*yscl+yoff);
//        CGContextClosePath(cgContext);
//        CGContextDrawPath(cgContext, kCGPathFillStroke);
//        // TODO: if (QLPreviewRequestIsCancelled(preview)) break;
//    }
}


void ETModelGCode::PrepareDrawing()
{
    // prepare the polygon data for rendering
//    FindBoundingBox();
//    FixupCoordinates();
//    SimpleProjection();
//    GenerateFaceNormals();
//    DepthSort();
}


/**
 * Rotate the model slightly around y and x to give an orthogonal view from the top right.
 */
int ETModelGCode::SimpleProjection()
{
//    uint32_t i;
//    for (i=0; i<nTri; ++i) {
//        ETTriangle *t = tri+i;
//        SimpleProjection(t->p0);
//        SimpleProjection(t->p1);
//        SimpleProjection(t->p2);
//    }
    return 0;
}



int ETModelGCode::FixupCoordinates()
{
//    uint32_t i;
//    
//    // find object center and size
//    dx = 0.5f*(pBBoxMax.x-pBBoxMin.x);
//    dy = 0.5f*(pBBoxMax.y-pBBoxMin.y);
//    dz = 0.5f*(pBBoxMax.z-pBBoxMin.z);
//    cx = pBBoxMin.x + dx;
//    cy = pBBoxMin.y + dy;
//    cz = pBBoxMin.z + dz;
//    
//    // normalize the object size and position to fit into a -1 to 1 cube
//    float scl = dx; if (dy>scl) scl = dy; if (dz>scl) scl = dz;
//    if (scl>0.0) scl = 1.0f/scl;
//    for (i=0; i<nTri; ++i) {
//        ETTriangle *t = tri+i;
//        t->p0.x = (t->p0.x - cx) * scl;
//        t->p0.y = (t->p0.y - cy) * scl;
//        t->p0.z = (t->p0.z - cz) * scl;
//        t->p1.x = (t->p1.x - cx) * scl;
//        t->p1.y = (t->p1.y - cy) * scl;
//        t->p1.z = (t->p1.z - cz) * scl;
//        t->p2.x = (t->p2.x - cx) * scl;
//        t->p2.y = (t->p2.y - cy) * scl;
//        t->p2.z = (t->p2.z - cz) * scl;
//    }
//    
    return 0;
}


/**
 * Find the bounding box and reposition coordinates to fit into a -1/1 box.
 */
void ETModelGCode::FindBoundingBox()
{
//    uint32_t i;
    float minX=0, minY=0, minZ=0, maxX=0, maxY=0, maxZ=0;
    
//    for (i=0; i<nTri; ++i) {
//        ETTriangle *t = tri+i;
//        if (i==0) {
//            minX = maxX = t->p0.x;
//            minY = maxY = t->p0.y;
//            minZ = maxZ = t->p0.z;
//        }
//        // update the boundign box
//        if (t->p0.x<minX) minX = t->p0.x; if (t->p0.x>maxX) maxX = t->p0.x;
//        if (t->p0.y<minY) minY = t->p0.y; if (t->p0.y>maxY) maxY = t->p0.y;
//        if (t->p0.z<minZ) minZ = t->p0.z; if (t->p0.z>maxZ) maxZ = t->p0.z;
//        if (t->p1.x<minX) minX = t->p1.x; if (t->p1.x>maxX) maxX = t->p1.x;
//        if (t->p1.y<minY) minY = t->p1.y; if (t->p1.y>maxY) maxY = t->p1.y;
//        if (t->p1.z<minZ) minZ = t->p1.z; if (t->p1.z>maxZ) maxZ = t->p1.z;
//        if (t->p2.x<minX) minX = t->p2.x; if (t->p2.x>maxX) maxX = t->p2.x;
//        if (t->p2.y<minY) minY = t->p2.y; if (t->p2.y>maxY) maxY = t->p2.y;
//        if (t->p2.z<minZ) minZ = t->p2.z; if (t->p2.z>maxZ) maxZ = t->p2.z;
//    }
    pBBoxMin.set(minX, minY, minZ);
    pBBoxMax.set(maxX, maxY, maxZ);
}


