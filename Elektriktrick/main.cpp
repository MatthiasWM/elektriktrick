//
//  main.cpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 9/29/15.
//  Copyright © 2015 M.Melcher GmbH. All rights reserved.
//

#include "main.h"

#include "FL/Fl.H"
#include "FL/Fl_Preferences.H"
#include "FL/Fl_Window.H"
#include "FL/Fl_Pack.H"
#include "FL/Fl_Menu_Bar.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_File_Browser.H"
#include "FL/filename.h"

#include "ETSerialPort.h"
#include "ETOpenGLWidget.h"

#include "ETOpenGLSurface.h"

#include "ETGMesh.h"

#include "ETModel.h"
#include "ETModelSTL.h"


const int gMainWindowDefaultX = 100;
const int gMainWindowDefaultY = 120;
const int gMainWindowDefaultW = 800;
const int gMainWindowDefaultH = 600;
const int gFileBrowserDefaultW = 200;

int gMainWindowX;
int gMainWindowY;
int gMainWindowW;
int gMainWindowH;
int gFileBrowserW;

Fl_Window *gMainWindow;
Fl_File_Browser *gFileBrowser;

char gBasePath[FL_PATH_MAX] = "";



ETOpenGLWidget *ogl;

ETModel *gPreviewModel = 0L;


void quitCB(Fl_Widget*, void*)
{
    gMainWindow->hide();
}


void shrinkAndSaveCB(Fl_Widget*, void*)
{
    if (gMeshList.size()==0)
        return;
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


void file_browser_cb(Fl_Widget *w, void*)
{
    const char *name = gFileBrowser->text(gFileBrowser->value());
    if (!name) return;

    char buf[FL_PATH_MAX];
    strcpy(buf, gBasePath);
    strcat(buf, "/");
    strcat(buf, gFileBrowser->text(gFileBrowser->value()));

    if (fl_filename_isdir(buf)) {
        chdir(gBasePath);
        fl_filename_absolute(gBasePath, FL_PATH_MAX, name);
        gFileBrowser->load(gBasePath);
    } else {
        loadFile(buf);
    }
}


void ReadPreferences()
{
    Fl_Preferences prefs(Fl_Preferences::USER, "com.matthiasm.elektriktrick", "elektriktrick");

    Fl_Preferences screen(prefs, "screen");
    screen.get("x", gMainWindowX, gMainWindowDefaultX);
    screen.get("y", gMainWindowY, gMainWindowDefaultY);
    screen.get("w", gMainWindowW, gMainWindowDefaultW);
    screen.get("h", gMainWindowH, gMainWindowDefaultH);

    Fl_Preferences fileBrowser(prefs, "fileBrowser");
    fileBrowser.get("w", gFileBrowserW, gFileBrowserDefaultW);
    fileBrowser.get("path", gBasePath, fl_getenv("HOME"), FL_PATH_MAX);
}


void WritePreferences()
{
    Fl_Preferences prefs(Fl_Preferences::USER, "com.matthiasm.elektriktrick", "elektriktrick");

    Fl_Preferences screen(prefs, "screen");
    screen.set("x", gMainWindow->x());
    screen.set("y", gMainWindow->y());
    screen.set("w", gMainWindow->w());
    screen.set("h", gMainWindow->h());

    Fl_Preferences fileBrowser(prefs, "fileBrowser");
    fileBrowser.set("w", gFileBrowser->w());
    fileBrowser.set("path", gBasePath);
}


/**
 
 +-----------------------------------------------------+
 | File  Edit  Help                                    |
 +-----------------------------------------------------+
 | Filebrowser |      3D Model Redering                |
 :             :                                       :

 :             :                                       :
 +-----------------------------------------------------+


 */
int main (int argc, char **argv)
{
    Fl::args(argc, argv);
    ReadPreferences();

    gMainWindow = new Fl_Window(gMainWindowX, gMainWindowY,
                                gMainWindowW, gMainWindowH,
                                "Elektriktrick");

    Fl_Menu_Bar *mainMenu = new Fl_Menu_Bar(0, 0, gMainWindow->w(), 25);
    mainMenu->menu(mainMenuTable);

    Fl_Group *toolLine = new Fl_Group(0, gMainWindow->h()-20, gMainWindow->w(), 20);
    toolLine->box(FL_DOWN_BOX);
    {
        ETSerialPort *ser = new ETSerialPort(gMainWindow->w()-52, gMainWindow->h()-18, 50, 16);
        ser->open("/dev/tty.usb1411", 19200);
    }
    toolLine->end();

    Fl_Pack *machineBar = new Fl_Pack(gMainWindow->w()-32, mainMenu->h(), 32, gMainWindow->h()-mainMenu->h()-toolLine->h());
    {
        Fl_Button *b = new Fl_Button(0, 0, 32, 32, "fix\n&&\nsave");
        b->labelsize(8);
        b->callback(shrinkAndSaveCB);
        b->tooltip("Fix model for FDM print and save with '_fit' extension");
    }
    machineBar->end();

    gFileBrowser = new Fl_File_Browser(0, mainMenu->h(), gFileBrowserW, gMainWindow->h()-mainMenu->h()-toolLine->h());

    ogl = new ETOpenGLWidget(gFileBrowserW, mainMenu->h(), gMainWindow->w()-machineBar->w()-gFileBrowserW, gMainWindow->h()-mainMenu->h()-toolLine->h());
    ogl->draw_callback(draw_cb);
    ogl->drop_callback(drop_cb);
    ogl->end();

    gMainWindow->resizable(ogl);
    gMainWindow->show();
    gMainWindow->show();

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

    gFileBrowser->type(FL_HOLD_BROWSER);
    gFileBrowser->filetype(Fl_File_Browser::FILES);
    gFileBrowser->iconsize(20);
    gFileBrowser->filter("[^.]*{.stl|.dxf|.gcode}");
    gFileBrowser->load(gBasePath);
    gFileBrowser->callback(file_browser_cb);


//    Fl::add_timeout(3, test);
    Fl::run();

    WritePreferences();

    return 0;
}
