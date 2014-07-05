//
//  ETVector.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETVector.h"

#include <math.h>


ETVector::ETVector()
{
    x = y = z = 0.0;
}

void ETVector::zero()
{
    x = y = z = 0.0;
}

void ETVector::set(const ETVector &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
}

void ETVector::set(float ax, float ay, float az)
{
    x = ax;
    y = ay;
    z = az;
}

void ETVector::sub(const ETVector &v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
}

void ETVector::add(const ETVector &v)
{
    x += v.x;
    y += v.y;
    z += v.z;
}

void ETVector::mul(float v)
{
    x *= v;
    y *= v;
    z *= v;
}

float ETVector::dot(const ETVector &v) const
{
    return x*v.x + y*v.y + z*v.z;
}

void ETVector::swapWith(ETVector &v)
{
    float t;
    t = x; x = v.x; v.x = t;
    t = y; y = v.y; v.y = t;
    t = z; z = v.z; v.z = t;
}

void ETVector::normalize()
{
    float len = sqrtf(x*x + y*y + z*z);
    if (len>0.0f) {
        x /= len;
        y /= len;
        z /= len;
    }
}

float ETVector::length() const
{
    return sqrtf(x*x + y*y + z*z);
}

int ETVector::isFinite() const
{
    return isfinite(x) && isfinite(y) && isfinite(z);
}

void ETVector::fixFinite()
{
    if (!isfinite(x)) x = 0.0f;
    if (!isfinite(x)) y = 0.0f;
    if (!isfinite(x)) z = 0.0f;
}

void ETVector::min(const ETVector &v)
{
    if (v.x<x) x = v.x;
    if (v.y<y) y = v.y;
    if (v.z<z) z = v.z;
}

void ETVector::max(const ETVector &v)
{
    if (v.x>x) x = v.x;
    if (v.y>y) y = v.y;
    if (v.z>z) z = v.z;
}

