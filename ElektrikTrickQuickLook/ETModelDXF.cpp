//
//  ETModelDXF.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETModelDXF.h"
#include "ETEdge.h"
#include "ETVector.h"


ETModel *ETModelDXF::Create(uint8_t *buf, size_t size)
{
    // 999
    // dxflib 2.2.0.0
    //   0
    // SECTION
    //   2
    // HEADER
    //   9
/*
 0
 SECTION
 2
 ENTITIES
 0
 CIRCLE
 
 
 0
 LINE
 5
 BB
 100
 AcDbEntity
 100
 AcDbLine
 8
 0
 62
 256
 370
 -1
 6
 ByLayer
 10
 392.6296817685332599
 20
 22.5559351767177603
 30
 0.0
 11
 393.3911687115805194
 21
 22.5902010829989699
 31
 0.0

*/
    return new ETModelDXF();
    // gcode lines evetually start with "M#" or "G#", where # is some decimal integer
    signed int i;
    for (i=0; i<size-4; i++) {
        if (i==0 || buf[i-1]=='\r' || buf[i-1]=='\n') {
            if (buf[i]=='G' || buf[i]=='M') {
                if (isdigit(buf[i+1])) return new ETModelDXF();
            }
        }
    }
    return 0L;
}


ETModelDXF::ETModelDXF()
:   ETWireframeModel()
{
}


ETModelDXF::~ETModelDXF()
{
}


int ETModelDXF::NextCode(int &code, char *data)
{
    char codeBuf[1024];
    codeBuf[0] = 0;
    data[0] = 0;
    code = -1;
    if (FEof()) return 0;
    FGetS(codeBuf, 1023);
    code = atoi(codeBuf);
    if (FEof()) return 0;
    FGetS(data, 1023);
    return 1;
}

enum {
    kEntityNone = 0,
    kEntityLINE,
    kEntityARC,
    kEntityLWPOLYLINEFirst,
    kEntityLWPOLYLINESecond,
    kEntityLWPOLYLINENext,
    kEntityPOLYLINEFirst,
    kEntityPOLYLINESecond,
    kEntityPOLYLINENext,
    kEntitySPLINEFirst,
    kEntitySPLINESecond,
    kEntitySPLINENext
};

/**
 * \return no error code, it fails silently, but creates an intact (possibly incomplete) dataset
 */
