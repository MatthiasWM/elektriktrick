#include "ETApp.h"
#include "ETModel.h"
#include "ETImport.h"

#include "glwidget.h"



/*
 * Measurements of the standard calibration object on my machine:
 * X: 10.1, 20.1, 30.2
 * Y:  9.9, 19.9, 30.0
 * Z: 10.1, 20.0, 30.0
 */



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
    mdl->Filename(filename);
    ETImport::load(mdl, filename);
    redrawScene();
    return pModel;
}


void ETApp::redrawScene()
{
    if (pGLWidget)
        pGLWidget->updateGL();
}
