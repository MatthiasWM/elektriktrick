#ifndef ETIMPORTSTL_H
#define ETIMPORTSTL_H

#include "ETImport.h"

class ETModel;


typedef char *ETString;


class ETImportSTL : public ETImport
{
public:
    ETImportSTL(const char *filename);
    virtual int read(ETModel *mdl);
protected:
    char* FGetS(char *dst, int size);
    int FEof();
    int Find(ETString &src, const char *key);
    int LoadTextSTL();
    int LoadBinarySTL();
    int IsBinary();

    ETModel *pModel;
};


#endif // ETIMPORTSTL_H
