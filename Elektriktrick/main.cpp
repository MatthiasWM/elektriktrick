//
//  main.cpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 9/29/15.
//  Copyright © 2015 M.Melcher GmbH. All rights reserved.
//

#include "main.h"

#include "FL/Fl.H"
#include "FL/Fl_Window.H"
#include "FL/Fl_Pack.H"
#include "FL/Fl_Menu_Bar.H"
#include "FL/Fl_Button.H"

#include "ETSerialPort.h"
#include "ETOpenGLWidget.h"

#include "ETOpenGLSurface.h"

#include "ETGMesh.h"


#include "ETModel.h"
#include "ETModelSTL.h"


ETOpenGLWidget *ogl;
Fl_Window *qMainWindow;

ETModel *gPreviewModel = 0L;


void quitCB(Fl_Widget*, void*)
{
    qMainWindow->hide();
}


void shrinkAndSaveCB(Fl_Widget*, void*)
{
    ISMesh *isMesh = gMeshList.at(0);
    isMesh->shrink(0.15, 0.15, 0.0);
//    isMesh->shrink(1.0, 1.0, 0.0);
    isMesh->saveCopy("_fix");
    ogl->redraw();
}


Fl_Menu_Item mainMenuTable[] =
{
    { "File", 0, 0, 0, FL_SUBMENU },
    {   "Quit", FL_COMMAND+'q', quitCB, 0, 0 },
    {   0 },
    { "Edit" },
    { "Help" },
    { 0 }
};


void test(void*)
{
    ETOpenGLSurface srf(800, 600);
}


void dropSTL(const char *filename)
{
    clearSTL();
    loadSTL(filename);
    ogl->redraw();
}


void loadFile(const char *filename)
{
    if (gPreviewModel) {
        ETModel *mdl = gPreviewModel;
        gPreviewModel = 0L;
        delete mdl;
    }
    ETModel *model = ETModel::ModelForFileType(filename);
    if (model) {
        if (model->Load()) {
            model->Prepare3DDrawing();
            gPreviewModel = model;
        }
    }
    ogl->redraw();
}


void drop_cb(const char *filenames)
{
//    dropSTL(filenames);
    loadFile(filenames);
}


void draw_cb()
{
    if (gPreviewModel) {
        gPreviewModel->GLDraw3D();
    } else {
        drawModelGouraud();
    }
}


int main (int argc, char **argv)
{
//    Polyhedron P;
//    Halfedge_handle h = P.make_tetrahedron();

    Fl::args(argc, argv);
    Fl_Window *win = qMainWindow = new Fl_Window(800, 500, "Elektriktrick");

    Fl_Menu_Bar *mainMenu = new Fl_Menu_Bar(0, 0, win->w(), 25);
    mainMenu->menu(mainMenuTable);

    Fl_Group *toolLine = new Fl_Group(0, win->h()-20, win->w(), 20);
    toolLine->box(FL_DOWN_BOX);
    {
        ETSerialPort *ser = new ETSerialPort(win->w()-52, win->h()-18, 50, 16);
        ser->open("/dev/tty.usb1411", 19200);
    }
    toolLine->end();

    Fl_Pack *machineBar = new Fl_Pack(win->w()-32, mainMenu->h(), 32, win->h()-mainMenu->h()-toolLine->h());
    {
        Fl_Button *b = new Fl_Button(0, 0, 32, 32, "fix\n&&\nsave");
        b->labelsize(8);
        b->callback(shrinkAndSaveCB);
        b->tooltip("Fix model for FDM print and save with '_fit' extension");
    }
    machineBar->end();

    ogl = new ETOpenGLWidget(0, mainMenu->h(), win->w()-machineBar->w(), win->h()-mainMenu->h()-toolLine->h());
    ogl->draw_callback(draw_cb);
    ogl->drop_callback(drop_cb);
    ogl->end();

    win->resizable(ogl);
    win->show();
    ogl->show();

#if 0

//    loadStl("/Users/matt/Desktop/Machine Shop/Machine Pwdr/0.02_dragon_2.stl");
//    loadStl("/Users/matt/Desktop/Machine Shop/Data 3d/ETCalibrate_v2_x02_y01.stl");
//    loadStl("/Users/matt/Desktop/Machine Shop/Data 3d/yoda-figure.stl");
//    loadStl("/Users/matt/Desktop/Machine Shop/Data 3d/trunicos40mm.stl");
//    loadStl("/Users/matt/female.stl");
    loadSTL("/Users/matt/Desktop/Machine Shop/Project InMoov/WeVolver/Bicep_for_Robot_InMoov/SpacerV1.stl");

#else 

//    ETModel *model = ETModel::ModelForFileType("/Users/matt/Desktop/Machine Shop/Project InMoov/WeVolver/Bicep_for_Robot_InMoov/SpacerV1.stl");
    loadFile("/Users/matt/test.dxf");

#endif


//    Fl::add_timeout(3, test);
    Fl::run();
    return 0;
}
