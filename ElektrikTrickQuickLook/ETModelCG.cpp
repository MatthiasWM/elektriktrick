//
//  ETModelCG.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETModelCG.h"
#include "ETTriangle.h"
#include "ETVector.h"


ETModelCG::ETModelCG() : ETModel()
{
}

/**
 * Draw all triangles in the model starting at the furthest triangle all the way to the closest one.
 */
void ETModelCG::Draw(void* ctx, int width, int height)
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


void ETModelCG::PrepareDrawing()
{
  // prepare the polygon data for rendering
  FindBoundingBox();
  FixupCoordinates();
  SimpleProjection();
  GenerateFaceNormals();
  DepthSort();
}


/**
 * Roate a vector slightly around y and x to give an orthogonal view from the top right.
 */
void ETModelCG::SimpleProjection(ETVector &v)
{
  const float ry = 180.0+30.0; // deg
  float rys = sinf(ry/180.0*M_PI);
  float ryc = cosf(ry/180.0*M_PI);
  
  const float rx = 30.0; // deg
  float rxs = sinf(rx/180.0*M_PI);
  float rxc = cosf(rx/180.0*M_PI);
  
  float x = -v.x;
  float y = v.z;
  float z = -v.y;
  
  // rotate around the vertical a little to the left
  v.x = x*ryc + z*rys;
  v.y = y;
  v.z = z*ryc - x*rys;
  x = v.x; y = v.y; z = v.z;
  
  // now rotate around the x axis a bit so we see a depth
  v.x = x;
  v.y = y*rxc + z*rxs;
  v.z = z*rxc - y*rxs;
  
  // let's keep it at that, so that we have an orthagonal projection
}


/**
 * Rotate the model slightly around y and x to give an orthogonal view from the top right.
 */
int ETModelCG::SimpleProjection()
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
int ETModelCG::DepthSort()
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
int ETModelCG::CompareTriZ(const void *a, const void *b)
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


int ETModelCG::FixupCoordinates()
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



