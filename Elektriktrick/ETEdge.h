#ifndef ETEDGE_H
#define ETEDGE_H


class ETTriangle;
class ETVertex;

class ETEdge
{
public:
    ETEdge();
//private:
    ETEdge *pNext;
    int pNTriangle;
    ETVertex *v0, *v1;
    ETTriangle *t0, *t1;
};

#endif // ETEDGE_H
