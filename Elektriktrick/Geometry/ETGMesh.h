//
//  ETGMesh.hpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 10/21/15.
//  Copyright © 2015 M.Melcher GmbH. All rights reserved.
//

#ifndef ETGMesh_hpp
#define ETGMesh_hpp

#include <vector>

class ISVertex;
class ISEdge;
class ISFace;
class ISMesh;


class ISVec3
{
public:
    ISVec3();
    ISVec3(const ISVec3&);
    ISVec3(double*);
    ISVec3(float, float, float);
    ISVec3& operator-=(const ISVec3&);
    ISVec3& operator+=(const ISVec3&);
    ISVec3& operator*=(double);
    double *dataPointer() { return pV; }
    ISVec3& cross(const ISVec3&);
    double normalize();
    void zero();
    void write(double*);
    void read(float*);
    void read(double*);
    double x() { return pV[0]; }
    double y() { return pV[1]; }
    double z() { return pV[2]; }
    void set(float, float, float);
    void setMinimum(const ISVec3 &b);
    void setMaximum(const ISVec3 &b);
    double pV[3];
};


class ISVertex
{
public:
    ISVertex();
    ISVertex(const ISVertex*);
    bool validNormal() { return pNNormal==1; }
    void addNormal(const ISVec3&);
    void averageNormal();
    void print();
    ISVec3 pPosition;
    ISVec3 pNormal;
    int pNNormal;
};


typedef ISVertex *ISVertexPtr;
typedef std::vector<ISVertex*> ISVertexList;

class ISEdge
{
public:
    ISEdge();
    ISVertex *findZ(double);
    ISVertex *vertex(int i, ISFace *f);
    ISFace *otherFace(ISFace *);
    ISVertex *otherVertex(ISVertex*);
    int indexIn(ISFace *);
    int nFaces();
    ISFace *pFace[2];
    ISVertex *pVertex[2];
};

typedef std::vector<ISEdge*> ISEdgeList;

class ISFace
{
public:
    ISFace();
    bool validNormal() { return pNNormal==1; }
    void rotateVertices();
    void print();
    int pointsBelowZ(double z);
    ISVertex *pVertex[3];
    ISEdge *pEdge[3];
    ISVec3 pNormal;
    int pNNormal;
    bool pUsed;
};

typedef ISFace *ISFacePtr;
typedef std::vector<ISFace*> ISFaceList;

class ISMesh
{
public:
    ISMesh(const char *filename);
    virtual ~ISMesh() { clear(); }
    virtual void clear();
    bool validate();
    void drawGouraud();
    void drawFlat(unsigned int);
    void drawShrunk(unsigned int, double);
    void drawEdges();
    void addFace(ISFace*);
    void clearFaceNormals();
    void clearVertexNormals();
    void clearNormals() { clearFaceNormals(); clearVertexNormals(); }
    void calculateFaceNormals();
    void calculateVertexNormals();
    void calculateNormals() { calculateFaceNormals(); calculateVertexNormals(); }
    void fixHoles();
    void fixHole(ISEdge*);
    void shrink(double dx, double dy, double dz);
    void saveCopy(const char *fix);
    void saveCopyAs(const char *filename);
    ISEdge *findEdge(ISVertex*, ISVertex*);
    ISEdge *addEdge(ISVertex*, ISVertex*, ISFace*);
    ISVertexList vertexList;
    ISEdgeList edgeList;
    ISFaceList faceList;
    char *pFilename;
};

typedef std::vector<ISMesh*> ISMeshList;

class ISMeshSlice : public ISMesh
{
public:
    ISMeshSlice();
    virtual ~ISMeshSlice();
    virtual void clear();
    void drawLidEdge();
    void tesselate();
    void addZSlice(const ISMesh&, double);
    void addFirstLidVertex(ISFace *isFace, double zMin);
    void addNextLidVertex(ISFacePtr &isFace, ISVertexPtr &vCutA, int &edgeIndex, double zMin);
    ISEdgeList lidEdgeList;
};

void clearSTL();
void loadSTL(const char *filename);
void drawModelGouraud();
void drawModelFlat(unsigned int color);

extern ISMeshList gMeshList;


#endif /* ETGMesh_hpp */
