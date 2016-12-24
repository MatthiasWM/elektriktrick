//
//  ETGMesh.cpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 10/21/15.
//  Copyright © 2015 M.Melcher GmbH. All rights reserved.
//

#include "ETGMesh.h"

#include <FL/fl_ask.h>
#include <errno.h>


#undef M_MONKEY
#define M_DRAGON

static int kNDrops = 10;

void sendShort(FILE *f, unsigned short v) {
    fputc( (v&0xff), f);
    fputc( ((v>>8)&0xff), f);
}

void sendInt(FILE *f, unsigned int v) {
    fputc( (v&0xff), f);
    fputc( ((v>>8)&0xff), f);
    fputc( ((v>>16)&0xff), f);
    fputc( ((v>>24)&0xff), f);
}

void sendFloat(FILE *f, float v) {
    fwrite(&v, 4, 1, f);
}

/*

 Slicing Strategy:

 We need two sets of information in a slice.

 Set one defines what is inside and outside of the slice. This is where binder
 needs to go to make the model complete. Missing triangles in non-manifold
 models cause missing layers and broken models. The slicer must not crash
 on missing or deformed triangles. It should instead fill holes in some way.

 Set two calculates the shell color. The color layer is only a few millimeters
 thick to give a deep color, but it must not overlap with other triangles. The
 surface color pixels are extended into the model, following the interopolated
 point normals.

 Slices always have a thickness! The binder slice is generated using two boolean
 operations that cut above and below the slice while maintaining a close
 surface (the resulting slice has a lid at the top and bottom. The slice is then
 rendered from Z to generate the binder voxel layer.

 The color slice is basically a binder slice minus the top and bottom lid, plus
 color and texture information. It is rendered viewing from the top. Only pure
 binder voxels can be changed (no outside voxels, no voxels that were already
 colored).

 To generate a thickness in the color shell, all model vertices are moved by
 half a voxel radius along their negtive point normal, and the color information
 is rendered again. This is repeated until the shell has the required thickness.


 Advanced options:

 We can support known point normals by creating convex and concave surfaces
 that will give a shading similar to what the point-normal-interpolation
 would achieve.

 We can apply bump maps to the model by generating appropriate structures
 in the voxels.


 Support / packaging:

 We can generate support structures for the model while printing it, without
 generating a physical connection to the model. For shipping, the model can then
 be wrapped in a thin foam and placed back in the support structure.


 Dimensions:

 +-V----------+    +-----+        o----> x
 |ooo         |  ==| | | |==      |
 | +--------+ |    |:|:|:|        |     z points up
 > |o       | |    +-----+        V y
 | |        | |
 | |        | |
 | +--------+ |
 | |o       | |
 | |        | |
 | |        | |
 | +--------+ |
 | +--------+ |
 |            |
 +------------+

 physical range rect
 supply box
 build box
 carriage rect
 nozzle rect
 carriage park pos
 roller y pos
 powder collector y pos

 rect: x, y, w, h in relation to end stops
 box: x, y, z, w, h, d in relation to end stops
 pos: x, y

 all measurements are in step of the corresponding axis
 steps per mm are stored seperately


 Test Patterns:

 - Layer height vs. ink per dot
 print 5x5x100mm columns, ten next to each other with 1..10 drops
 print ten layers of that, with smaller layer height every time
 */

/*

 X range is 0...18500, 18500 steps or 512 dots
 y range is 21500...??

 */


#include <FL/Fl.h>
#include <FL/Fl_Button.h>
#include <FL/Fl_Gl_Window.h>
#include <FL/Fl_Slider.h>
#include <FL/gl.h>
#include <FL/glu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

//#include "lib3ds.h"


#ifdef _MSC_VER
#pragma warning ( disable : 4996 )
#endif

Fl_Slider *zSlider1, *zSlider2;

ISMeshList gMeshList;
ISMeshSlice gMeshSlice;

GLUtesselator *gGluTess = 0;

bool gShowSlice = false;
int gWriteSliceNext = 0;

FILE *gOutFile;

// -----------------------------------------------------------------------------
// Machine Parameters

// ink head data
const double gIotaXDpi = 96.0;            // targeted resolution in X direction
const double gIotaYDpi = 96.0;            // ink head resolution in Y direction
const double gIotaXDotsPmm = 96.0;        // same in metric
const double gIotaYDotsPmm = 96.0;        // same in metric
const int gIotaNozzles = 12;              // number of nozzles on cartridge
                                          // TODO: this assumes a straigh vertical row of nozzles. Patterns must be implemented later

// stepper motor data
const double gIotaXStepsPmm = 10.0;       // x motion of carriage per step pulse
const double gIotaYStepsPmm = 10.0;       // y motion of bridge per step pulse
const double gIotaZ1StepsPmm = 10.0;      // Supply piston motion per step
const double gIotaZ2StepsPmm = 10.0;      // Build piston motion per step
const double gIotaRStepsPRound = 200.0;

// carriage dimensions
const double gIotaCarW = 30.0;
const double gIotaCarH = 60.0;
const double gIotaHeadX = 10.0;
const double gIotaHeadY = 10.0;
const double gIotaHeadW = 10.0;
const double gIotaHeadH = 60.0;
const double gIotaRollerX = 10.0;
const double gIotaRollerY = 10.0;
const double gIotaRollerW = 10.0;
const double gIotaRollerH = 60.0;
const double gIotaHeadParkX = 10.0;
const double gIotaHeadParkY = 10.0;

// printer dimensions
const double gIotaPrinterX = -1.5;
const double gIotaPrinterY = -1.5;
const double gIotaPrinterW = 600.0;
const double gIotaPrinterH = 800.0;
const double gIotaSupplyBoxX = -1.5;
const double gIotaSupplyBoxY = -1.5;
const double gIotaSupplyBoxW = 600.0;
const double gIotaSupplyBoxH = 800.0;
const double gIotaSupplyBoxMinD = 0.0;
const double gIotaSupplyBoxMaxD = 150.0;
const double gIotaBuildBoxX = -1.5;
const double gIotaBuildBoxY = -1.5;
const double gIotaBuildBoxW = 600.0;
const double gIotaBuildBoxH = 800.0;
const double gIotaBuildBoxMinD = 0.0;
const double gIotaBuildBoxMaxD = 150.0;

// model
const double gModelScale = 40.0;
const double gMinimumShell = 4.0; // mm

// -----------------------------------------------------------------------------

