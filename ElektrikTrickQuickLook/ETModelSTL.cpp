//
//  ETModelSTL.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETModelSTL.h"
#include "ETTriangle.h"
#include "ETVector.h"

#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


ETModelSTL::ETModelSTL() : ETModel()
{
    tri = 0L;
    nTri = 0;
    NTri = 0;
    sortedTri = 0L;
    nSortedTri = 0;
}


ETModelSTL::~ETModelSTL()
{
    if (tri)
        free(tri);
    if (sortedTri)
        free(sortedTri);
}

/**
 * Draw all triangles in the model starting at the furthest triangle all the way to the closest one.
 */
void ETModelSTL::Draw(void* ctx, int width, int height)
{
    CGContextRef cgContext = (CGContextRef)ctx;
    int xoff = width/2;
    int yoff = height/2;
    int xscl = height*0.42; // yes, that is "height", assuming that height is smaller than width
    int yscl = height*0.42;
    uint32_t i;
    for (i=0; i<nSortedTri; ++i) {
        ETTriangle *t = sortedTri[i];
        float lum = (sinf((t->n.x+1.2f)*0.5f*M_PI)*0.5f+0.5f) * (sinf((t->n.y+1.4f)*0.5f*M_PI)*0.5f+0.5f);
        float lumStroke = lum * 0.75f;
        CGContextSetRGBFillColor(cgContext, lum, lum, lum, 0.8);
        CGContextSetRGBStrokeColor(cgContext, lumStroke, lumStroke, lumStroke, 0.9);
        CGContextBeginPath(cgContext);
        CGContextMoveToPoint(cgContext, t->p0.x*xscl+xoff, t->p0.y*yscl+yoff);
        CGContextAddLineToPoint(cgContext, t->p1.x*xscl+xoff, t->p1.y*yscl+yoff);
        CGContextAddLineToPoint(cgContext, t->p2.x*xscl+xoff, t->p2.y*yscl+yoff);
        CGContextClosePath(cgContext);
        CGContextDrawPath(cgContext, kCGPathFillStroke);
        // TODO: if (QLPreviewRequestIsCancelled(preview)) break;
    }
}


void ETModelSTL::PrepareDrawing()
{
    // prepare the polygon data for rendering
    FindBoundingBox();
    FixupCoordinates();
    SimpleProjection();
    GenerateFaceNormals();
    DepthSort();
}


/**
 * Rotate the model slightly around y and x to give an orthogonal view from the top right.
 */
int ETModelSTL::SimpleProjection()
{
    uint32_t i;
    for (i=0; i<nTri; ++i) {
        ETTriangle *t = tri+i;
        SimpleProjection(t->p0);
        SimpleProjection(t->p1);
        SimpleProjection(t->p2);
    }
    return 0;
}


/**
 * Sort all triangles in Z depth to allow rendering overlaps using the painters algorithm.
 *
 * This function creates an array of pointers to the original tri data. All
 * tri's pointing away from us are ignored. All others are sorted by quicksort
 * using an approximation. Results are not perfect, but good enough and fast
 * without requiring a Z-buffer.
 */
int ETModelSTL::DepthSort()
{
    uint32_t i, j=0;
    sortedTri = (ETTriangle**)calloc(nTri, sizeof(ETTriangle*));
    for (i=0; i<nTri; i++) {
        ETTriangle *t = tri+i;
        if (t->n.z>=0.0f) { // handle only front facing triangles
            sortedTri[j++] = t;
            // prepare for depth sorting
#if 0
            //   sort coordinates by z (lowes z in p0, highest z in p2)
            if (t->p2.z < t->p0.z && t->p2.z < t->p1.z) {
                t->p0.swapWith(t->p2);
            } else if (t->p1.z < t->p2.z && t->p1.z < t->p0.z) {
                t->p0.swapWith(t->p1);
            }
            if (t->p1.z > t->p2.z) {
                t->p2.swapWith(t->p1);
            }
#endif
            //   calculate the z midpoint
            t->z = (t->p0.z + t->p1.z + t->p2.z) / 3.0;
        }
    }
    nSortedTri = j;
    
    qsort(sortedTri, nSortedTri, sizeof(ETTriangle*), CompareTriZ);
    
    return 0;
}


