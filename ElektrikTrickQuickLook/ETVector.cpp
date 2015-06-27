//
//  ETVector.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETVector.h"

#include <math.h>


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

void ETVector::mul(float v)
{
    x *= v;
    y *= v;
    z *= v;
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

float ETVector::dot(const ETVector &v) const
{
    return x*v.x + y*v.y + z*v.z;
}


void ETVector::cross(const ETVector &v)
{
    ETVector u; u.set(*this);
    x = (u.y*v.z) - (u.z*v.y);
    y = (u.z*v.x) - (u.x*v.z);
    z = (u.x*v.y) - (u.y*v.x);
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


float ETVector::angle2d() const
{
    float ret = 0.0;
    float m = sqrtf(x*x + y*y);
    
    if (m>1.0e-6) {
        ETVector x; x.set(1, 0, 0);
        float dp = dot(x);
        if (dp/m>=1.0) {
            ret = 0.0;
        }
        else if (dp/m<-1.0) {
            ret = M_PI;
        }
        else {
            ret = acosf(dp/m);
        }
        if (y<0.0) {
            ret = 2*M_PI - ret;
        }
    }
    return ret;
}


float ETVector::angleTo2d(const ETVector& v) const
{
    ETVector w;
    w.set(v);
    w.sub(*this);
    return w.angle2d();
}


void ETVector::setPolar2d(float radius, float angle)
{
    x = radius * cosf(angle);
    y = radius * sinf(angle);
    z = 0.0;
}