int ETModelDXF::Load()
{
    NEdge = 8192;
    edge = (ETEdge*)calloc(NEdge, sizeof(ETEdge));
    
    int code;
    char data[1024];
    
    // skip to 0:SECTION 2:ENTITIES
    for (;;) {
        if (!NextCode(code, data)) return 0;
        if (code==0 && strcmp(data, "SECTION")==0) {
            if (!NextCode(code, data)) return 0;
            if (code==2 && strcmp(data, "ENTITIES")==0) {
                break;
            }
        }
    }
    
    // read all know geometries in the ENTITIES section
    int currentEntity = 0;
    bool done = false;
    ETVector p0, p1, pPrev, pFirst;
    float r = 1.0;
    float a0 = 0.0, a1 = 360.0;
    float bulge = 0.0, prevBulge = 0.0;
    unsigned int flags = 0;
    while (!done) {
        if (!NextCode(code, data)) return 0;
        // when a new entity starts, wrap up the previous entity
        if (code==0) {
            switch (currentEntity) {
                case kEntityNone: break; // no entity
                case kEntityLINE: { // LINE
                    ETEdge *e = NewEdge();
                    e->p0.set(p0);
                    e->p1.set(p1);
                    }
                    currentEntity = kEntityNone;
                    break;
                case kEntityARC: // ARC
                    NewArc(p0, r, a0, a1);
                    currentEntity = kEntityNone;
                    break;
                case kEntityLWPOLYLINENext: // LWPOLYLINE_next
                    NewSegment(pPrev, p0, prevBulge);
                    if (flags & 1)
                        NewSegment(p0, pFirst, bulge);
                    currentEntity = kEntityNone;
                    break;
                case kEntitySPLINENext: // SPLINE_next
                    if (flags & 1) { // loop
                        NewSplineSegment(pPrev, p0);
                        NewLastSplineSegment(p0, pFirst);
                    } else {
                        NewLastSplineSegment(pPrev, p0);
                    }
                    currentEntity = kEntityNone;
                    break;
                case kEntityPOLYLINEFirst: // POLYLINE_first
                case kEntityPOLYLINESecond: // POLYLINE_second
                case kEntityPOLYLINENext: // POLYLINE_next
                    break;
                default:
                    currentEntity = kEntityNone;
                    break;
            }
        }
        if (code==10) {
            // possibly a new LWPOLYLINE or SPLINE segment is starting
            switch (currentEntity) {
                case kEntityNone: break; // no entity
                case kEntityLWPOLYLINEFirst: // LWPOLYLINE_first
                    currentEntity = kEntityLWPOLYLINESecond;
                    break;
                case kEntityLWPOLYLINESecond: // LWPOLYLINE_second
                    currentEntity = kEntityLWPOLYLINENext;
                    prevBulge = bulge;
                    bulge = 0;
                    pPrev.set(p0);
                    pFirst.set(p0);
                    break;
                case kEntityLWPOLYLINENext: // LWPOLYLINE_next
                    NewSegment(pPrev, p0, prevBulge);
                    prevBulge = bulge;
                    bulge = 0;
                    pPrev.set(p0);
                    break;
                case kEntitySPLINEFirst: // SPLINE_first
                    currentEntity = kEntitySPLINESecond;
                    break;
                case kEntitySPLINESecond: // SPLINE_second
                    currentEntity = kEntitySPLINENext;
                    pPrev.set(p0);
                    pFirst.set(p0);
                    break;
                case kEntitySPLINENext: // SPLINE_next
                    NewSplineSegment(pPrev, p0);
                    pPrev.set(p0);
                    break;
            }
        }
        switch (code) {
            case 0:
                if (strcmp(data, "ENDSEC")==0) {
                    done = true;
                } else if (strcmp(data, "LINE")==0) {
                    // LINE: x=10, y=20, z=30, x=11, y=21, z=31
                    currentEntity = kEntityLINE;
                } else if (strcmp(data, "ARC")==0) {
                    // ARC: x=10, y=20, z=30, r=40, 50=start angle, 51=end angle
                    currentEntity = kEntityARC;
                } else if (strcmp(data, "CIRCLE")==0) {
                    // CIRCLE: x=10, y=20, z=30, r=40
                    a0 = 0.0; a1 = 360.0;
                    currentEntity = kEntityARC;
                } else if (strcmp(data, "LWPOLYLINE")==0) {
                    // LWPOLYLINE: x=10, y=20, z=30, bulge=42
                    bulge = prevBulge = 0;
                    flags = 0;
                    currentEntity = kEntityLWPOLYLINEFirst;
                } else if (strcmp(data, "SPLINE")==0) {
                    flags = 0;
                    currentEntity = kEntitySPLINEFirst;
                } else if (strcmp(data, "POLYLINE")==0) {
                    flags = 0;
                    currentEntity = kEntityPOLYLINEFirst;
                } else if (strcmp(data, "VERTEX")==0) {
                    if (currentEntity==kEntityPOLYLINEFirst) {
                        currentEntity = kEntityPOLYLINESecond;
                    } else if (currentEntity==kEntityPOLYLINESecond) {
                        currentEntity = kEntityPOLYLINENext;
                        pPrev.set(p0);
                        pFirst.set(p0);
                    }
                    else {
                        NewSplineSegment(pPrev, p0);
                        pPrev.set(p0);
                    }
                } else if (strcmp(data, "SEQEND")==0) {
                    if (currentEntity==kEntityPOLYLINENext) {
                        if (flags & 1) { // close polygon
                            NewSplineSegment(pPrev, p0);
                            NewLastSplineSegment(p0, pFirst);
                        } else {
                            NewLastSplineSegment(pPrev, p0);
                        }
                    }
                    currentEntity = 0;
                } else {
                    printf("Unsupported entity %s\n", data);
                }
                break;
            case 10: p0.x = atof(data); break;
            case 11: p1.x = atof(data); break;
            case 20: p0.y = atof(data); break;
            case 21: p1.y = atof(data); break;
            case 30: p0.z = atof(data); break;
            case 31: p1.z = atof(data); break;
            case 40: r = atof(data); break;
            case 42: bulge = atof(data); break;
            case 50: a0 = atof(data); break;
            case 51: a1 = atof(data); break;
            case 70: flags = atoi(data); break;
        }
    }
    return 1;
}

