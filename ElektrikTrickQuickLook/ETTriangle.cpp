//
//  ETTriangle.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETTriangle.h"

#if 0

typedef struct {
  float x, y, z;
} ETVector;

typedef struct {
  ETVector n;
  ETVector p0, p1, p2;
  uint16_t attr;
  float z;
} ETTriangle;

typedef char *string;

class Dog {
public:
  Dog();
};


ETTriangle *tri = 0L;
uint32_t nTri = 0, NTri = 0;
ETTriangle **sortedTri = 0L;
uint32_t nSortedTri = 0;
float cx, cy, cz, dx, dy, dz;


int ElFind(string &src, const char *key)
{
  for (;;) {
    char c = *src;
    if (c!=' ' && c!='\t')
      break;
    src++;
  }
  size_t n = strlen(key);
  if (strncmp(src, key, n)!=0)
    return 0;
  char c = src[n];
  if (c==' ' || c=='\t') {
    src = src + n + 1;
    return 1;
  }
  if (c=='\r' || c=='\n') {
    src = src + n + 1;
    return 1;
  }
  return 0;
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
int ElLoadTextSTL(FILE *f)
{
  NTri = 2048;
  tri = (ETTriangle*)calloc(NTri, sizeof(ETTriangle));
  
  char buf[1024];
  
  // read "solid name" (we already verified that);
  fgets(buf, 1023, f);
  
  for (;;) {
    char *src;
    if (feof(f)) break;
    buf[0] = 0;
    fgets(buf, 1023, f); src = buf;
    // read "facet normal ni nj nk"
    if (!ElFind(src, "facet normal")) break;
    if (NTri==nTri) {
      NTri += 2048;
      tri = (ETTriangle*)realloc(tri, NTri*sizeof(ETTriangle));
    }
    ETTriangle &t = tri[nTri++];
    if (sscanf(src, "%f %f %f", &t.n.x, &t.n.y, &t.n.z)!=3) break;
    // read "outer loop"
    fgets(buf, 1023, f); src = buf;
    if (!ElFind(src, "outer loop")) break;
    // read "vertex x y z"
    fgets(buf, 1023, f); src = buf;
    if (!ElFind(src, "vertex")) break;
    if (sscanf(src, "%f %f %f", &t.p0.x, &t.p0.y, &t.p0.z)!=3) break;
    // read "vertex x y z"
    fgets(buf, 1023, f); src = buf;
    if (!ElFind(src, "vertex")) break;
    if (sscanf(src, "%f %f %f", &t.p1.x, &t.p1.y, &t.p1.z)!=3) break;
    // read "vertex x y z"
    fgets(buf, 1023, f); src = buf;
    if (!ElFind(src, "vertex")) break;
    if (sscanf(src, "%f %f %f", &t.p2.x, &t.p2.y, &t.p2.z)!=3) break;
    // read "outer loop"
    fgets(buf, 1023, f); src = buf;
    if (!ElFind(src, "endloop")) break;
    // read "outer loop"
    fgets(buf, 1023, f); src = buf;
    if (!ElFind(src, "endfacet")) break;
  }
  
  return 0;
}


int ElLoadBinarySTL(FILE *f)
{
  fseek(f, 80, SEEK_SET);
  
  // number of triangles
  uint32_t i;
  fread(&nTri, 1, 4, f);
  //  fprintf(stderr, "We have %d triangles\n", nTri);
  tri = (ETTriangle*)calloc(nTri, sizeof(ETTriangle));
  
  for (i=0; i<nTri; ++i) {
    ETTriangle *t = tri+i;
    fread(t, 50, 1, f);
  }
  return 0;
}


int ElNormalizeCoordinates()
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
  
  // find object center and size
  dx = 0.5f*(maxX-minX);
  dy = 0.5f*(maxY-minY);
  dz = 0.5f*(maxZ-minZ);
  cx = minX + dx;
  cy = minY + dy;
  cz = minZ + dz;
  
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

void ElProject(ETVector &v)
{
  float x = -v.x;
  float y = v.z;
  float z = -v.y;
  
  //  587 809
  
  // rotate around the vertical a little to the left
  v.x = x*.809f + z*.587f;
  v.y = y;
  v.z = z*.809f - x*.587f;
  x = v.x; y = v.y; z = v.z;
  
  // now rotate around the x axis a bit so we see a depth
  v.x = x;
  v.y = y*.951f + z*.309f;
  v.z = z*.951f - y*.309f;
  
  // let's keep it at that, so that we have an orthagonal projection
}


int ElSimpleProjection()
{
  uint32_t i;
  for (i=0; i<nTri; ++i) {
    ETTriangle *t = tri+i;
    ElProject(t->p0);
    ElProject(t->p1);
    ElProject(t->p2);
  }
  return 0;
}


int ElGenerateFaceNormals()
{
  uint32_t i;
  for (i=0; i<nTri; ++i) {
    ETTriangle *t = tri+i;
    
    ETVector u; u.x = t->p1.x-t->p0.x; u.y = t->p1.y-t->p0.y; u.z = t->p1.z-t->p0.z;
    ETVector v; v.x = t->p2.x-t->p0.x; v.y = t->p2.y-t->p0.y; v.z = t->p2.z-t->p0.z;
    t->n.x = (u.y*v.z) - (u.z*v.y);
    t->n.y = (u.z*v.x) - (u.x*v.z);
    t->n.z = (u.x*v.y) - (u.y*v.x);
    
    float len = sqrtf(t->n.x*t->n.x + t->n.y*t->n.y + t->n.z*t->n.z);
    if (len>0.0f) {
      t->n.x /= len;
      t->n.y /= len;
      t->n.z /= len;
    }
  }
  return 0;
}


int ElCompareTriZ(const void *a, const void *b)
{
  const ETTriangle* ta = *(const ETTriangle**)a;
  const ETTriangle* tb = *(const ETTriangle**)b;
  int v = (ta->z < tb->z) - (ta->z > tb->z);
  return v;
}

int ElDepthSort()
{
  uint32_t i, j=0;
  sortedTri = (ETTriangle**)calloc(nTri, sizeof(ETTriangle*));
  for (i=0; i<nTri; i++) {
    ETTriangle *t = tri+i;
    if (t->n.z>=0.0f) { // handle only front facing triangles
      sortedTri[j++] = t;
      t->z = t->p0.z + t->p1.z + t->p2.z;
    }
  }
  nSortedTri = j;
  
  qsort(sortedTri, nSortedTri, sizeof(ETTriangle*), ElCompareTriZ);
  
  return 0;
}

#endif
