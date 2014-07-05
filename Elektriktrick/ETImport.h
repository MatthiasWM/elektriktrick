#ifndef ETIMPORT_H
#define ETIMPORT_H


#include <stdio.h>

class ETModel;


class ETImport
{
public:
    /**
     * @brief Loads a model form local storage.
     * This function finds the best importer and imports the
     * data from the fileinto the current model.
     * @param mdl
     * @param filename
     * @return -1, if the operation failed
     */
    static int load(ETModel *mdl, const char *filename);

public:
    ETImport(const char *filename);
    virtual ~ETImport();
    virtual int read(ETModel *mdl);

protected:
    char *pFilename;
    FILE *pFile;
    off_t pFilesize;
};

#endif // ETIMPORT_H
