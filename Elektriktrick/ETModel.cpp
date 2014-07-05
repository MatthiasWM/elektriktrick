//
//  ETModel.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETModel.h"

#include "ETTriangle.h"
#include "ETVector.h"
#include "ETVertex.h"
#include "ETEdge.h"

#include "glwidget.h"

#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <stdio.h>

#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



/** 
 * Create an empty 3D model.
 */
ETModel::ETModel() :
    pFirstVertex(0L),
    pFirstEdge(0L),
    pFilename(0L),
    pFile(0),
    pFilesize(0)
{
    sortedTri = 0L;
    nSortedTri = 0;
}


/**
 * Delet a model and all datasets asssociated with it.
 */
ETModel::~ETModel()
{
    uint32_t i, n = pTriList.size();
    for (i=0; i<n; i++) {
        ETTriangle *t = pTriList[i];
        delete t;
    }
    //if (tri)
    //    free(tri);
    if (sortedTri)
        free(sortedTri);
    if (pFilename)
        free(pFilename);
    if (pFile)
        fclose(pFile);
    // TODO: delete vertices
    // TODO: delete edges
}


/**
 * Find the bounding box and reposition coordinates to fit into a -1/1 box.
 */
void ETModel::FindBoundingBox()
{
    float minX=0, minY=0, minZ=0, maxX=0, maxY=0, maxZ=0;
    ETVertex *v = pFirstVertex;
    while (v) {
        if (v==pFirstVertex) {
            minX = maxX = v->p.x;
            minY = maxY = v->p.y;
            minZ = maxZ = v->p.z;
        }
        // update the boundign box
        if (v->p.x<minX) minX = v->p.x; if (v->p.x>maxX) maxX = v->p.x;
        if (v->p.y<minY) minY = v->p.y; if (v->p.y>maxY) maxY = v->p.y;
        if (v->p.z<minZ) minZ = v->p.z; if (v->p.z>maxZ) maxZ = v->p.z;
        v = v->pNext;
    }
    pBBoxMin.set(minX, minY, minZ);
    pBBoxMax.set(maxX, maxY, maxZ);
}



int ETModel::FixupCoordinates()
{
    ETVertex *v = pFirstVertex;
    while (v) {
        float tmp;
        v->p.x = -v->p.x;
        tmp = v->p.y; v->p.y = v->p.z; v->p.z = tmp;
        v = v->pNext;
    }
    return 0;
}


/**
 * Calculate all face normals from scratch.
 *
 * This function uses the direction of the triangle coordinate system. It does
 * not verify or fix flipped normals.
 */
int ETModel::GenerateFaceNormals()
{
    uint32_t i, n = pTriList.size();
    for (i=0; i<n; ++i) {
        ETTriangle *t = pTriList[i];
        ETVector u; u.set(t->v1->p); u.sub(t->v0->p);
        ETVector v; v.set(t->v2->p); v.sub(t->v0->p);
        t->n.x = (u.y*v.z) - (u.z*v.y);
        t->n.y = (u.z*v.x) - (u.x*v.z);
        t->n.z = (u.x*v.y) - (u.y*v.x);
        t->n.normalize();
    }
    return 0;
}


/**
 * Create point normals based on connected face normals multiplied by the corner angle.
 */
int ETModel::GenerateVertexNormals()
{
    uint32_t i;

    ETVertex *v = pFirstVertex;
    while (v) {
        v->n.zero();
        v->nMin.zero(); v->nMinN = 0;
        v->nMax.zero(); v->nMaxN = 0;
        v = v->pNext;
    }

    uint32_t n = pTriList.size();
    for (i=0; i<n; ++i) {
        ETTriangle *t = pTriList[i];
#if 0
        // This is a very simple approach. By just taking the average of all
        // face normals, we do not take into account that larger triangles
        // should have more impact on the normal than small triangles
        t->v0->n.add(t->n);
        t->v1->n.add(t->n);
        t->v2->n.add(t->n);
#endif
#if 1
        // This approach weighs the normals by the angle of the triangle that provides
        // the normals. This makes for a more even object in 3D rendering.
        ETVector u, v, n;
        float a;
        // handle vertex 0
        u.set(t->v1->p); u.sub(t->v0->p); u.normalize();
        v.set(t->v2->p); v.sub(t->v0->p); v.normalize();
        a = acos(u.x*v.x + u.y*v.y + u.z*v.z);
        n.set(t->n); n.mul(a);
        t->v0->n.add(n);
        // handle vertex 1
        u.set(t->v0->p); u.sub(t->v1->p); u.normalize();
        v.set(t->v2->p); v.sub(t->v1->p); v.normalize();
        a = acos(u.x*v.x + u.y*v.y + u.z*v.z);
        n.set(t->n); n.mul(a);
        t->v1->n.add(n);
        // handle vertex 2
        u.set(t->v0->p); u.sub(t->v2->p); u.normalize();
        v.set(t->v1->p); v.sub(t->v2->p); v.normalize();
        a = acos(u.x*v.x + u.y*v.y + u.z*v.z);
        n.set(t->n); n.mul(a);
        t->v2->n.add(n);
#endif
#if 1
        // This approach generates the maxima in each direction. The generated normals
        // are bad for rendering the object, but great for compensating filament
        // diameter issues.
        t->v0->nMin.min(t->n);
        t->v0->nMax.max(t->n);
        t->v1->nMin.min(t->n);
        t->v1->nMax.max(t->n);
        t->v2->nMin.min(t->n);
        t->v2->nMax.max(t->n);
#endif
    }

    v = pFirstVertex;
    while (v) {
#if 0
        v->n.normalize();
#endif
#if 1
        v->n.set(v->nMax);
        v->n.add(v->nMin);
#endif
        v = v->pNext;
    }

    return 0;
}



