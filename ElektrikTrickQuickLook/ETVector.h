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
    void set(const ETVector &v);
    void set(float, float, float);
    void sub(const ETVector &v);
    void add(const ETVector &v);
    void mul(float v);
    void cross(const ETVector &v);
    float dot(const ETVector &v) const;
    void swapWith(ETVector &v);
    void normalize();
    float length() const;
    int isFinite() const;
    void fixFinite();
    float angle2d() const;
    float angleTo2d(const ETVector& v) const;
    void setPolar2d(float radius, float angle);
    float x, y, z;
};


#endif /* defined(__Electrictrick__ETVector__) */
