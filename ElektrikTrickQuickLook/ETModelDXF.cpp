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


/**
 * \return no error code, it fails silently, but creates an intact (possibly incomplete) dataset
 */
int ETModelDXF::Load()
{
//    float x0, y0, z0, x1, y1, z1;
//    NEdge = 8192;
//    edge = (ETEdge*)calloc(NEdge, sizeof(ETEdge));
//    
//    char buf[1024];
//    
//    for (;;) {
//        if (FEof()) break;
//        buf[0] = 0;
//        FGetS(buf, 1023);
//        if (strcmp(buf, "LINE")==0) {
//            int gCode = atoi(buf+1);
//            if (gCode==0 || gCode==1) { // move
//                if (NEdge==nEdge) {
//                    NEdge += 8192;
//                    edge = (ETEdge*)realloc(edge, NEdge*sizeof(ETEdge));
//                }
//                ETEdge &e = edge[nEdge];
//                e.p0.set(cx, cy, cz);
//                char *arg = strchr(buf+2, 'X');
//                if (arg) cx = (absPos?0:cx) + atof(arg+1);
//                arg = strchr(buf+2, 'Y');
//                if (arg) cy = (absPos?0:cx) + atof(arg+1);
//                arg = strchr(buf+2, 'Z');
//                if (arg) cz = (absPos?0:cx) + atof(arg+1);
//                e.p1.set(cx, cy, cz);
//                e.attr = 0;
//                if (gCode==1) e.attr |= 1;
//                // ignore the first segments if they move from the origin
//                if (ignoreFirst) {
//                    if (e.p0.x!=0.0 || e.p0.y!=0.0 || e.p0.z!=0.0)
//                        ignoreFirst = false;
//                } else {
//                    nEdge++;
//                }
//            } else switch (gCode) {
//                case 90: // Set to Absolute Positioning
//                    absPos = true; break;
//                case 91: // Set to Relative Positioning
//                    absPos = false; break;
//                default:
//                    break;
//            }
//        }
//    }
//    // ignore the last segment. On some machines it moves away from the model.
//    if (nEdge>0)
//        nEdge--;
    return 1;
}

