#include "ETImport.h"
#include "ETImportSTL.h"
#include "ETModel.h"

#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int ETImport::load(ETModel *mdl, const char *filename)
{
    ETImportSTL loader(filename);
    int ret = loader.read(mdl);
    mdl->PrepareDrawing();
    return ret;
}


ETImport::ETImport(const char *filename) :
    pFilename(0L),
    pFile(0L),
    pFilesize(0)
{
    // make sure taht there is a filename
    if (!filename || !*filename) {
        // ERROR, no filename given
        return;
    }
    pFilename = strdup(filename);

    // get the status of the given file. We need the file size.
    struct stat st;
    int ret = stat(filename, &st);
    if (ret==-1) {
        // ERROR, can't get file status
        return;
    }
    pFilesize = st.st_size;

    // open the file for reading in a buffered file descriptor
    pFile = fopen(filename, "rb");
    if (!pFile) {
        // ERROR, can't open file for reading
        return;
    }
}


ETImport::~ETImport()
{
    if (pFile)
        fclose(pFile);
    if (pFilename)
        free(pFilename);
}


int ETImport::read(ETModel *)
{
    return -1;
}