void ETModel::Draw(void*, int, int)
{
    uint32_t i, n;

    glPushMatrix();
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (1., 1.);

    // draw surface
    glEnable(GL_LIGHTING);
    glColor3f(0.8f,0.8f,0.8f);
    glBegin(GL_TRIANGLES);
    n = pTriList.size();
    for (i=0; i<n; ++i) {
        ETTriangle *t = pTriList[i];
        glNormal3fv((const GLfloat*)&t->n.x);
        glVertex3fv((const GLfloat*)&t->v0->p.x);
        glVertex3fv((const GLfloat*)&t->v1->p.x);
        glVertex3fv((const GLfloat*)&t->v2->p.x);
    }
    glEnd();

    // draw edges
    glDisable(GL_LIGHTING);
    ETEdge *e = pFirstEdge;
    glBegin(GL_LINES);
#if 1
    while(e) {
        switch(e->pNTriangle) {
        case 0: glColor3f(1.0f,1.0f,0.0f); break;
        case 1: glColor3f(0.0f,1.0f,0.0f); break;
        case 2: glColor3f(0.0f,0.0f,0.0f); break;
        default: glColor3f(1.0f,0.0f,0.0f); break;
        }
        glVertex3fv((const GLfloat*)&e->v0->p.x);
        glVertex3fv((const GLfloat*)&e->v1->p.x);
        e = e->pNext;
    }
#endif
    glEnd();

    // draw normals

    glDisable(GL_COLOR_MATERIAL);
    glPopMatrix();
}

int ETModel::verifyIntegrity()
{
    uint32_t i, n;

    int nEdge = 0, nVertice = 0;

    int verticesPerTriError = 0;
    int doubleVertexError = 0;
    int edgesPerTriError = 0;
    int doubleEdgeError = 0;
    int noTriPerEdgeError = 0;
    int singleTriPerEdgeError = 0;
    int tooManyTrisPerEdgeError = 0;
    int doubleTriPerEdgeError = 0;
    int doubleVertexPerEdgeError = 0;

    // check if all triangles have three vertices and three edges
    n = pTriList.size();
    for (i=0; i<n; i++) {
        ETTriangle *t = pTriList[i];
        if (t->v0==0L || t->v1==0L || t->v2==0L) verticesPerTriError++;
        if (t->v0==t->v1 || t->v0==t->v2 || t->v1==t->v2) doubleVertexError++;
        if (t->e0==0L || t->e1==0L || t->e2==0L) edgesPerTriError++;
        if (t->e0==t->e1 || t->e0==t->e2 || t->e1==t->e2) doubleEdgeError++;
    }

    // check all edges for valid number of connected triangles
    ETEdge *e = pFirstEdge;
    while (e) {
        if (e->pNTriangle==0) noTriPerEdgeError++;
        if (e->pNTriangle==1) singleTriPerEdgeError++;
        if (e->pNTriangle>=3) tooManyTrisPerEdgeError++;
        if (e->t0==e->t1) doubleTriPerEdgeError++;
        if (e->v0==e->v1) doubleVertexPerEdgeError++;
        nEdge++;
        e = e->pNext;
    }

    // check vertices
    ETVertex *v = pFirstVertex;
    while (v) {
        nVertice++;
        v = v->pNext;
    }

    if (verticesPerTriError)
        printf("ERROR: %d triangles with less than three vertices\n", verticesPerTriError);
    if (doubleVertexError)
        printf("ERROR: %d triangles with duplicate vertices\n", doubleVertexError);
    if (edgesPerTriError)
        printf("ERROR: %d triangles with less than three edges\n", edgesPerTriError);
    if (doubleEdgeError)
        printf("ERROR: %d triangles with duplicate edges\n", doubleEdgeError);

    if (noTriPerEdgeError)
        printf("ERROR: %d edges with no triangle\n", noTriPerEdgeError);
    if (singleTriPerEdgeError)
        printf("ERROR: %d edgese with only a single triangle\n", singleTriPerEdgeError);
    if (tooManyTrisPerEdgeError)
        printf("ERROR: %d edges with more than two triangles\n", tooManyTrisPerEdgeError);
    if (doubleTriPerEdgeError)
        printf("ERROR: %d edges with duplicate triangles\n", doubleTriPerEdgeError);
    if (doubleVertexPerEdgeError)
        printf("ERROR: %d edges with duplicate vertices\n", doubleVertexPerEdgeError);

    int nErr = verticesPerTriError + doubleVertexError + edgesPerTriError + doubleEdgeError
            + noTriPerEdgeError + singleTriPerEdgeError + tooManyTrisPerEdgeError + doubleTriPerEdgeError + doubleVertexPerEdgeError;
    printf("%d errors in model (%d vertices, %d edges, %d triangles)\n", nErr, nVertice, nEdge, pTriList.size());

    fflush(stdout);

    return nErr;
}




