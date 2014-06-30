//
//  ETModelCG.h
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Electrictrick__ETModelCG__
#define __Electrictrick__ETModelCG__

#include "ETModel.h"

class ETModelCG : public ETModel
{
public:
  ETModelCG();
  virtual void Draw(void*, int, int);
  virtual void PrepareDrawing();
  void SimpleProjection(ETVector &v);
  virtual int SimpleProjection();
  static int CompareTriZ(const void *a, const void *b);
  virtual int FixupCoordinates();
  int DepthSort();
};

#endif /* defined(__Electrictrick__ETModelCG__) */
