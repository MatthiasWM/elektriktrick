//
//  ETOpenGLWidget.cpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 10/21/15.
//  Copyright © 2015 M.Melcher GmbH. All rights reserved.
//

#include "ETOpenGLWidget.h"

#include <math.h>
#include <Fl/gl.h>
#include <Fl/glu.h>
#include <Fl/FL.h>

#include "ETGMesh.h"


ETOpenGLWidget::ETOpenGLWidget(int x, int y, int w, int h)
:   pDrawCallback(0),
    pDropCallback(0),
    Fl_Gl_Window(x, y, w, h)
{
    Fl::use_high_res_GL(true);
    mode(
         FL_RGB8 |
         FL_DOUBLE |
         FL_DEPTH |
         FL_MULTISAMPLE
    );
}


ETOpenGLWidget::~ETOpenGLWidget()
{
}


int ETOpenGLWidget::handle(int event)
{
    // Native QuickLook for MacOS uses these key combinations:
    // LMB: drag to rotate on a trackball
    // LMB-Alt: drag model around left/right/up/down
    // Scrollwheel: drag model around
    // ScrollVert+Shift: move camara in Z (Horizontal is blocked)
    // ScrollVert+Alt: zomm camera (lens angle), horizontal drags horizontally

    switch (event) {
        case FL_MOUSEWHEEL:
            if (Fl::event_state(FL_SHIFT|FL_CTRL|FL_META|FL_ALT)==FL_SHIFT) {
                pXOffset += Fl::event_dx();
                pYOffset += Fl::event_dy();
                redraw();
            } else if (Fl::event_state(FL_SHIFT|FL_CTRL|FL_META|FL_ALT)==FL_META) {
                pZOffset += 0.2*Fl::event_dy();
                redraw();
            } else {
                pXRotation += Fl::event_dx();
                pYRotation += Fl::event_dy();
                redraw();
            }
            break;
        case FL_DND_ENTER:
            return 1;
        case FL_DND_DRAG:
            return 1;
        case FL_DND_RELEASE:
            return 1;
        case FL_PASTE:
            if (pDropCallback) pDropCallback(Fl::event_text());
            break;
    }
    return Fl_Gl_Window::handle(event);
}


void ETOpenGLWidget::draw()
{
    static bool firstTime = true;
    if (firstTime) {
        firstTime = false;
        //      setShaders();
    }

    if (!valid()) {
        static GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
        static GLfloat mat_shininess[] = { 50.0 };
        //static GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
        static GLfloat light_position[] = { 1.0, -5.0, 3.0, 0.0 };
        static GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1.0};

        gl_font(FL_HELVETICA_BOLD, 16 );

        glClearColor (0.0, 0.0, 0.0, 0.0);
        glShadeModel (GL_SMOOTH);

        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

        glEnable(GL_LIGHT0);
        glEnable(GL_NORMALIZE);

        glEnable(GL_BLEND);
        //      glBlendFunc(GL_ONE, GL_ZERO);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        glEnable(GL_LINE_SMOOTH); glEnable(GL_POLYGON_SMOOTH); glEnable(GL_POINT_SMOOTH);
        glEnable( GL_MULTISAMPLE );

        glViewport(0,0,pixel_w(),pixel_h());
        valid(1);
    }

    glClearColor(0.4, 0.4, 0.4, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    double z1 = zSlider1->value();
//    double z2 = zSlider2->value();

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glViewport(0,0,pixel_w(),pixel_h());
//    if (gShowSlice) {
//        glOrtho(-66.1,66.1,-66.1,66.1, -z1, -z1-z2); // mm
//    } else {
//        glOrtho(-66.1,66.1,-66.1,66.1,-66.1,66.1); // mm
//    }

    gluPerspective(60.0, ((double)w())/((double)h()), 1, 400);
    glTranslated(0.0, 0.0, pZOffset);
    glRotated(-90.0, 1.0, 0.0, 0.0);
    glTranslated(-pXOffset, 0.0, pYOffset);
    glRotated(-pYRotation, 1.0, 0.0, 0.0);
    glRotated(-pXRotation, 0.0, 0.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    glScaled(10, 10, 10);
    int i;
    for (i=-10; i<=10; i++) {
        double p = (double)i;
        glColor3d(0.6, 0.6, 0.6);
        glBegin(GL_LINES);
        glVertex3d(p, -10.0, 0.0); glVertex3d(p, 10.0, 0.0);
        glVertex3d(-10.0, p, 0.0); glVertex3d(10.0, p, 0.0);
        glEnd();
    }
    glBegin(GL_LINES);
    glColor3d(0.8, 0.8, 0.8);
    glVertex2d(0.0, -10.0); glVertex2d(0.0, 10.0);
    glVertex2d(-10.0, 0.0); glVertex2d(10.0, 0.0);
    glColor3d(1.0, 0.4, 0.4);
    glVertex2d(0.0, 0.0); glVertex2d(10.0, 0.0);
    glColor3d(0.4, 1.0, 0.4);
    glVertex2d(0.0, 0.0); glVertex2d(0.0, 10.0);
    glEnd();
    glPopMatrix();

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glPushMatrix();

    // BG gradient from QuickLook:     CGFloat components[8] = {0.9, 0.9, 0.9, 1.0,  0.6, 0.6, 0.9, 1.0 };

//    if (gShowSlice) {
//        // show just the slice
//        glDisable(GL_LIGHTING);
//        glDisable(GL_DEPTH_TEST);
//        // draw the model using the z buffer for clipping
//        drawModelFlat(FL_RED);
//        glMatrixMode (GL_PROJECTION);
//        // change the z range to disable clipping
//        glLoadIdentity();
//        glOrtho(-66.1,66.1,-66.1,66.1,-66.1,66.1); // mm
//        glMatrixMode(GL_MODELVIEW);
//        gMeshSlice.drawFlat(FL_GREEN);
//        gMeshSlice.drawLidEdge();
//        // set the z range again to enable drawing the shell
//        glMatrixMode (GL_PROJECTION);
//        glLoadIdentity();
//        glOrtho(-66.1,66.1,-66.1,66.1, -z1, -z1-z2); // mm
//        glMatrixMode(GL_MODELVIEW);
//        // the following code guarantees a hull of at least 1mm width
//        //      double sd;
//        //      glHint(GL_POLYGON_SMOOTH_HINT, );
//        //      glDisable (GL_POLYGON_SMOOTH);
//        //      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//        //      for (sd = 0.2; sd<gMinimumShell; sd+=0.2) {
//        //        drawModelShrunk(Fl_WHITE, sd);
//        //      }
//        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//    } else {
        // show the 3d model
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        if (pDrawCallback) pDrawCallback();
//        drawModelGouraud();
//    }
    glPopMatrix();
//
//    if (gWriteSliceNext==1) {
//        gWriteSliceNext = 0;
//        writeSlice();
//    } else if (gWriteSliceNext==2) {
//        gWriteSliceNext = 0;
//        writePrnSlice();
//    }

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w(), 0, h(), -10, 10); // mm
    glMatrixMode(GL_MODELVIEW);
    gl_color(FL_WHITE);
//    char buf[1024];
//    sprintf(buf, "Slice at %.4gmm", z1); gl_draw(buf, 10, 40);
//    sprintf(buf, "%.4gmm thick", z2); gl_draw(buf, 10, 20);
}



