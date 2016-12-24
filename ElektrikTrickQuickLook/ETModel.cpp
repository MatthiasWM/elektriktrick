//
//  ETModel.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETModel.h"
#include "ETModelTextSTL.h"
#include "ETModelBinarySTL.h"
#include "ETModelGCode.h"
#include "ETModelDXF.h"

#include "ETTriangle.h"
#include "ETVector.h"

#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


/**
 * Determine the format of a file and return the matching empty model.
 *
 * \return NULL if the file type is not supported.
 */
ETModel *ETModel::ModelForFileType(const char *filename)
{
    // make sure that there is a filename
    if (!filename || !*filename) {
        // ERROR, no filename given
        fprintf(stderr, "ElektriktrickQL: no filename\n");
        return 0;
    }

    // get the status of the given file. We need the file size.
    struct stat st;
    int ret = stat(filename, &st);
    if (ret==-1) {
        // ERROR, can't get file status
        fprintf(stderr, "ElektriktrickQL: no file status for %s\n", filename);
        return 0;
    }
    
    // open the file for reading in a buffered file descriptor
    FILE *file = fopen(filename, "rb");
    if (!file) {
        // ERROR, can't open file for reading
        fprintf(stderr, "ElektriktrickQL: can't open file %s for reading\n", filename);
        return 0;
    }
    
    uint8_t buf[4096];
    fseek(file, 0, SEEK_SET);
    size_t size = fread(buf, 1, 4096, file);
    fseek(file, 0, SEEK_SET);
    
    ETModel *mdl = 0L;
    
    if (!mdl) {
        mdl = ETModelBinarySTL::Create(buf, size);
        if (mdl) fprintf(stderr, "ElektriktrickQL: binary STL file\n");
    }
    if (!mdl) {
        mdl = ETModelTextSTL::Create(buf, size);
        if (mdl) fprintf(stderr, "ElektriktrickQL: text STL file\n");
    }
    if (!mdl) {
        mdl = ETModelGCode::Create(buf, size);
        if (mdl) fprintf(stderr, "ElektriktrickQL: GCode file\n");
    }
    if (!mdl) {
        mdl = ETModelDXF::Create(buf, size);
        if (mdl) fprintf(stderr, "ElektriktrickQL: DXF file\n");
    }
    if (mdl) {
        mdl->pFile = file;
        mdl->pFilename = strdup(filename);
        mdl->pFilesize = st.st_size;
    } else {
        fprintf(stderr, "ElektriktrickQL: unrecognized file type\n");
    }
    return mdl;
}


bool ETModel::FileIsBinarySTL(uint8_t *buf, size_t size)
{
    if (size<84)
        return false;
    int i;
    for (i=0; i<84; i++) {
        if (buf[i]==0 || buf[i]>127)
            return true;
    }
    return false;
}


bool ETModel::FileIsTextSTL(uint8_t *buf, size_t size)
{
    if (size<5)
        return false;
    if (strncmp((char*)buf, "solid", 5)==0)
        return true;
    return false;
}


/**
 * Create an empty 3D model.
 */
ETModel::ETModel() :
    pFilename(0L),
    pFile(0),
    pFilesize(0),
    pBoundingBoxKnown(false)
{
}


/**
 * Delet a model and all datasets asssociated with it.
 */
ETModel::~ETModel()
{
    if (pFilename)
        free(pFilename);
    if (pFile)
        fclose(pFile);
}


/**
 * Find the first occurence of an ASCII string inside a text.
 *
 * Used for text based STL files.
 */
int ETModel::Find(ETString &src, const char *key)
{
    for (;;) {
        char c = *src;
        if (c!=' ' && c!='\t')
            break;
        src++;
    }
    size_t n = strlen(key);
    if (strncmp(src, key, n)!=0)
        return 0;
    char c = src[n];
    if (c==' ' || c=='\t') {
        src = src + n + 1;
        return 1;
    }
    if (c=='\r' || c=='\n' || c==0) {
        src = src + n + 1;
        return 1;
    }
    return 0;
}


/**
 * This function fixes fgets() to be aware of alterntive line endings.
 *
 * After reading thousands of STL files, I have seen it all :-( . Mmost have
 * the Posix '\n' line ending, but some have "\r\n", especially when they
 * were loaded into a MSWindows text editor. I also found output from
 * "SolidWorks" that uses the MacOS '\r'-only ending from the 90'ies.
 *
 * This function strips the line ending and fixes the seek() position if the
 * line ending was not found by fgets().
 */
char* ETModel::FGetS(char *dst, int size)
{
    off_t pos = ftell(pFile);
    char *str = fgets(dst, size, pFile);
    if (str) {
        char *s = str;
        for (;;) {
            char c = *s;
            // check if we reached the end of the file - that's ok
            if (c==0)
                break;
            // check to see if this is a CR character
            if (c=='\r') {
                // in MSDOS, a LF will follow, which is correctly handles by fgets
                if (s[1]=='\n' && s[2]==0) {
                    *s = 0;
                    break;
                }
                // or, if this was the last character in the file, we are fine, too
                if (s[1]==0) {
                    *s = 0;
                    break;
                }
                // oops, it's the old MacOS CR only style! Go ahead and fix the file
                // position to point to the character following the CR
                *s = 0;
                fseek(pFile, pos + s-str+1, SEEK_SET);
                break;
            }
            // this is the standard Unix line ending
            if (c=='\n') {
                // this should only happen if fgets() is broken. Fix it.
                if (s[1]!=0) {
                    *s = 0;
                    fseek(pFile, pos + s-str+1, SEEK_SET);
                    break;
                }
                *s = 0;
                break;
            }
            s++;
        }
    }
    return str;
}


/**
 * feof() function for alternative file reader.
 */
int ETModel::FEof()
{
    return feof(pFile);
}

/**
 * Roate a vector slightly around y and x to give an orthogonal view from the top right.
 */
void ETModel::SimpleProjection(ETVector &v)
{
    const float ry = 180.0+30.0; // deg
    float rys = sinf(ry/180.0*M_PI);
    float ryc = cosf(ry/180.0*M_PI);
    
    const float rx = 30.0; // deg
    float rxs = sinf(rx/180.0*M_PI);
    float rxc = cosf(rx/180.0*M_PI);
    
    float x = -v.x;
    float y = v.z;
    float z = -v.y;
    
    // rotate around the vertical a little to the left
    v.x = x*ryc + z*rys;
    v.y = y;
    v.z = z*ryc - x*rys;
    x = v.x; y = v.y; z = v.z;
    
    // now rotate around the x axis a bit so we see a depth
    v.x = x;
    v.y = y*rxc + z*rxs;
    v.z = z*rxc - y*rxs;
    
    // let's keep it at that, so that we have an orthagonal projection
}