/**
 * Compare the z position of triangles and retrun -1, 0, or 1 for close, equal, and further.
 *
 * This is just a rough approximation. It is actually quite difficult to find
 * the correct z order of triangles, as occlusion depends not only on z, but
 * also on x, y, and size of the triangle. There is no perfect solution for
 * cyclic overlapping triangles.
 *
 * If done well, qsort is the wrong approch!
 *
 * For a rough preview however, a few rendering bugs do not bother us. We
 * compare the z midpoint of the give triangles.
 */
int ETModelSTL::CompareTriZ(const void *a, const void *b)
{
    const ETTriangle* ta = *(const ETTriangle**)a;
    const ETTriangle* tb = *(const ETTriangle**)b;
    
    // simple mid point depth comparison
    int v = (ta->z < tb->z) - (ta->z > tb->z);
    return v;
    
#if 0
    // this partial algorithem is 100% correct, but does not solve all possible cases
    if (ta->p0.z > tb->p2.z) return -1; // smallest of a is larger than largets of b
    if (ta->p2.z < tb->p0.z) return  1; // largets of a is smaller than smallest of b
#endif
    
#if 0
    // the following code solves X/Y positioning, but fails for qsort
    if (ta->p0.x < tb->p0.x && ta->p0.x < tb->p1.x && ta->p0.x < tb->p2.x &&
        ta->p1.x < tb->p0.x && ta->p1.x < tb->p1.x && ta->p1.x < tb->p2.x &&
        ta->p2.x < tb->p0.x && ta->p2.x < tb->p1.x && ta->p2.x < tb->p2.x) return 1;
    if (ta->p0.x > tb->p0.x && ta->p0.x > tb->p1.x && ta->p0.x > tb->p2.x &&
        ta->p1.x > tb->p0.x && ta->p1.x > tb->p1.x && ta->p1.x > tb->p2.x &&
        ta->p2.x > tb->p0.x && ta->p2.x > tb->p1.x && ta->p2.x > tb->p2.x) return -1;
    if (ta->p0.y < tb->p0.y && ta->p0.y < tb->p1.y && ta->p0.y < tb->p2.y &&
        ta->p1.y < tb->p0.y && ta->p1.y < tb->p1.y && ta->p1.y < tb->p2.y &&
        ta->p2.y < tb->p0.y && ta->p2.y < tb->p1.y && ta->p2.y < tb->p2.y) return 1;
    if (ta->p0.y > tb->p0.y && ta->p0.y > tb->p1.y && ta->p0.y > tb->p2.y &&
        ta->p1.y > tb->p0.y && ta->p1.y > tb->p1.y && ta->p1.y > tb->p2.y &&
        ta->p2.y > tb->p0.y && ta->p2.y > tb->p1.y && ta->p2.y > tb->p2.y) return -1;
#endif
    
#if 0
    // this is another nice approach, but unfortunatley not correct
    // calculate the angle between the normal of the first face and the delta between one point on each face
    ETVector d;
    // get a vector from surface a to surface b
    d.set(tb->p2);
    d.sub(ta->p1);
    d.normalize();
    // now calculate the dot product between
    float angle = d.dot(ta->n);
    if (angle<0.0) return -1;
    return (angle>0.0);
#endif
    
#if 0
    // alternative code to calculate distance to plane, but that is not the correct solution
    d.set(tb->p0); d.sub(ta->p1); d.normalize(); float a0 = d.dot(ta->n);
    d.set(tb->p1); d.sub(ta->p1); d.normalize(); float a1 = d.dot(ta->n);
    d.set(tb->p2); d.sub(ta->p1); d.normalize(); float a2 = d.dot(ta->n);
    int n = 0;
    if (a0<0.0) n++;
    if (a1<0.0) n++;
    if (a2<0.0) n++;
    if (n<1) return -1;
    return 1;
#endif
    
    // the correct solution for this problem has multiple steps in increasing complexity
    // 1: don't use qsort. Qsort depends on one single correct order of elements. This
    //    problem here includes circular overlapping triangles and other caveats which
    //    throws qsort off
    // 2: compare zdepths and sort non-overlapping triangles
    // 3: for the remaining triangles, create XY bounding boxes.
    //     Non-overlapping tris need not be sorted
    // 4: project all triangles in XY and check if they truly overlap. If not,
    //    there is no need to sort
    // 5: if they do overlap, there is a simple way and a correct way
    // 5a: find any point where the triangles overlap and calculate their z.
    //     Sort by that z. Don;t use points on the edge as triangles may be
    //     sharing that same edge, giving us no usable result.
    // 5b: check if one triangle goes through the other triangle. Subdivide
    //     one triangle until there are no overlaps in X/Y anymore, but instead
    //     aligned triangles. This solution will lead to perfect results in
    //     all cases, but will create a myriad of triangles..
}


