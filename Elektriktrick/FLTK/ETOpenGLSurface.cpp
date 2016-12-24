//
//  ETOpenGLSurface.cpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 10/24/15.
//  Copyright © 2015 M.Melcher GmbH. All rights reserved.
//

#include "ETOpenGLSurface.h"

#include <stdio.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreGraphics/CGImage.h>
#include <CoreGraphics/CGContext.h>
#include <CoreServices/CoreServices.h>

#include <FL/Fl_JPEG_Image.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
extern "C" {
#include "/Users/matt/dev/fltk-1.3.4/jpeg/jpeglib.h"
}


ETOpenGLSurface::ETOpenGLSurface(int w, int h)
{

    //RGBA8 RenderBuffer, 24 bit depth RenderBuffer, 256x256
    fb = 0;
    glGenFramebuffers(1, &fb);
//    printf("%d %d\n", glGetError(), fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
//    printf("%d\n", glGetError());
    //Create and attach a color buffer
    glGenRenderbuffers(1, &color_rb);
//    printf("%d\n", glGetError());
    //We must bind color_rb before we call glRenderbufferStorage
    glBindRenderbuffer(GL_RENDERBUFFER, color_rb);
//    printf("%d\n", glGetError());
    //The storage format is RGBA8
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
//    printf("%d\n", glGetError());
//    glRenderbufferStorageMultisample(...)
    //Attach color buffer to FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_rb);
//    printf("%d\n", glGetError());
    //-------------------------
    glGenRenderbuffers(1, &depth_rb);
//    printf("%d\n", glGetError());
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
//    printf("%d\n", glGetError());
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
//    printf("%d\n", glGetError());
    //-------------------------
    //Attach depth buffer to FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
//    printf("%d\n", glGetError());
    //-------------------------
    //Does the GPU support current FBO configuration?
    GLenum status;
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//    printf("%d\n", glGetError());
    switch(status)
    {
        case GL_FRAMEBUFFER_COMPLETE:
            /* printf("Framebuffer is OK\n"); */ break;
        case  GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            printf(")GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            printf(")GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n"); break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            printf(")GL_FRAMEBUFFER_UNSUPPORTED\n");
        default:
            printf("ERROR creating Framebuffer (%d)\n", status);
    }
    //-------------------------
    //and now you can render to the FBO (also called RenderBuffer)
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
//    printf("%d\n", glGetError());
    glClearColor(0.0, 1.0, 0.0, 0.0);
//    printf("%d\n", glGetError());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    printf("%d\n", glGetError());
    //-------------------------
    glViewport(0, 0, 256, 256);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
//    printf("%d\n", glGetError());
    //-------------------------
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
//    printf("%d\n", glGetError());
    //-------------------------
    //**************************
    //RenderATriangle, {0.0, 0.0}, {256.0, 0.0}, {256.0, 256.0}
    //Read http://www.opengl.org/wiki/VBO_-_just_examples

    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_TRIANGLES);
    glVertex3d(10, 10, 0);
    glVertex3d(300, 30, 0);
    glVertex3d(30, 200, 0);
    glEnd();

    //-------------------------
    GLubyte pixels[w*h*4];
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
//    printf("%d\n", glGetError());
    //pixels 0, 1, 2 should be white
    //pixel 4 should be black
    //----------------
    //Bind 0, which means render to back buffer
    {
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        cinfo.err = jpeg_std_error((jpeg_error_mgr *)&jerr);
        jpeg_create_compress(&cinfo);
        FILE* outfile;
        outfile = fopen("/Users/matt/test.jpg", "wb");
        if (!outfile) return;
        jpeg_stdio_dest(&cinfo, outfile);
        cinfo.image_width = w;
        cinfo.image_height = h;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB; /* colorspace of input image */
        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, 90, TRUE);
        jpeg_start_compress(&cinfo, TRUE);
        JSAMPROW row_pointer[1];        /* pointer to a single row */
        int row_stride = w*3;
        unsigned char *dd = pixels;
        while (cinfo.next_scanline < cinfo.image_height) {
            row_pointer[0] = dd+(cinfo.next_scanline * row_stride);
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }
        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
        fclose(outfile);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ETOpenGLSurface::~ETOpenGLSurface()
{
    //Delete resources
    glDeleteRenderbuffers(1, &color_rb);
    glDeleteRenderbuffers(1, &depth_rb);
    //Bind 0, which means render to back buffer, as a result, fb is unbound
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fb);
}

