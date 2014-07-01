#ifndef ETVERTEX_H
#define ETVERTEX_H

#include "ETVector.h"

class ETVertex
{
public:
    ETVertex();
//private:
    ETVertex *pNext;
    ETVector p;
    ETVector n;
};

#endif // ETVERTEX_H
