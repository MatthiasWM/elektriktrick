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
#include "FL/Fl_Tile.H"
#include "FL/Fl_Menu_Bar.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_File_Browser.H"
#include "FL/filename.h"

#include "ETSerialPort.h"
#include "ETOpenGLWidget.h"
#include "ETFileBrowser.h"

#include "ETOpenGLSurface.h"

#include "ETGMesh.h"

#include "ETModel.h"
#include "ETModelSTL.h"



const char *gMainTitle = "Elektriktrick";
const char *gMainTitleAndFile = "Elektriktrick - %s";

bool gDrawMode2D = true;

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
ETFileBrowser *gFileBrowser;
ETOpenGLWidget *gRenderWidget;

char gBasePath[FL_PATH_MAX] = "";

ETModel *gPreviewModel = 0L;



void quitCB(Fl_Widget*, void*)
{
    gMainWindow->hide();
}


Fl_Menu_Item mainMenuTable[] =
{
    { "File", 0, 0, 0, FL_SUBMENU },
    {   "Quit", FL_COMMAND+'q', quitCB, 0, 0 },
    {   0 },
//    { "Edit" },
//    { "Help" },
    { 0 }
};


void loadFile(const char *filename)
{
    if (gPreviewModel) {
        ETModel *mdl = gPreviewModel;
        gPreviewModel = 0L;
        delete mdl;
    }

    if (!filename) {
        gMainWindow->copy_label(gMainTitle);
        return;
    }

    ETModel *model = ETModel::ModelForFileType(filename);
    if (model) {
        if (model->Load()) {
            if (gDrawMode2D) {
                model->Prepare2DDrawing();
            } else {
                model->Prepare3DDrawing();
            }
            gPreviewModel = model;
        }
    }
    gRenderWidget->redraw();

    char buf[FL_PATH_MAX];
    snprintf(buf, FL_PATH_MAX, gMainTitleAndFile, fl_filename_name(filename));
    gMainWindow->copy_label(buf);

    // FIXME: we are keeping track of the path twice: in gFileBroswer and in gBasePath! Don't do that!
}


void drop_cb(const char *filenames)
{
    loadFile(filenames);
}


void draw_cb()
{
    if (gPreviewModel) {
        if (gDrawMode2D) {
            gPreviewModel->GLDraw2D(gRenderWidget->pixel_w(), gRenderWidget->pixel_h());
        } else {
            gPreviewModel->GLDraw3D();
        }
    } else {
        drawModelGouraud();
    }
}


void file_select_cb(const char *filename)
{
    loadFile(gFileBrowser->full_path());
}


void directory_select_cb(const char *directory)
{
    chdir(gBasePath);
    fl_filename_absolute(gBasePath, FL_PATH_MAX, directory);
    gFileBrowser->load(gBasePath);
}


void directory_up_cb(const char *directory)
{
    chdir(gBasePath);
    fl_filename_absolute(gBasePath, FL_PATH_MAX, directory);
    gFileBrowser->load(gBasePath);
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

 // TODO: QuickView has an "Open With" button in the top right corner. Should we add that too?

 */
int main (int argc, char **argv)
{
    ReadPreferences();
    Fl::args(argc, argv);
    Fl::set_font(0, "Helvetice Neu"); // TODO: does not work as expected
    fl_open_callback(drop_cb);
    Fl_File_Icon::load_system_icons();

    gMainWindow = new Fl_Window(gMainWindowX, gMainWindowY,
                                gMainWindowW, gMainWindowH,
                                gMainTitle);

    Fl_Menu_Bar *mainMenu = new Fl_Menu_Bar(0, 0, gMainWindow->w(), 25);
    mainMenu->menu(mainMenuTable);

    Fl_Tile *mainTile = new Fl_Tile(0, mainMenu->h(), gMainWindow->w(), gMainWindow->h()-mainMenu->h());

    gFileBrowser = new ETFileBrowser(0, mainMenu->h(), gFileBrowserW, gMainWindow->h()-mainMenu->h());
    gFileBrowser->type(FL_HOLD_BROWSER);
    gFileBrowser->filetype(Fl_File_Browser::FILES);
    gFileBrowser->iconsize(20);
    gFileBrowser->filter("[^.]*{.stl|.dxf|.gcode|.bfb}");
    gFileBrowser->directory_filter("[^.]*");
    gFileBrowser->file_select_callback(file_select_cb);
    gFileBrowser->directory_select_callback(directory_select_cb);
    gFileBrowser->directory_up_callback(directory_up_cb);

    gRenderWidget = new ETOpenGLWidget(gFileBrowserW, mainMenu->h(), gMainWindow->w()-gFileBrowserW, gMainWindow->h()-mainMenu->h());
    gRenderWidget->draw_callback(draw_cb);
    gRenderWidget->drop_callback(drop_cb);
    gRenderWidget->drawMode2d(gDrawMode2D);
    gRenderWidget->end();

    mainTile->end();
    // TODO: with a 'resizable' widget, we can limit the resizing of gMainWindow

    gMainWindow->resizable(mainTile);
    gMainWindow->show();

    // The function below uses the advanced geometry system in ETGMesh.h/.cxx
    // loadSTL("filename.stl");

    // TODO: load the file given from the command line
    // TODO: or load the file given via Open message
    // TODO: change the path accordingly
    // TODO: direct path entry
    // TODO: other OS X specific things: http://www.fltk.org/doc-1.3/group__group__macosx.html#ga0702a54934d10f5b72157137cf291296

    gFileBrowser->load(gBasePath);

    Fl::run();

    WritePreferences();

    return 0;
}
