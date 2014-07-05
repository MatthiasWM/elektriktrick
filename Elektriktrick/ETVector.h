//
//  ETVector.h
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Electrictrick__ETVector__
#define __Electrictrick__ETVector__

#include <iostream>


class ETVector
{
public:
    ETVector();
    void set(const ETVector &v);
    void set(float, float, float);
    void zero();
    void mul(float v);
    void sub(const ETVector &v);
    void add(const ETVector &v);
    float dot(const ETVector &v) const;
    void swapWith(ETVector &v);
    void min(const ETVector &v);
    void max(const ETVector &v);
    void normalize();
    float length() const;
    int isFinite() const;
    void fixFinite();
    float x, y, z;
};


#endif /* defined(__Electrictrick__ETVector__) */
