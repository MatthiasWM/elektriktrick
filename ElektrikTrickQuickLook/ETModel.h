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
#include <stdlib.h>
#include <stdio.h>

#include "ETVector.h"

class ETTriangle;

typedef char *ETString;

class ETModel
{
public:
  ETModel();
  virtual ~ETModel();
  int Find(ETString &src, const char *key);
  int LoadTextSTL();
  int LoadBinarySTL();
  void FindBoundingBox();
  virtual int FixupCoordinates();
  int IsBinary();
  int GenerateFaceNormals();
  virtual void Draw(void*, int, int);
  virtual void PrepareDrawing();
  int Load(const char *filename);
  char* FGetS(char *dst, int size);
  int FEof();
//private:
  ETTriangle *tri;
  uint32_t nTri;
  uint32_t NTri;
  ETTriangle **sortedTri;
  uint32_t nSortedTri;
  float cx, cy, cz, dx, dy, dz;
  // Posix File Interface
  char *pFilename;
  FILE *pFile;
  off_t pFilesize;
  ETVector pBBoxMin;
  ETVector pBBoxMax;
};


#endif /* defined(__Electrictrick__ETModel__) */
