#include "ETImportSTL.h"
#include "ETModel.h"
#include "ETTriangle.h"
#include "ETEdge.h"
#include "ETVertex.h"
#include "ETVector.h"

ETImportSTL::ETImportSTL(const char *filename) :
    ETImport(filename),
    pModel(0L)
{
}


int ETImportSTL::read(ETModel *mdl)
{
    pModel = mdl;

    int ret = 0;
    // read the file content into memory
    ret = IsBinary();
    if (ret==-1) {
        // ERROR, can't determine the file type (too short, etc.)
        return -1;
    }
    if (ret==1) {
        ret = LoadBinarySTL();
    } else {
        ret = LoadTextSTL();
    }
    mdl->createEdgeList();
    mdl->verifyIntegrity();
    return ret;
}

/**
 * Check if the open STL file is in binary or text format.
 *
 * This is not as easy as it seem. Although there are rules for recognizing
 * a text file (it should start with the string "solid") there are many export
 * scripts out there that do not comply.
 *
 * Our strategy is to check a list of indicators until we have reliable
 * information.
 */
int ETImportSTL::IsBinary()
{
    // check if there is an open file descriptor
    if (!pFile)
        return -1;

    // read a sufficient amount of data for guessing the file type
    if (pFilesize<84) // a file that is smaller than 84 bytes can not contain an STL model
        return -1;
    uint8_t buf[84];
    fseek(pFile, 0, SEEK_SET);
    fread(buf, 84, 1, pFile);
    fseek(pFile, 0, SEEK_SET);

    // first test: is there a 0 or a non-ASCII character?
    // Those would not appear in a text file.
    int i;
    for (i=0; i<84; i++) {
        if (buf[i]==0 || buf[i]>127)
            return 1;
    }

    // actually, I have no other smart test at thispoint in time.
    // So let's assume that this is a text file
    return 0;
}


/**
 * Find the first occurence of an ASCII string inside a text.
 *
 * Used for text based STL files.
 */
int ETImportSTL::Find(ETString &src, const char *key)
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
char* ETImportSTL::FGetS(char *dst, int size)
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
int ETImportSTL::FEof()
{
    return feof(pFile);
}


// ASCII FORMAT (ca. 140 characters per triangle)
//              solid name
//                facet normal ni nj nk
//                  outer loop
//                    vertex v1x v1y v1z
//                    vertex v2x v2y v2z
//                    vertex v3x v3y v3z
//                  endloop
//                endfacet
//              endsolid name
// Usually the line deliminator is 0x0A, but I found files that use 0x0D.
// Likely, there are files as well that use 0x0D followed by 0x0A.


/**
 * Text mode STL reader.
 *
 * The reader should be ablet to read some information even when the fil is
 * truncated or otherwise broken without crashing.
 *
 * \returns no error code, it fails silently, but creates an intact (possibly incomplete) dataset
 */
int ETImportSTL::LoadTextSTL()
{
    char buf[1024];

    pModel->pTriList.reserve(2048);
    // read "solid name" (we already verified that);
    for (;;) {
        if (FEof()) break;
        FGetS(buf, 1023);
        char *src = buf;
        if (Find(src, "solid")) break;
    }

    ETTriangle *t = 0L;
    for (;;) {
        ETVector p0, p1, p2;
        char *src = buf;
        for (;;) {
            if (FEof()) break;
            buf[0] = 0;
            FGetS(buf, 1023); src = buf;
            // read "facet normal ni nj nk"
            if (Find(src, "facet normal")) break;
        }
        if (!t) t = new ETTriangle();
        if (sscanf(src, "%f %f %f", &t->n.x, &t->n.y, &t->n.z)!=3) break;
        if (!t->n.isFinite()) continue;
        // read "outer loop"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "outer loop")) break;
        // read "vertex x y z"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "vertex")) break;
        if (sscanf(src, "%f %f %f", &p0.x, &p0.y, &p0.z)!=3) break;
        if (!p0.isFinite()) continue;
        // read "vertex x y z"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "vertex")) break;
        if (sscanf(src, "%f %f %f", &p1.x, &p1.y, &p1.z)!=3) break;
        if (!p1.isFinite()) continue;
        // read "vertex x y z"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "vertex")) break;
        if (sscanf(src, "%f %f %f", &p2.x, &p2.y, &p2.z)!=3) break;
        if (!p2.isFinite()) continue;
        // read "outer loop"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "endloop")) break;
        // read "outer loop"
        FGetS(buf, 1023); src = buf;
        if (!Find(src, "endfacet")) break;
        // don't increment the counter until we are sure that the triangle is read.
        t->v0 = pModel->findOrAddVertex(p0);
        t->v1 = pModel->findOrAddVertex(p1);
        t->v2 = pModel->findOrAddVertex(p2);
        pModel->pTriList.push_back(t); t = 0L;
    }
    if (t)
        delete t;

    return 0;
}


/**
 * Load a binary STL file.
 *
 * FIXME: file may be truncated! Make sure we don;t read beyond EOF.
 * FIXME: file may conatin NaN's. Make sure to weed those out as they may cause exceptions and slow us down tremendously.
 * FIXME: make sure that bytes are swapped everywhere for big endian machines (file format is little endian):
 *          CFSwapInt32LittleToHost(), CFConvertFloat64SwappedToHost()
 */
int ETImportSTL::LoadBinarySTL()
{
    fseek(pFile, 80, SEEK_SET);

    // number of triangles
    uint32_t i, nTriFileSize=0, nTriFileInfo=0, nTri=0;
    // get the number of triangles in the file, as claimed by the numTri fied
    fread(&nTriFileInfo, 1, 4, pFile);
    // get the maximum number of readble triangles based on the file size
    nTriFileSize = ((uint32_t)pFilesize-84) / 50;
    if (nTriFileSize < nTriFileInfo) {
        // the file may have been truncated
        nTri = nTriFileSize;
    } else {
        nTri = nTriFileInfo;
    }
    pModel->pTriList.reserve(nTri);

    ETTriangle *t = 0L;
    for (i=0; i<nTri; ++i) {
        if (!t) t = new ETTriangle();
        ETVector p0, p1, p2;
        if (fread(&t->n, sizeof(float), 3, pFile)!=3) break;
        t->n.fixFinite();
        if (fread(&p0, sizeof(float), 3, pFile)!=3) break;
        p0.fixFinite();
        if (fread(&p1, sizeof(float), 3, pFile)!=3) break;
        p1.fixFinite();
        if (fread(&p2, sizeof(float), 3, pFile)!=3) break;
        p2.fixFinite();
        if (fread(&t->attr, sizeof(uint16_t), 1, pFile)!=1) break;
        t->v0 = pModel->findOrAddVertex(p0);
        t->v1 = pModel->findOrAddVertex(p1);
        t->v2 = pModel->findOrAddVertex(p2);
        pModel->pTriList.push_back(t); t = 0L;
    }
    if (t)
        delete t;
    return 0;
}