void ETModel::PrepareDrawing()
{
    // prepare the polygon data for rendering
    FindBoundingBox();
    FixupCoordinates();
    GenerateFaceNormals();
    GenerateVertexNormals();
}


ETVertex *ETModel::findOrAddVertex(const ETVector &pIn)
{
    ETVertex *v = pFirstVertex;
    while (v) {
        if (v->p.x==pIn.x && v->p.y==pIn.y && v->p.z==pIn.z)
            return v;
        v = v->pNext;
    }
    v = new ETVertex;
    v->p.set(pIn);
    v->pNext = pFirstVertex;
    pFirstVertex = v;
    return v;
}


void ETModel::createEdgeList()
{
    uint32_t i, n = pTriList.size();
    for (i=0; i<n; ++i) {
        ETTriangle *t = pTriList[i];
        t->e0 = findOrAddEdge(t->v1, t->v2, t);
        t->e1 = findOrAddEdge(t->v2, t->v0, t);
        t->e2 = findOrAddEdge(t->v0, t->v1, t);
    }
}


ETEdge *ETModel::findOrAddEdge(ETVertex *v0, ETVertex *v1, ETTriangle *t)
{
    ETEdge *e = pFirstEdge;
    while (e) {
        if (  ((e->v0==v0)&&(e->v1==v1))
           || ((e->v0==v1)&&(e->v1==v0)) ) {
            e->pNTriangle++;
            return e;
        }
        e = e->pNext;
    }
    e = new ETEdge;
    e->v0 = v0;
    e->v1 = v1;
    e->pNTriangle++;
    switch (e->pNTriangle) {
    case 1: e->t0 = t; break;
    case 2: e->t1 = t; break;
    }
    e->pNext = pFirstEdge;
    pFirstEdge = e;
    return e;
}


void ETModel::calibrate(float xGrow, float yGrow, float zGrow, float xScale, float yScale, float zScale)
{
    ETVertex *v = pFirstVertex;
    while (v) {
        // shrink in x and z
        // this is needed if the string layed down is thicker than anticipated
        ETVector xzNormal = v->n;   // get the point normal
        xzNormal.x *= xGrow;      // fix the x shrink in x direction
        xzNormal.y = 0.0;           // forget the y component (up/down)
        xzNormal.z *= zGrow;      // fix the z shrink in z direction
        v->p.add(xzNormal);
        // shrinking in z makes not much sense

        v = v->pNext;
    }
}

int ETModel::SaveAs()
{
    uint32_t i, n;
    QString f = QFileDialog::getSaveFileName(0L, "Save STL file:",
                                             "", "STL (*.stl)");
    if (!f.isEmpty()) {
        FILE *out = fopen(f.toUtf8().data(), "wb");
        if (!out) {
            QMessageBox::warning(0L, "ERROR creating file", "Can't cretae file for writing.");
            return -1;
        }
        const char hdr[80] = "STL file created with Elektriktrick.";
        fwrite(hdr, 80, 1, out);
        n = pTriList.size();
        fwrite(&n, 4, 1, out);
        for (i=0; i<n; i++) {
            ETTriangle *t = pTriList[i];
            float tmp;
            ETVector v;
            v.set(t->n); v.x = -v.x; tmp = v.y; v.y = v.z; v.z = tmp;
            fwrite(&v.x, 4, 3, out);
            v.set(t->v0->p); v.x = -v.x; tmp = v.y; v.y = v.z; v.z = tmp;
            fwrite(&v.x, 4, 3, out);
            v.set(t->v1->p); v.x = -v.x; tmp = v.y; v.y = v.z; v.z = tmp;
            fwrite(&v.x, 4, 3, out);
            v.set(t->v2->p); v.x = -v.x; tmp = v.y; v.y = v.z; v.z = tmp;
            fwrite(&v.x, 4, 3, out);
            uint16_t attr = 0;
            fwrite(&attr, 2, 1, out);
        }
        fclose(out);
    }
    return 0;
}