void writeInt(FILE *f, int32_t x)
{
    uint8_t v;
    // bits 34..28
    if (x&(0xffffffff<<28)) {
        v = ((x>>28) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 27..21
    if (x&(0xffffffff<<21)) {
        v = ((x>>21) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 20..14
    if (x&(0xffffffff<<14)) {
        v = ((x>>14) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 13..7
    if (x&(0xffffffff<<7)) {
        v = ((x>>7) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 6..0
    v = x & 0x7f;
    fputc(v, f);
}

// -----------------------------------------------------------------------------

ISVec3::ISVec3()
{
    pV[0] = 0.0;
    pV[1] = 0.0;
    pV[2] = 0.0;
}

ISVec3::ISVec3(const ISVec3 &v)
{
    pV[0] = v.pV[0];
    pV[1] = v.pV[1];
    pV[2] = v.pV[2];
}

ISVec3::ISVec3(double *v)
{
    pV[0] = v[0];
    pV[1] = v[1];
    pV[2] = v[2];
}

ISVec3::ISVec3(float x, float y, float z)
{
    pV[0] = x;
    pV[1] = y;
    pV[2] = z;
}

void ISVec3::set(float x, float y, float z)
{
    pV[0] = x;
    pV[1] = y;
    pV[2] = z;
}

void ISVec3::read(float *v)
{
    pV[0] = v[0];
    pV[1] = v[1];
    pV[2] = v[2];
}

void ISVec3::read(double *v)
{
    pV[0] = v[0];
    pV[1] = v[1];
    pV[2] = v[2];
}

void ISVec3::write(double *v)
{
    v[0] = pV[0];
    v[1] = pV[1];
    v[2] = pV[2];
}

double ISVec3::normalize()
{
    double len = sqrt(pV[0]*pV[0]+pV[1]*pV[1]+pV[2]*pV[2]);
    if (len==0.0) {
        len = 1.0;
    } else {
        len = 1.0/len;
    }
    pV[0] *= len;
    pV[1] *= len;
    pV[2] *= len;
    return len;
}

ISVec3& ISVec3::operator-=(const ISVec3 &v)
{
    pV[0] -= v.pV[0];
    pV[1] -= v.pV[1];
    pV[2] -= v.pV[2];
    return *this;
}

ISVec3& ISVec3::operator+=(const ISVec3 &v)
{
    pV[0] += v.pV[0];
    pV[1] += v.pV[1];
    pV[2] += v.pV[2];
    return *this;
}

ISVec3& ISVec3::operator*=(double n)
{
    pV[0] *= n;
    pV[1] *= n;
    pV[2] *= n;
    return *this;
}

ISVec3& ISVec3::cross(const ISVec3 &b)
{
    ISVec3 a(*this);
    pV[0] = a.pV[1]*b.pV[2] - a.pV[2]*b.pV[1];
    pV[1] = a.pV[2]*b.pV[0] - a.pV[0]*b.pV[2];
    pV[2] = a.pV[0]*b.pV[1] - a.pV[1]*b.pV[0];
    return *this;
}

void ISVec3::zero()
{
    pV[0] = 0.0;
    pV[1] = 0.0;
    pV[2] = 0.0;
}

void ISVec3::setMinimum(const ISVec3 &b)
{
    if (b.pV[0]<pV[0]) pV[0] = b.pV[0];
    if (b.pV[1]<pV[1]) pV[1] = b.pV[1];
    if (b.pV[2]<pV[2]) pV[2] = b.pV[2];
}

void ISVec3::setMaximum(const ISVec3 &b)
{
    if (b.pV[0]>pV[0]) pV[0] = b.pV[0];
    if (b.pV[1]>pV[1]) pV[1] = b.pV[1];
    if (b.pV[2]>pV[2]) pV[2] = b.pV[2];
}


// -----------------------------------------------------------------------------

ISVertex::ISVertex()
{
    pPosition.zero();
    pNormal.zero();
    pNNormal = 0;
}

ISVertex::ISVertex(const ISVertex *v)
{
    pPosition = v->pPosition;
    pNormal = v->pNormal;
    pNNormal = v->pNNormal;
}

void ISVertex::addNormal(const ISVec3 &v)
{
    ISVec3 vn(v);
    vn.normalize();
    pNormal += vn;
    pNNormal++;
}

void ISVertex::averageNormal()
{
    if (pNNormal>0) {
        double len = 1.0/pNNormal;
        pNormal *= len;
    }
}

void ISVertex::print()
{
    printf("v=[%g, %g, %g]\n", pPosition.x(), pPosition.y(), pPosition.z());
}

// -----------------------------------------------------------------------------

ISEdge::ISEdge()
{
    pVertex[0] = 0L;
    pVertex[1] = 0L;
    pFace[0] = 0L;
    pFace[1] = 0L;
}

ISVertex *ISEdge::vertex(int i, ISFace *f)
{
    if (pFace[0]==f) {
        return pVertex[i];
    } else if (pFace[1]==f) {
        return pVertex[1-i];
    } else {
        puts("ERROR: vertex() - this edge is not associated with this face!");
        return 0L;
    }
}

ISVertex *ISEdge::findZ(double zMin)
{
    ISVertex *v0 = pVertex[0], *v1 = pVertex[1];
    ISVec3 vd0(v0->pPosition);
    bool retVec = false;
    vd0 -= v1->pPosition;
    double dzo = vd0.z(), dzn = zMin-v1->pPosition.z();
    double m = dzn/dzo;  // TODO: division by zero should not be possible...
    if (m>=0.0 && m<=1) retVec = true;
    if (retVec) {
        ISVertex *v2 = new ISVertex();
        vd0 *= m;
        vd0 += v1->pPosition;
        v2->pPosition = vd0;
        return v2;
    } else {
        return 0L;
    }
}

ISFace *ISEdge::otherFace(ISFace *f)
{
    if (pFace[0]==f) {
        return pFace[1];
    } else if (pFace[1]==f) {
        return pFace[0];
    } else {
        puts("ERROR: otherFace() - this edge is not associated with this face!");
        return 0L;
    }
}

int ISEdge::indexIn(ISFace *f)
{
    if (f->pEdge[0]==this) return 0;
    if (f->pEdge[1]==this) return 1;
    if (f->pEdge[2]==this) return 2;
    puts("ERROR: indexIn() - this edge was not found with this face!");
    return -1;
}

int ISEdge::nFaces()
{
    int n = 0;
    if (pFace[0]) n++;
    if (pFace[1]) n++;
    return n;
}

// -----------------------------------------------------------------------------

ISFace::ISFace()
{
    pVertex[0] = 0L;
    pVertex[1] = 0L;
    pVertex[2] = 0L;
    pEdge[0] = 0L;
    pEdge[1] = 0L;
    pEdge[2] = 0L;
    pNormal.zero();
    pNNormal = 0;
}

void ISFace::rotateVertices()
{
    ISVertex *v = pVertex[0];
    pVertex[0] = pVertex[1];
    pVertex[1] = pVertex[2];
    pVertex[2] = v;
    ISEdge *e = pEdge[0];
    pEdge[0] = pEdge[1];
    pEdge[1] = pEdge[2];
    pEdge[2] = e;
}

void ISFace::print()
{
    printf("Face: \n");
    pVertex[0]->print();
    pVertex[1]->print();
    pVertex[2]->print();
}

int ISFace::pointsBelowZ(double zMin)
{
    double z0 = pVertex[0]->pPosition.z();
    double z1 = pVertex[1]->pPosition.z();
    double z2 = pVertex[2]->pPosition.z();
    int n = (z0<zMin) + (z1<zMin) + (z2<zMin);
    return n;
}

// -----------------------------------------------------------------------------

ISMesh::ISMesh(const char *filename)
{
  pFilename = strdup(filename);
}

void ISMesh::clear()
{
    int i, n = (int)edgeList.size();
    for (i=0; i<n; i++) {
        delete edgeList[i];
    }
    edgeList.clear();
    n = (int)faceList.size();
    for (i=0; i<n; i++) {
        delete faceList[i];
    }
    faceList.clear();
    n = (int)vertexList.size();
    for (i=0; i<n; i++) {
        delete vertexList[i];
    }
    vertexList.clear();
    
    free(pFilename);
    pFilename = strdup("");
}

bool ISMesh::validate()
{
    if (faceList.size()>0 && edgeList.size()==0) {
        puts("ERROR: empty edge list!");
    }
    int i, n = (int)edgeList.size();
    for (i=0; i<n; i++) {
        ISEdge *e = edgeList[i];
        if (e) {
            if (e->pFace[0]==0L) {
                printf("ERROR: edge %d [%p] without face found!\n", i, e);
            } else if (e->pFace[1]==0L) {
                printf("ERROR: edge %d [%p] with single face found (hole in mesh)!\n", i, e);
            }
            if (e->pFace[0]) {
                if (e->pFace[0]->pEdge[0]!=e && e->pFace[0]->pEdge[1]!=e && e->pFace[0]->pEdge[2]!=e) {
                    printf("ERROR: face [%p] is not pointing back at edge %d [%p]!\n", e->pFace[0], i, e);
                }
            }
            if (e->pFace[1]) {
                if (e->pFace[1]->pEdge[0]!=e && e->pFace[1]->pEdge[1]!=e && e->pFace[1]->pEdge[2]!=e) {
                    printf("ERROR: face [%p] is not pointing back at edge %d [%p]!\n", e->pFace[1], i, e);
                }
            }
            if (e->pVertex[0]==0L || e->pVertex[1]==0L) {
                printf("ERROR: edge %d [%p] missing a vertex reference!\n", i, e);
            }
        } else {
            puts("ERROR: zero edge found!");
        }
    }
    n = (int)faceList.size();
    for (i=0; i<n; i++) {
        ISFace *f = faceList[i];
        if (f) {
            if (f->pVertex[0]==0L || f->pVertex[1]==0L || f->pVertex[1]==0L) {
                printf("ERROR: face %d has an empty vertex field.\n", i);
            }
            if (f->pEdge[0]==0L || f->pEdge[1]==0L || f->pEdge[1]==0L) {
                printf("ERROR: face %d has an empty edge field.\n", i);
            } else {
                if (f->pEdge[0]->vertex(0, f)!=f->pVertex[0])
                    printf("ERROR: face %d has an edge0/vertex0 missmatch.\n", i);
                if (f->pEdge[0]->vertex(1, f)!=f->pVertex[1])
                    printf("ERROR: face %d has an edge0/vertex1 missmatch.\n", i);
                if (f->pEdge[1]->vertex(0, f)!=f->pVertex[1])
                    printf("ERROR: face %d has an edge1/vertex1 missmatch.\n", i);
                if (f->pEdge[1]->vertex(1, f)!=f->pVertex[2])
                    printf("ERROR: face %d has an edge1/vertex2 missmatch.\n", i);
                if (f->pEdge[2]->vertex(0, f)!=f->pVertex[2])
                    printf("ERROR: face %d has an edge2/vertex2 missmatch.\n", i);
                if (f->pEdge[2]->vertex(1, f)!=f->pVertex[0])
                    printf("ERROR: face %d has an edge2/vertex0 missmatch.\n", i);
                if (f->pEdge[0]->pFace[0]!=f && f->pEdge[0]->pFace[1]!=f)
                    printf("ERROR: face %d edge0 does not point back at face.\n", i);
                if (f->pEdge[1]->pFace[0]!=f && f->pEdge[1]->pFace[1]!=f)
                    printf("ERROR: face %d edge1 does not point back at face.\n", i);
                if (f->pEdge[2]->pFace[0]!=f && f->pEdge[2]->pFace[1]!=f)
                    printf("ERROR: face %d edge2 does not point back at face.\n", i);
            }
        } else {
            puts("ERROR: zero face found!");
        }
    }
    return true;
}

/**
 This function finds edges that have only a single face associated.
 It then adds a face to this edge and the next edge without a second face.
 If three edges are connected and none has a second face, a new triangle
 will fill the hole.
 */
void ISMesh::fixHoles()
{
    printf("Fixing holes...\n");
#ifdef M_MONKEY
    fixHole(edgeList[21]);
#elif defined M_DRAGON
    return;
#endif
//    int i;
//    for (i=0; i<(int)edgeList.size(); i++) {
//        ISEdge *e = edgeList[i];
//        while ( e->nFaces()==1 ) // FIXME: make sure that this is not endless
//            fixHole(e);
//    }
}

void ISMesh::fixHole(ISEdge *e)
{
    printf("Fixing a hole...\n");
    ISFace *fFix;
    if (e->pFace[0])
        fFix = e->pFace[0];
    else
        fFix = e->pFace[1];
    // walk the fan to the left and find the next edge
    ISFace *fLeft = fFix;
    ISEdge *eLeft = e;
    for (;;) {
        int ix = eLeft->indexIn(fLeft);
        eLeft = fLeft->pEdge[(ix+2)%3];
        if (eLeft->nFaces()==1)
            break;
        fLeft = eLeft->otherFace(fLeft);
    }
    // walk the fan to the right and find the next edge
    ISFace *fRight = fFix;
    ISEdge *eRight = e;
    for (;;) {
        int ix = eRight->indexIn(fRight);
        eRight = fRight->pEdge[(ix+1)%3];
        if (eRight->nFaces()==1)
            break;
        fRight = eRight->otherFace(fRight);
    }
    // eLeft and eRight are the next connecting edges
    // fLeft and fRight are the only connected faces
    // fLeft and fRight can well be fFix
    ISVertex *vLeft = eLeft->vertex(0, fLeft);
    ISVertex *vRight = eRight->vertex(1, fRight);
    if (eLeft==eRight) {
        // this is a zero size hole: merge the edges
        puts("ERROR: zero size hole!");
    } else if ( vLeft==vRight ) {
        // this triangle fill conpletely fill the hole
        ISFace *fNew = new ISFace();
        fNew->pVertex[0] = e->vertex(1, fFix);
        fNew->pVertex[1] = e->vertex(0, fFix);
        fNew->pVertex[2] = vLeft;
        addFace(fNew);
    } else if (fFix==fRight) {
        if (fLeft==fRight) {
            // we have a single triangle without any connections, delete?
            ISFace *fNew = new ISFace();
            fNew->pVertex[0] = fFix->pVertex[2];
            fNew->pVertex[1] = fFix->pVertex[1];
            fNew->pVertex[2] = fFix->pVertex[0];
            addFace(fNew);
        } else {
            fixHole(eRight);
        }
    } else {
        // add one more triangle to get closer to filling the hole
        ISFace *fNew = new ISFace();
        fNew->pVertex[0] = e->vertex(1, fFix);
        fNew->pVertex[1] = e->vertex(0, fFix);
        fNew->pVertex[2] = eRight->vertex(1, fRight);
        addFace(fNew);
    }
}

void ISMesh::addFace(ISFace *newFace)
{
    newFace->pEdge[0] = addEdge(newFace->pVertex[0], newFace->pVertex[1], newFace);
    newFace->pEdge[1] = addEdge(newFace->pVertex[1], newFace->pVertex[2], newFace);
    newFace->pEdge[2] = addEdge(newFace->pVertex[2], newFace->pVertex[0], newFace);
    faceList.push_back(newFace);
}

ISEdge *ISMesh::addEdge(ISVertex *v0, ISVertex *v1, ISFace *face)
{
    ISEdge *isEdge = findEdge(v0, v1);
    if (isEdge) {
        isEdge->pFace[1] = face;
    } else {
        isEdge = new ISEdge();
        isEdge->pVertex[0] = v0;
        isEdge->pVertex[1] = v1;
        isEdge->pFace[0] = face;
        edgeList.push_back(isEdge);
    }
    return isEdge;
}

ISEdge *ISMesh::findEdge(ISVertex *v0, ISVertex *v1)
{
    int i, n = (int)edgeList.size();
    for (i=0; i<n; i++) {
        ISEdge *isEdge = edgeList.at(i);
        ISVertex *ev0 = isEdge->pVertex[0];
        ISVertex *ev1 = isEdge->pVertex[1];
        if ((ev0==v0 && ev1==v1)||(ev0==v1 && ev1==v0))
            return isEdge;
    }
    return 0;
}

void ISMesh::clearFaceNormals()
{
    int i, n = (int)faceList.size();
    for (i=0; i<n; i++) {
        ISFace *isFace = faceList.at(i);
        isFace->pNNormal = 0;
    }
}

void ISMesh::clearVertexNormals()
{
    int i, n = (int)vertexList.size();
    for (i=0; i<n; i++) {
        ISVertex *isVertex = vertexList.at(i);
        isVertex->pNNormal = 0;
    }
}

void ISMesh::calculateFaceNormals()
{
    int i, n = (int)faceList.size();
    for (i=0; i<n; i++) {
        ISFace *isFace = faceList.at(i);
        ISVec3 p0(isFace->pVertex[0]->pPosition);
        ISVec3 p1(isFace->pVertex[1]->pPosition);
        ISVec3 p2(isFace->pVertex[2]->pPosition);
        p1 -= p0;
        p2 -= p0;
        ISVec3 n = p1.cross(p2);
        n.normalize();
        isFace->pNormal = n;
        isFace->pNNormal = 1;
    }
}

void ISMesh::calculateVertexNormals()
{
    int i, n = (int)faceList.size();
    for (i=0; i<n; i++) {
        ISFace *isFace = faceList.at(i);
        ISVec3 n(isFace->pNormal);
        isFace->pVertex[0]->addNormal(n);
        isFace->pVertex[1]->addNormal(n);
        isFace->pVertex[2]->addNormal(n);
    }
    n = (int)vertexList.size();
    for (i=0; i<n; i++) {
        ISVertex *isVertex = vertexList.at(i);
        isVertex->averageNormal();
    }
}

void ISMesh::drawGouraud() {
    int i, j, n = (int)faceList.size();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    for (i = 0; i < n; i++) {
        ISFace *isFace = faceList[i];
        for (j = 0; j < 3; ++j) {
            ISVertex *isVertex = isFace->pVertex[j];
            glNormal3dv(isVertex->pNormal.dataPointer());
            glVertex3dv(isVertex->pPosition.dataPointer());
        }
    }
    glEnd();
}

void ISMesh::drawFlat(unsigned int color) {
    int i, j, n = (int)faceList.size();
    unsigned char r, g, b;
    Fl::get_color(color, r, g, b);
    glColor3f(r/266.0, g/266.0, b/266.0);
    glBegin(GL_TRIANGLES);
    for (i = 0; i < n; i++) {
        ISFace *isFace = faceList[i];
        glNormal3dv(isFace->pNormal.dataPointer());
        for (j = 0; j < 3; ++j) {
            ISVertex *isVertex = isFace->pVertex[j];
            glVertex3dv(isVertex->pPosition.dataPointer());
        }
    }
    glEnd();
}

void ISMesh::drawShrunk(unsigned int color, double scale) {
    int i, j, n = (int)faceList.size();
    unsigned char r, g, b;
    Fl::get_color(color, r, g, b);
    glColor3f(r/266.0, g/266.0, b/266.0);
    glBegin(GL_TRIANGLES);
    for (i = 0; i < n; i++) {
        ISFace *isFace = faceList[i];
        for (j = 0; j < 3; ++j) {
            ISVertex *isVertex = isFace->pVertex[j];
            ISVec3 p = isVertex->pPosition;
            ISVec3 n = isVertex->pNormal;
            n *= scale;
            p -= n;
            glVertex3dv(p.dataPointer());
        }
    }
    glEnd();
}

void ISMesh::drawEdges() {
    int i, j, n = (int)edgeList.size();
    glColor3f(0.8f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    for (i = 0; i < n; i++) {
        ISEdge *isEdge = edgeList[i];
        for (j = 0; j < 2; ++j) {
            ISVertex *isVertex = isEdge->pVertex[j];
            glVertex3dv(isVertex->pPosition.dataPointer());
        }
    }
    glEnd();
}

void ISMesh::shrink(double dx, double dy, double dz)
{
    /* Goal: shrink the surface of an object without scaling it (like ppeling an orange)
       Strategy: for every vertex, collect all associated face normals. Find the 
             minimum and maximum for each component of the normal. Take the average
             of min and max and move the vertex by -2 times the given direction.
     */
    // slow, but doesn't need additional storage
    unsigned int nv = (unsigned int)vertexList.size();
    unsigned int nf = (unsigned int)faceList.size();
    for (unsigned int i=0; i<nv; i++) {
        ISVertex *v = vertexList.at(i);
        ISVec3 min, max;
        for (unsigned int j=0; j<nf; j++) {
            ISFace *f = faceList.at(j);
            if (f->pVertex[0]==v || f->pVertex[1]==v || f->pVertex[2]==v) {
                ISVec3 n = f->pNormal;
                n.normalize();
                min.setMinimum(n);
                max.setMaximum(n);
            }
        }
        v->pPosition.set(
                         v->pPosition.x() - dx*(min.x()+max.x()),
                         v->pPosition.y() - dy*(min.y()+max.y()),
                         v->pPosition.z() - dz*(min.z()+max.z())
        );
    }
    printf("DONE\n");
}


void ISMesh::saveCopyAs(const char *filename)
{
    printf("Filename is '%s'\n", filename);
    FILE *f = fopen(filename, "wb");
    if (!f) {
        fl_message("Can't open file\n%s\nfor writing:\n%s", filename, strerror(errno));
        return;
    }
    unsigned int i, n = (unsigned int)faceList.size();
    for (i=0; i<80; i++) fputc(0, f);
    sendInt(f, n);
    for (i=0; i<n; i++) {
        ISFace *face = faceList.at(i);
        sendFloat(f, face->pNormal.x());
        sendFloat(f, face->pNormal.y());
        sendFloat(f, face->pNormal.z());
        for (int j=0; j<3; j++) {
            ISVertex *v = face->pVertex[j];
            sendFloat(f, v->pPosition.x());
            sendFloat(f, v->pPosition.y());
            sendFloat(f, v->pPosition.z());
        }
        sendShort(f, 0);
    }
    fclose(f);
    printf("Wrote %s\n", filename);
}


void ISMesh::saveCopy(const char *fix)
{
    char *buf = (char*)malloc(strlen(pFilename)+strlen(fix)+1);
    char *ext = strrchr(pFilename, '.');
    if (ext) {
        int n = (int)(ext-pFilename);
        memcpy(buf, pFilename, n);
        strcpy(buf+n, fix);
        strcat(buf, ext);
    } else {
        strcpy(buf, pFilename);
        strcat(buf, fix);
    }
    saveCopyAs(buf);
    free(buf);
}



// -----------------------------------------------------------------------------


ISMeshSlice::ISMeshSlice()
: ISMesh("")
{
}

ISMeshSlice::~ISMeshSlice()
{
    clear();
}

void ISMeshSlice::clear()
{
    int i, n = (int)lidEdgeList.size();
    for (i=0; i<n; i++) {
        delete lidEdgeList[i];
    }
    lidEdgeList.clear();
    ISMesh::clear();
}

void ISMeshSlice::drawLidEdge()
{
    int i, j, n = (int)lidEdgeList.size();
    glColor3f(0.5f, 0.5f, 1.0f);
    glBegin(GL_LINES);
    for (i = 0; i < n; i++) {
        ISEdge *isEdge = lidEdgeList[i];
        if (isEdge) {
            for (j = 0; j < 2; ++j) {
                ISVertex *isVertex = isEdge->pVertex[j];
                glVertex3dv(isVertex->pPosition.dataPointer());
            }
        }
    }
    glEnd();
}


int tessVertexCount = 0;
ISVertex *tessV0, *tessV1, *tessV2;

void tessBeginCallback(GLenum which)
{
    tessVertexCount = 0;
}

void tessEndCallback()
{
    // tessVertexCount must be 0!
}

void tessVertexCallback(GLvoid *vertex)
{
    if (tessVertexCount==0) {
        tessV0 = (ISVertex*)vertex;
        tessVertexCount = 1;
    } else if (tessVertexCount==1) {
        tessV1 = (ISVertex*)vertex;
        tessVertexCount = 2;
    } else {
        tessV2 = (ISVertex*)vertex;
        ISFace *f = new ISFace();
        f->pVertex[0] = tessV0;
        f->pVertex[1] = tessV1;
        f->pVertex[2] = tessV2;
        gMeshSlice.addFace(f);
        tessVertexCount = 0;
    }
}

void tessCombineCallback(GLdouble coords[3],
                         ISVertex *vertex_data[4],
                         GLfloat weight[4], ISVertex **dataOut )
{
    ISVertex *v = new ISVertex();
    v->pPosition.read(coords);
    gMeshSlice.vertexList.push_back(v);
    *dataOut = v;
}

void tessEdgeFlagCallback(GLboolean flag)
{
}

void tessErrorCallback(GLenum errorCode)
{
    const GLubyte *estring;
    estring = gluErrorString(errorCode);
    fprintf (stderr, "Tessellation Error: %s\n", estring);
}

void ISMeshSlice::tesselate()
{
    if (!gGluTess)
        gGluTess = gluNewTess();

    gluTessCallback(gGluTess, GLU_TESS_VERTEX, (GLvoid (*) ()) &tessVertexCallback);
    gluTessCallback(gGluTess, GLU_TESS_BEGIN, (GLvoid (*) ()) &tessBeginCallback);
    gluTessCallback(gGluTess, GLU_TESS_END, (GLvoid (*) ()) &tessEndCallback);
    gluTessCallback(gGluTess, GLU_TESS_ERROR, (GLvoid (*) ()) &tessErrorCallback);
    gluTessCallback(gGluTess, GLU_TESS_COMBINE, (GLvoid (*) ()) &tessCombineCallback);
    gluTessCallback(gGluTess, GLU_TESS_EDGE_FLAG, (GLvoid (*) ()) &tessEdgeFlagCallback);

    gluTessProperty(gGluTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);

    int i, n = (int)lidEdgeList.size();
    tessVertexCount = 0;
    gluTessBeginPolygon(gGluTess, this);
    gluTessBeginContour(gGluTess);
    for (i=0; i<n; i++) {
        ISEdge *e = lidEdgeList[i];
        if (e==NULL) {
            gluTessEndContour(gGluTess);
            gluTessBeginContour(gGluTess);
        } else {
            gluTessVertex(gGluTess, e->pVertex[0]->pPosition.dataPointer(), e->pVertex[0]);
        }
    }
    gluTessEndContour(gGluTess);
    gluTessEndPolygon(gGluTess);
}


/*
 Create the edge that cuts this triangle in half.

 The first point is know to be on the z slice. The second edge that crosses
 z is found and the point of intersection is calculated. Then an edge is
 created that splits the face on the z plane.

 \param isFace the face that is split in two; the face must cross zMin
 \param vCutA the first point on zMin along the first edge
 \param edgeIndex the index of the first edge that crosses zMin
 \param zMin slice on this z plane
 */
void ISMeshSlice::addNextLidVertex(ISFacePtr &isFace, ISVertexPtr &vCutA, int &edgeIndex, double zMin)
{
    // faces are always clockwise
    ISVertex *vOpp = isFace->pVertex[(edgeIndex+2)%3];
    int newIndex;
    if (vOpp->pPosition.z()<zMin) {
        newIndex = (edgeIndex+1)%3;
    } else {
        newIndex = (edgeIndex+2)%3;
    }
    ISEdge *eCutB = isFace->pEdge[newIndex];
    ISVertex *vCutB = eCutB->findZ(zMin);
    if (!vCutB) {
        puts("ERROR: addNextLidVertex failed, no Z point found!");
    }
    vertexList.push_back(vCutB);
    ISEdge *lidEdge = new ISEdge();
    lidEdge->pVertex[0] = vCutA;
    lidEdge->pVertex[1] = vCutB;
    lidEdgeList.push_back(lidEdge);

    vCutA = vCutB;
    isFace = eCutB->otherFace(isFace);
    edgeIndex = eCutB->indexIn(isFace);
}

/*
 Create the edge that cuts this triangle in half.

 The first point is know to be on the z slice. The second edge that crosses
 z is found and the point of intersection is calculated. Then an edge is
 created that splits the face on the z plane.

 \param isFace the face that is split in two; the face must cross zMin
 \param vCutA the first point on zMin along the first edge
 \param edgeIndex the index of the first edge that crosses zMin
 \param zMin slice on this z plane
 */
void ISMeshSlice::addFirstLidVertex(ISFace *isFace, double zMin)
{
    ISFace *firstFace = isFace;
    // find first edge that crosses zMin
    int edgeIndex = -1;
    if (isFace->pVertex[0]->pPosition.z()<zMin && isFace->pVertex[1]->pPosition.z()>=zMin) edgeIndex = 0;
    if (isFace->pVertex[1]->pPosition.z()<zMin && isFace->pVertex[2]->pPosition.z()>=zMin) edgeIndex = 1;
    if (isFace->pVertex[2]->pPosition.z()<zMin && isFace->pVertex[0]->pPosition.z()>=zMin) edgeIndex = 2;
    if (edgeIndex==-1) {
        puts("ERROR: addFirstLidVertex failed, not crossing zMin!");
    }
    ISVertex *vCutA = isFace->pEdge[edgeIndex]->findZ(zMin);
    if (!vCutA) {
        puts("ERROR: addFirstLidVertex failed, no Z point found!");
    }
    vertexList.push_back(vCutA);
    //  addNextLidVertex(isFace, vCutA, edgeIndex, zMin);
    int cc = 0;
    for (;;) {
        addNextLidVertex(isFace, vCutA, edgeIndex, zMin);
        cc++;
        if (isFace->pUsed)
            break;
        isFace->pUsed = true;
    }
    printf("%d edges linked\n", cc);
    if (firstFace==isFace) {
        puts("It's a loop!");
    } else {
        puts("It's NOT a loop!");
    }
    lidEdgeList.push_back(0L);
}

void ISMeshSlice::addZSlice(const ISMesh &m, double zMin)
{
    int i, n = (int)m.faceList.size();
    for (i = 0; i < n; i++) {
        m.faceList[i]->pUsed = false;
    }
    for (i = 0; i < n; i++) {
        ISFace *isFace = m.faceList[i];
        if (isFace->pUsed) continue;
        isFace->pUsed = true;
        int nBelow = isFace->pointsBelowZ(zMin);
        if (nBelow==0) {
            // do nothing
        } else if (nBelow==1) {
            addFirstLidVertex(isFace, zMin);
        } else if (nBelow==2) {
            addFirstLidVertex(isFace, zMin);
        } else if (nBelow==3) {
            // do nothing
        }
    }
}

// -----------------------------------------------------------------------------

static int max_vertices = 0;
static int max_texcos = 0;
static int max_normals = 0;


// -----------------------------------------------------------------------------

float minf(float a, float b) { return (a<b)?a:b; }
float maxf(float a, float b) { return (a>b)?a:b; }


void drawModelGouraud()
{
    int i, n = (int)gMeshList.size();
    for (i=0; i<n; i++) {
        ISMesh *isMesh = gMeshList[i];
        glDepthRange (0.1, 1.0);
        isMesh->drawGouraud();
    }
}


void drawModelFlat(unsigned int color)
{
    int i, n = (int)gMeshList.size();
    for (i=0; i<n; i++) {
        ISMesh *isMesh = gMeshList[i];
        isMesh->drawFlat(color);
    }
}


void drawModelShrunk(unsigned int color, double d)
{
    int i, n = (int)gMeshList.size();
    for (i=0; i<n; i++) {
        ISMesh *isMesh = gMeshList[i];
        isMesh->drawShrunk(color, d);
    }
}

void setShaders() {

    GLuint v, f, p;

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);

    /*
     vec4 v = vec4(gl_Vertex);
     v.z = 0.0;
     gl_Position = gl_ModelViewProjectionMatrix * v;
     */
    const char * vv =
    "void main(void) {\n"
    "    gl_Position = ftransform();\n"
    "}";

    //  "void main()\n"
    //  "{\n"
    //  "  gl_Position = ftransform();\n"
    //  "}\n"
    //  ;

    // if (pixelIsSilly) dicard;
    const char * ff =
    "void main() {\n"
    "    gl_FragColor = vec4( 1, 1, 0, 1);\n"
    "}"
    ;

    glShaderSource(v, 1, &vv,NULL);
    glShaderSource(f, 1, &ff,NULL);

    glCompileShader(v);
    glCompileShader(f);

    p = glCreateProgram();

    glAttachShader(p,v);
    glAttachShader(p,f);

    glLinkProgram(p);
    glUseProgram(p);
}

class MyGLView : public Fl_Gl_Window
{
public:
    MyGLView(int x, int y, int w, int h, const char *l=0)
    : Fl_Gl_Window(x, y, w, h, l)
    {
    }
    void draw()
    {
        static bool firstTime = true;
        if (firstTime) {
            firstTime = false;
            //      setShaders();
        }

        if (!valid()) {
            static GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
            static GLfloat mat_shininess[] = { 50.0 };
            //static GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
            static GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
            static GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1.0};

            gl_font(FL_HELVETICA_BOLD, 16 );

            glClearColor (0.0, 0.0, 0.0, 0.0);
            glShadeModel (GL_SMOOTH);

            glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
            glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position);
            glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);

            glEnable(GL_BLEND);
            //      glBlendFunc(GL_ONE, GL_ZERO);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glViewport(0,0,w(),h());
        }

        double z1 = zSlider1->value();
        double z2 = zSlider2->value();

        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();
        if (gShowSlice) {
            glOrtho(-66.1,66.1,-66.1,66.1, -z1, -z1-z2); // mm
        } else {
            glOrtho(-66.1,66.1,-66.1,66.1,-66.1,66.1); // mm
        }
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();

        if (gShowSlice) {
            // show just the slice
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);
            // draw the model using the z buffer for clipping
            drawModelFlat(FL_RED);
            glMatrixMode (GL_PROJECTION);
            // change the z range to disable clipping
            glLoadIdentity();
            glOrtho(-66.1,66.1,-66.1,66.1,-66.1,66.1); // mm
            glMatrixMode(GL_MODELVIEW);
            gMeshSlice.drawFlat(FL_GREEN);
            gMeshSlice.drawLidEdge();
            // set the z range again to enable drawing the shell
            glMatrixMode (GL_PROJECTION);
            glLoadIdentity();
            glOrtho(-66.1,66.1,-66.1,66.1, -z1, -z1-z2); // mm
            glMatrixMode(GL_MODELVIEW);
            // the following code guarantees a hull of at least 1mm width
            //      double sd;
            //      glHint(GL_POLYGON_SMOOTH_HINT, );
            //      glDisable (GL_POLYGON_SMOOTH);
            //      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            //      for (sd = 0.2; sd<gMinimumShell; sd+=0.2) {
            //        drawModelShrunk(Fl_WHITE, sd);
            //      }
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            // show the 3d model
            glEnable(GL_LIGHTING);
            glEnable(GL_DEPTH_TEST);
            drawModelGouraud();
        }
        glPopMatrix();

        if (gWriteSliceNext==1) {
            gWriteSliceNext = 0;
            writeSlice();
        } else if (gWriteSliceNext==2) {
            gWriteSliceNext = 0;
            writePrnSlice();
        }

        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w(), 0, h(), -10, 10); // mm
        glMatrixMode(GL_MODELVIEW);
        gl_color(FL_WHITE);
        char buf[1024];
        sprintf(buf, "Slice at %.4gmm", z1); gl_draw(buf, 10, 40);
        sprintf(buf, "%.4gmm thick", z2); gl_draw(buf, 10, 20);
    }

    void writeSlice(int nDrops = kNDrops, int interleave=4) {
        printf("# slice at %gmm\n", zSlider1->value());
        // 500 high at 12 pixels = 41 swashes.
        // 500 high at 12 pixels, interleave 4 = 166 swashes.
        int incr = 12/interleave;
        int x, i, n = (h()-11)/incr, ww = w(), row;
        for (i=0; i<n; i++) {
            int nLeft = 0, nFill = 0, nRight = 0;
            printf("Swash %3d: ", i);
            uint32_t *buf = (uint32_t*)malloc(ww*12*4);
            glReadPixels(0, i*incr, ww, 12, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buf);
            // find the first pixel
            for (x=0; x<ww; x++) {
                for (row=0; row<12; row++) {
                    if (buf[x+row*ww]!=0) break;
                }
                if (row<12) break;
            }
            if (x<ww) {
                nLeft = x;
                for (x=ww-1; x>nLeft; x--) {
                    for (row=0; row<12; row++) {
                        if (buf[x+row*ww]!=0) break;
                    }
                    if (row<12) break;
                }
                nRight = ww-x;
                nFill = ww-nLeft-nRight;
                printf("%d / %d /%d", nLeft, nFill, nRight);
                //fprintf(f, "# swash %d\n", i);
#if 0
                int rpt;
                for (rpt = 0; rpt<1; rpt++) {
                    // yGoto
                    writeInt(gOutFile, 147);
                    writeInt(gOutFile, 25536+425*i); // swash height (428)
                                                     // xGoto
                    writeInt(gOutFile, 144);
                    writeInt(gOutFile, 100+36*nLeft); // first pixel
                    for (x=nLeft; x<nLeft+nFill; x++) {
                        uint32_t v = 0;
                        for (row=0; row<12; row++) {
                            v = v<<1;
                            if (buf[x+row*ww]!=0) v |= 1;
                        }
                        // fire pattern
                        writeInt(gOutFile, 1);
                        writeInt(gOutFile, v);
                    }
                }
#else
                // yGoto
                writeInt(gOutFile, 147);
                writeInt(gOutFile, 22000+425*i*incr/12); // swash height (428)
                                                         // xGoto
                writeInt(gOutFile, 144);
                writeInt(gOutFile, 100+36*nLeft); // first pixel
                for (x=nLeft; x<nLeft+nFill; x++) {
                    uint32_t v = 0;
                    for (row=0; row<12; row++) {
                        v = v<<1;
                        if (buf[x+row*ww]!=0) v |= 1;
                    }
                    // fire pattern times n
                    writeInt(gOutFile, 2);
                    writeInt(gOutFile, v);
                    writeInt(gOutFile, nDrops);
                }
#endif
            } else {
                printf("empty");
            }
            free(buf);
            printf("\n");
        }
    }

    /*
     720 dpi
     1440 dpi
     2128 bpl (1064 samples) 3"??
     11904 hgt = 8.2"
     8501 wdt = 11" ?? 27,94cm
     0.299896 paper  paperWdt = wdt / xdpi * 0.0254
     5 colors

     126658592 bytes  size = hgt * bpl * colors!
     */

    void writePrnSlice() {
        printf("# slice at %gmm\n", zSlider1->value());
        char name[2048];
        sprintf(name, "/Users/matt/prn/f%05d.prn", (int)((zSlider1->value()+20)*10));
        FILE *f = fopen(name, "wb");
        // print the header:
        int x, i, n = h(), ww = w(), w2 = 8501;
        struct hdr {
            uint32_t sig, xdpi, ydpi, bpl, hgt, wdt;
            float paperWidth;
            uint32_t nColors, r0, r1, r2, r3;
        };
        struct hdr hdr = {
            0x5555, 720, 1440, (uint32_t)(w()*2), (uint32_t)h()*2, (uint32_t)w2,
            w2/720.0f*0.0254f,
            4, 2, 8, 2, 0 };
        fwrite(&hdr, sizeof(hdr), 1, f);
        // print the color data:
        uint16_t blk = 0xffff, wht = 0x0000;
        for (i=0; i<n; i++) {
            printf("Swash %3d: ", i);
            uint32_t *buf = (uint32_t*)malloc(ww*1*4);
            glReadPixels(0, i, ww, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buf);
            int sw;
            for (sw=0; sw<2; sw++) {
                // C
                for (x=0; x<ww; x++) {
                    fwrite(&wht, sizeof(wht), 1, f);
                }
                // M
                for (x=0; x<ww; x++) {
                    fwrite(&wht, sizeof(wht), 1, f);
                }
                // Y
                for (x=0; x<ww; x++) {
                    fwrite(&wht, sizeof(wht), 1, f);
                }
                // black
                for (x=0; x<ww; x++) {
                    if (buf[x+ww]!=0) {
                        fwrite(&blk, sizeof(blk), 1, f);
                    } else {
                        fwrite(&wht, sizeof(wht), 1, f);
                    }
                }
            }
            free(buf);
            printf("\n");
        }
    }

};

MyGLView *glView = 0L;

double minX = 0.0, maxX = 0.0, minY = 0.0, maxY = 0.0, minZ = 0.0, maxZ = 0.0;

#ifdef SUPPORT_3DS

static double min(double a, double b) { return a<b?a:b; }
static double max(double a, double b) { return a>b?a:b; }

/**
 Load a single node from a 3ds file.
 */
void load3ds(Lib3dsFile *f, Lib3dsMeshInstanceNode *node) {
    float (*orig_vertices)[3];
    int export_texcos;
    int export_normals;
    int i;
    Lib3dsMesh *mesh;

    ISMesh *isMesh = new ISMesh();
    gMeshList.push_back(isMesh);

    mesh = lib3ds_file_mesh_for_node(f, (Lib3dsNode*)node);
    if (!mesh || !mesh->vertices) return;

    orig_vertices = (float(*)[3])malloc(sizeof(float) * 3 * mesh->nvertices);
    memcpy(orig_vertices, mesh->vertices, sizeof(float) * 3 * mesh->nvertices);
    {
        float inv_matrix[4][4], M[4][4];
        float tmp[3];
        int i;

        lib3ds_matrix_copy(M, node->base.matrix);
        lib3ds_matrix_translate(M, -node->pivot[0], -node->pivot[1], -node->pivot[2]);
        lib3ds_matrix_copy(inv_matrix, mesh->matrix);
        lib3ds_matrix_inv(inv_matrix);
        lib3ds_matrix_mult(M, M, inv_matrix);

        //lib3ds_matrix_rotate(M, 90, 1, 0, 0);

        for (i = 0; i < mesh->nvertices; ++i) {
            lib3ds_vector_transform(tmp, M, mesh->vertices[i]);
            lib3ds_vector_copy(mesh->vertices[i], tmp);
        }
    }
    {
        int i;
        for (i = 0; i < mesh->nvertices; ++i) {
            ISVertex *isPoint = new ISVertex();
            isPoint->pPosition.read(mesh->vertices[i]);
            minX = min(minX, isPoint->pPosition.x());
            maxX = max(maxX, isPoint->pPosition.x());
            minY = min(minY, isPoint->pPosition.y());
            maxY = max(maxY, isPoint->pPosition.y());
            minZ = min(minZ, isPoint->pPosition.z());
            maxZ = max(maxZ, isPoint->pPosition.z());
            //isPoint->pPosition *= 10;
            //isPoint->pPosition *= 40; // mokey full size
            //isPoint->pPosition *= 5; // mokey tiny (z=-5...+5)
#ifdef M_MONKEY
            isPoint->pPosition *= 10; // mokey tiny (z=-5...+5)
#elif defined M_DRAGON
            isPoint->pPosition *= 1; // dragon (z=-5...+5)
#endif
            isMesh->vertexList.push_back(isPoint);
        }
    }

    printf("Model bounding box is:\n  x: %g, %g\n  y: %g, %g\n  z: %g, %g\n",
           minX, maxX, minY, maxY, minZ, maxZ);
    /*
     Monkey Model bounding box is:
     x: -1.36719, 1.36719
     y: -0.984375, 0.984375
     z: -0.851562, 0.851562
     Iota wants mm, so scale by 40, resulting in a 112mm wide head.
     */

    export_texcos = (mesh->texcos != 0);
    export_normals = (mesh->faces != 0);

    //  for (i = 0; i < mesh->nvertices; ++i) {
    //    fprintf(o, "v %f %f %f\n", mesh->vertices[i][0],
    //            mesh->vertices[i][1],
    //            mesh->vertices[i][2]);
    //  }
    //  fprintf(o, "# %d vertices\n", mesh->nvertices);

    //  if (export_texcos) {
    //    for (i = 0; i < mesh->nvertices; ++i) {
    //      fprintf(o, "vt %f %f\n", mesh->texcos[i][0],
    //              mesh->texcos[i][1]);
    //    }
    //    fprintf(o, "# %d texture vertices\n", mesh->nvertices);
    //  }

    //  if (export_normals) {
    //    float (*normals)[3] = (float(*)[3])malloc(sizeof(float) * 9 * mesh->nfaces);
    //    lib3ds_mesh_calculate_vertex_normals(mesh, normals);
    //    for (i = 0; i < 3 * mesh->nfaces; ++i) {
    //      fprintf(o, "vn %f %f %f\n", normals[i][0],
    //              normals[i][1],
    //              normals[i][2]);
    //    }
    //    free(normals);
    //    fprintf(o, "# %d normals\n", 3 * mesh->nfaces);
    //  }

    {
        //    int mat_index = -1;
        for (i = 0; i < mesh->nfaces; ++i) {

            //      if (mat_index != mesh->faces[i].material) {
            //        mat_index = mesh->faces[i].material;
            //        if (mat_index != -1) {
            //          fprintf(o, "usemtl %s\n", f->materials[mat_index]->name);
            //        }
            //      }
            //      fprintf(o, "f ");
            //      for (j = 0; j < 3; ++j) {
            ISFace *isFace = new ISFace();
            isFace->pVertex[0] = isMesh->vertexList[mesh->faces[i].index[0]];
            isFace->pVertex[1] = isMesh->vertexList[mesh->faces[i].index[1]];
            isFace->pVertex[2] = isMesh->vertexList[mesh->faces[i].index[2]];
            //      isFace->print();
            isMesh->addFace(isFace);
            //        fprintf(o, "%d", mesh->faces[i].index[j] + max_vertices + 1);
            //        int vi;
            //        float *fv;
            //        vi = mesh->faces[i].index[j];
            //        fv = mesh->vertices[vi];
            //        glVertex3fv(fv);
            //        if (export_texcos) {
            //          fprintf(o, "/%d", mesh->faces[i].index[j] + max_texcos + 1);
            //        } else if (export_normals) {
            //          fprintf(o, "/");
            //        }
            //        if (export_normals) {
            //          fprintf(o, "/%d", 3 * i + j + max_normals + 1);
            //        }
            //        if (j < 3) {
            //          fprintf(o, " ");
            //        }
            //      }
            //      fprintf(o, "\n");
        }
    }

    max_vertices += mesh->nvertices;
    if (export_texcos)
        max_texcos += mesh->nvertices;
    if (export_normals)
        max_normals += 3 * mesh->nfaces;

    memcpy(mesh->vertices, orig_vertices, sizeof(float) * 3 * mesh->nvertices);
    free(orig_vertices);

    isMesh->validate();
    // TODO: fix seams
    // TODO: fix zero size holes
    // TODO: fix degenrate triangles
    isMesh->fixHoles();
    isMesh->validate();

    isMesh->clearNormals();
    isMesh->calculateNormals();
}

#endif


int getShort(FILE *f) {
    int ret = 0;
    ret |= fgetc(f);
    ret |= fgetc(f)<<8;
    return ret;
}

int getInt(FILE *f) {
    int ret = 0;
    ret |= fgetc(f);
    ret |= fgetc(f)<<8;
    ret |= fgetc(f)<<16;
    ret |= fgetc(f)<<24;
    return ret;
}

float getFloat(FILE *f) {
    float ret;
    fread(&ret, 4, 1, f);
    return ret;
}

int addPoint(ISMesh *isMesh, float x, float y, float z)
{
    int i, n = (int)isMesh->vertexList.size();
    for (i = 0; i < n; ++i) {
        ISVertex *v = isMesh->vertexList[i];
        if (   v->pPosition.x()==x
            && v->pPosition.y()==y
            && v->pPosition.z()==z)
        {
            return i;
        }
    }
    ISVertex *v = new ISVertex();
    v->pPosition.set(x, y, z);
    isMesh->vertexList.push_back(v);
    return n;
}

void clearSTL()
{
  ISMesh *isMesh = gMeshList.at(0);
  gMeshList.erase(gMeshList.begin());
  delete isMesh;
}


/**
 Load a single node from a binary stl file.
 */
void loadSTL(const char *filename) {
    int i;

    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "ERROR openening file!\n");
        return;
    }
    fseek(f, 0x50, SEEK_SET);
    ISMesh *isMesh = new ISMesh(filename);
    gMeshList.push_back(isMesh);

    int nFaces = getInt(f);
    for (i=0; i<nFaces; i++) {
        float x, y, z;
        int p1, p2, p3;
        // face normal
        getFloat(f);
        getFloat(f);
        getFloat(f);
        // point 1
        x = getFloat(f);
        y = getFloat(f);
        z = getFloat(f);
        p1 = addPoint(isMesh, x, y, z);
        // point 2
        x = getFloat(f);
        y = getFloat(f);
        z = getFloat(f);
        p2 = addPoint(isMesh, x, y, z);
        // point 3
        x = getFloat(f);
        y = getFloat(f);
        z = getFloat(f);
        p3 = addPoint(isMesh, x, y, z);
        // add face
        ISFace *isFace = new ISFace();
        isFace->pVertex[0] = isMesh->vertexList[p1];
        isFace->pVertex[1] = isMesh->vertexList[p2];
        isFace->pVertex[2] = isMesh->vertexList[p3];
        isMesh->addFace(isFace);
        // color
        getShort(f);
    }

    isMesh->validate();
    // TODO: fix seams
    // TODO: fix zero size holes
    // TODO: fix degenrate triangles
    isMesh->fixHoles();
    isMesh->validate();

    isMesh->clearNormals();
    isMesh->calculateNormals();

    fclose(f);
}


/**
 Load a model from a 3ds file.
 */
#ifdef SUPPORT_3DS
void load3ds(const char *filename)
{
    Lib3dsFile *f = lib3ds_file_open(filename);
    if (!f->nodes)
        lib3ds_file_create_nodes_for_meshes(f);
    lib3ds_file_eval(f, 0);
    Lib3dsNode *p;
    for (p = f->nodes; p; p = p->next) {
        if (p->type == LIB3DS_NODE_MESH_INSTANCE) {
            load3ds(f, (Lib3dsMeshInstanceNode*)p);
        }
    }
    lib3ds_file_free(f);
}
#endif

static void xButtonCB(Fl_Widget*, void*)
{
    gShowSlice = false;
    glView->redraw();
}

static void sliceCB(Fl_Widget*, void*)
{
    gMeshSlice.clear();
    int i, n = (int)gMeshList.size();
    for (i=0; i<n; i++) {
        ISMesh *isMesh = gMeshList[i];
        gMeshSlice.addZSlice(*isMesh, zSlider1->value());
        gMeshSlice.tesselate();
    }
    glView->redraw();
}

static void z1ChangedCB(Fl_Widget*, void*)
{
    sliceCB(0, 0);
    gShowSlice = true;
    glView->redraw();
}

static void z2ChangedCB(Fl_Widget*, void*)
{
    gShowSlice = true;
    glView->redraw();
}

static void writeSliceCB(Fl_Widget*, void*)
{
    double z;
#ifdef M_MONKEY
    double firstLayer  = -8.8;
    double lastLayer   =  9.0;
    double layerHeight =  0.1;
    gOutFile = fopen("/Users/matt/monkey.3dp", "wb");
#elif defined M_DRAGON
    double firstLayer  = -20.0;
    double lastLayer   =  14.0;
    double layerHeight =   0.1;
    gOutFile = fopen("/Users/matt/dragon.3dp", "wb");
#endif
    // header
    writeInt(gOutFile, 23);  // Magic
    writeInt(gOutFile, 3);
    writeInt(gOutFile, 2013);
    writeInt(gOutFile, 1);   // File Version
    writeInt(gOutFile, 159); // total number of layers
    writeInt(gOutFile,  (lastLayer-firstLayer)/layerHeight );

    for (z=firstLayer; z<=lastLayer; z+=layerHeight) {
        // spread powder
        writeInt(gOutFile, 158);
        writeInt(gOutFile,  10); // spread 0.1mm layers
                                 // render the layer
        zSlider1->value(z);
        zSlider1->do_callback();
        gWriteSliceNext = 1;
        glView->redraw();
        glView->flush();
        Fl::flush();
    }
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    fclose(gOutFile);
    fprintf(stderr, "/Users/matt/monkey.3dp");
}


static void writePrnSliceCB(Fl_Widget*, void*)
{
    double z;
#ifdef M_MONKEY
    double firstLayer  = -8.8;
    double lastLayer   =  9.0;
    double layerHeight =  0.1;
#elif defined M_DRAGON
    double firstLayer  = -20.0;
    double lastLayer   =  14.0;
    double layerHeight =   0.1;
#endif

    for (z=firstLayer; z<=lastLayer; z+=layerHeight) {
        zSlider1->value(z);
        zSlider1->do_callback();
        gWriteSliceNext = 2;
        glView->redraw();
        glView->flush();
        Fl::flush();
    }
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    fclose(gOutFile);
    fprintf(stderr, "/Users/matt/monkey.3dp");
}


int main_xx (int argc, char **argv)
{
    /*
     500 pixles at 96dpi = 5in = 13cm
     at 96dpi, 1 dot is 0.26mm in diameter
     */

    Fl_Window win(840, 800, "Iota Slice");
    win.begin();
    Fl_Group *g = new Fl_Group(0, 0, 800, 800);
    g->begin();
    glView = new MyGLView(150, 150, 500, 500);
    g->end();
    zSlider1 = new Fl_Slider(800, 0, 20, 720);
    zSlider1->tooltip("Position of the slice in Z\n-66 to +66 millimeters");
    zSlider1->range(30, -30);
    zSlider1->step(0.25);
    zSlider1->callback(z1ChangedCB);
    zSlider2 = new Fl_Slider(820, 0, 20, 720);
    zSlider2->tooltip("Slice thickness\n-10 to +10 millimeters");
    zSlider2->range(10, -10);
    zSlider2->value(0.25);
    zSlider2->callback(z2ChangedCB);
    Fl_Button *b = new Fl_Button(800, 680, 40, 40, "X");
    b->callback(xButtonCB);
    b = new Fl_Button(800, 720, 40, 40, "Write");
    b->callback(writeSliceCB);
    b = new Fl_Button(800, 760, 40, 40, ".prn");
    b->callback(writePrnSliceCB);
    win.end();
    win.resizable(g);
    win.show(argc, argv);
    glView->show();
    Fl::flush();
#ifdef M_MONKEY
    load3ds("/Users/matt/Desktop/Machine Shop/Machine Pwdr/lib3ds-20080909/monkey.3ds");
#elif defined M_DRAGON
    loadSTL("/Users/matt/Desktop/Machine Shop/Machine Pwdr/0.02_dragon_2.stl");
#endif
    //load3ds("/Users/matt/squirrel/NewSquirrel.3ds");
    //load3ds("/Users/matt/Desktop/Machine Shop/Machine Pwdr/0.02_dragon_2.3ds");
    glView->redraw();
    return Fl::run();
}
