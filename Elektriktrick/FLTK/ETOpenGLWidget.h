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
private:
    double pXRotation = 5.0, pYRotation = -20.0; // rotation in deg
    double pXOffset = 0.0, pYOffset = -15.0, pZOffset=-80.0; // camera offset in mm
};


#endif /* ETOpenGLWidget_hpp */
