#include "ETApp.h"
#include "ETModel.h"
#include "ETImport.h"

#include "glwidget.h"


ETApp ET;


ETApp::ETApp() :
    pModel(0L),
    pGLWidget(0L)
{
}


ETModel *ETApp::newModel()
{
    if (pModel)
        delete pModel;
    pModel = new ETModel();
    redrawScene();
    return pModel;
}


ETModel *ETApp::loadModel(const char *filename)
{
    ETModel *mdl = newModel();
    ETImport::load(mdl, filename);
    redrawScene();
    return pModel;
}


void ETApp::redrawScene()
{
    if (pGLWidget)
        pGLWidget->updateGL();
}
