//
//  ETModel.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETModel.h"

#include "ETTriangle.h"
#include "ETVector.h"

#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


/** 
 * Create an empty 3D model.
 */
ETModel::ETModel() :
  pFilename(0L),
  pFile(0),
  pFilesize(0)
{
  tri = 0L;
  nTri = 0;
  NTri = 0;
  sortedTri = 0L;
  nSortedTri = 0;
}


/**
 * Delet a model and all datasets asssociated with it.
 */
ETModel::~ETModel()
{
  if (tri)
    free(tri);
  if (sortedTri)
    free(sortedTri);
  if (pFilename)
    free(pFilename);
  if (pFile)
    fclose(pFile);
}


/**
 * Find the first occurence of an ASCII string inside a text.
 *
 * Used for text based STL files.
 */
int ETModel::Find(ETString &src, const char *key)
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
  if (c=='\r' || c=='\n' || c==0) {
    src = src + n + 1;
    return 1;
  }
  return 0;
}


/**
 * This function fixes fgets() to be aware of alterntive line endings.
 *
 * After reading thousands of STL files, I have seen it all :-( . Mmost have 
 * the Posix '\n' line ending, but some have "\r\n", especially when they
 * were loaded into a MSWindows text editor. I also found output from 
 * "SolidWorks" that uses the MacOS '\r'-only ending from the 90'ies.
 *
 * This function strips the line ending and fixes the seek() position if the 
 * line ending was not found by fgets().
 */
char* ETModel::FGetS(char *dst, int size)
{
  off_t pos = ftell(pFile);
  char *str = fgets(dst, size, pFile);
  if (str) {
    char *s = str;
    for (;;) {
      char c = *s;
      // check if we reached the end of the file - that's ok
      if (c==0)
        break;
      // check to see if this is a CR character
      if (c=='\r') {
        // in MSDOS, a LF will follow, which is correctly handles by fgets
        if (s[1]=='\n' && s[2]==0) {
          *s = 0;
          break;
        }
        // or, if this was the last character in the file, we are fine, too
        if (s[1]==0) {
          *s = 0;
          break;
        }
        // oops, it's the old MacOS CR only style! Go ahead and fix the file
        // position to point to the character following the CR
        *s = 0;
        fseek(pFile, pos + s-str+1, SEEK_SET);
        break;
      }
      // this is the standard Unix line ending
      if (c=='\n') {
        // this should only happen if fgets() is broken. Fix it.
        if (s[1]!=0) {
          *s = 0;
          fseek(pFile, pos + s-str+1, SEEK_SET);
          break;
        }
        *s = 0;
        break;
      }
      s++;
    }
  }
  return str;
}


/**
 * feof() function for alternative file reader.
 */
int ETModel::FEof()
{
  return feof(pFile);
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
int ETModel::LoadTextSTL()
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
  
  return 0;
}


/**
 * Load a binary STL file.
 *
 * FIXME: file may be truncated! Make sure we don;t read beyond EOF.
 * FIXME: file may conatin NaN's. Make sure to weed those out as they may cause exceptions and slow us down tremendously.
 * FIXME: make sure that bytes are swapped everywhere for big endian machines (file format is little endian):
 *          CFSwapInt32LittleToHost(), CFConvertFloat64SwappedToHost()
 */
int ETModel::LoadBinarySTL()
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
  return 0;
}


/**
 * Find the bounding box and reposition coordinates to fit into a -1/1 box.
 */
void ETModel::FindBoundingBox()
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

int ETModel::FixupCoordinates()
{
  return 0;
}


/**
 * Calculate all face normals from scratch.
 *
 * This function uses the direction of the triangle coordinate system. It does
 * not verify or fix flipped normals.
 */
int ETModel::GenerateFaceNormals()
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


/**
 * Check if the open STL file is in binary or text format.
 *
 * This is not as easy as it seem. Although there are rules for recognizing
 * a text file (it should start with the string "solid") there are many export
 * scripts out there that do not comply. 
 * 
 * Our strategy is to check a list of indicators until we have reliable
 * information.
 */
int ETModel::IsBinary()
{
  // check if there is an open file descriptor
  if (!pFile)
    return -1;
  
  // read a sufficient amount of data for guessing the file type
  if (pFilesize<84) // a file that is smaller than 84 bytes can not contain an STL model
    return -1;
  uint8_t buf[84];
  fseek(pFile, 0, SEEK_SET);
  fread(buf, 84, 1, pFile);
  fseek(pFile, 0, SEEK_SET);
  
  // first test: is there a 0 or a non-ASCII character?
  // Those would not appear in a text file.
  int i;
  for (i=0; i<84; i++) {
    if (buf[i]==0 || buf[i]>127)
      return 1;
  }
  
  // actually, I have no other smart test at thispoint in time.
  // So let's assume that this is a text file
  return 0;
}


void ETModel::Draw(void*, int, int)
{
}


/**
 * Load an STL model from a file and prepare it for rendering.
 */
int ETModel::Load(const char *filename)
{
  // make sure taht there is a filename
  if (!filename || !*filename) {
    // ERROR, no filename given
    return 0;
  }
  pFilename = strdup(filename);

  // get the status of the given file. We need the file size.
  struct stat st;
  int ret = stat(filename, &st);
  if (ret==-1) {
    // ERROR, can't get file status
    return 0;
  }
  pFilesize = st.st_size;
  
  // open the file for reading in a buffered file descriptor
  pFile = fopen(filename, "rb");
  if (!pFile) {
    // ERROR, can't open file for reading
    return 0;
  }
  
  // read the file content into memory
  ret = IsBinary();
  if (ret==-1) {
    // ERROR, can't determine the file type (too short, etc.)
    return 0;
  }
  if (ret==1) {
    LoadBinarySTL();
  } else {
    LoadTextSTL();
  }
  fclose(pFile);
  pFile = 0L;
  
  return 1;
}


void ETModel::PrepareDrawing()
{
  // prepare the polygon data for rendering
  FindBoundingBox();
  FixupCoordinates();
  GenerateFaceNormals();
}