int ETModelSTL::FixupCoordinates()
{
    uint32_t i;
    
    // find object center and size
    dx = 0.5f*(pBBoxMax.x-pBBoxMin.x);
    dy = 0.5f*(pBBoxMax.y-pBBoxMin.y);
    dz = 0.5f*(pBBoxMax.z-pBBoxMin.z);
    cx = pBBoxMin.x + dx;
    cy = pBBoxMin.y + dy;
    cz = pBBoxMin.z + dz;
    
    // normalize the object size and position to fit into a -1 to 1 cube
    float scl = dx; if (dy>scl) scl = dy; if (dz>scl) scl = dz;
    if (scl>0.0) scl = 1.0f/scl;
    for (i=0; i<nTri; ++i) {
        ETTriangle *t = tri+i;
        t->p0.x = (t->p0.x - cx) * scl;
        t->p0.y = (t->p0.y - cy) * scl;
        t->p0.z = (t->p0.z - cz) * scl;
        t->p1.x = (t->p1.x - cx) * scl;
        t->p1.y = (t->p1.y - cy) * scl;
        t->p1.z = (t->p1.z - cz) * scl;
        t->p2.x = (t->p2.x - cx) * scl;
        t->p2.y = (t->p2.y - cy) * scl;
        t->p2.z = (t->p2.z - cz) * scl;
    }
    
    return 0;
}


/**
 * Find the bounding box and reposition coordinates to fit into a -1/1 box.
 */
void ETModelSTL::FindBoundingBox()
{
    uint32_t i;
    float minX=0, minY=0, minZ=0, maxX=0, maxY=0, maxZ=0;
    
    for (i=0; i<nTri; ++i) {
        ETTriangle *t = tri+i;
        if (i==0) {
            minX = maxX = t->p0.x;
            minY = maxY = t->p0.y;
            minZ = maxZ = t->p0.z;
        }
        // update the boundign box
        if (t->p0.x<minX) minX = t->p0.x; if (t->p0.x>maxX) maxX = t->p0.x;
        if (t->p0.y<minY) minY = t->p0.y; if (t->p0.y>maxY) maxY = t->p0.y;
        if (t->p0.z<minZ) minZ = t->p0.z; if (t->p0.z>maxZ) maxZ = t->p0.z;
        if (t->p1.x<minX) minX = t->p1.x; if (t->p1.x>maxX) maxX = t->p1.x;
        if (t->p1.y<minY) minY = t->p1.y; if (t->p1.y>maxY) maxY = t->p1.y;
        if (t->p1.z<minZ) minZ = t->p1.z; if (t->p1.z>maxZ) maxZ = t->p1.z;
        if (t->p2.x<minX) minX = t->p2.x; if (t->p2.x>maxX) maxX = t->p2.x;
        if (t->p2.y<minY) minY = t->p2.y; if (t->p2.y>maxY) maxY = t->p2.y;
        if (t->p2.z<minZ) minZ = t->p2.z; if (t->p2.z>maxZ) maxZ = t->p2.z;
    }
    pBBoxMin.set(minX, minY, minZ);
    pBBoxMax.set(maxX, maxY, maxZ);
}


/**
 * Calculate all face normals from scratch.
 *
 * This function uses the direction of the triangle coordinate system. It does
 * not verify or fix flipped normals.
 */
int ETModelSTL::GenerateFaceNormals()
{
    uint32_t i;
    for (i=0; i<nTri; ++i) {
        ETTriangle *t = tri+i;
        
        ETVector u; u.x = t->p1.x-t->p0.x; u.y = t->p1.y-t->p0.y; u.z = t->p1.z-t->p0.z;
        ETVector v; v.x = t->p2.x-t->p0.x; v.y = t->p2.y-t->p0.y; v.z = t->p2.z-t->p0.z;
        t->n.x = (u.y*v.z) - (u.z*v.y);
        t->n.y = (u.z*v.x) - (u.x*v.z);
        t->n.z = (u.x*v.y) - (u.y*v.x);
        t->n.normalize();
    }
    return 0;
}



