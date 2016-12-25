//
//  ETOpenGLWidget.hpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 10/21/15.
//  Copyright © 2015 M.Melcher GmbH. All rights reserved.
//

#ifndef ETOpenGLWidget_hpp
#define ETOpenGLWidget_hpp

#include <FL/Fl_Gl_Window.H>

class ETOpenGLWidget : public Fl_Gl_Window
{
public:
    ETOpenGLWidget(int x, int y, int w, int h);
    ~ETOpenGLWidget();
    int handle(int event);
    void draw();
    void draw_callback(void (*cb)()) { pDrawCallback = cb; }
    void drop_callback(void (*cb)(const char*)) { pDropCallback = cb; }
    void drawMode2d(bool v) { pDrawMode2D = v; redraw(); }
private:
    void draw2D();
    void draw3D();
    void (*pDrawCallback)();
    void (*pDropCallback)(const char *filenames);
    double pXRotation = 5.0, pYRotation = -20.0; // rotation in deg
    double pXOffset = 0.0, pYOffset = -15.0, pZOffset=-140.0; // camera offset in mm
    bool pDrawMode2D;
};


#endif /* ETOpenGLWidget_hpp */
