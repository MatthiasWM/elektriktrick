//
//  ETOpenGLSurface.hpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 10/24/15.
//  Copyright © 2015 M.Melcher GmbH. All rights reserved.
//

#ifndef ETOpenGLSurface_hpp
#define ETOpenGLSurface_hpp

#include <Fl/gl.h>


class ETOpenGLSurface
{
public:
    ETOpenGLSurface(int w, int h);
    ~ETOpenGLSurface();
    void draw();
    void save(const char *filename);
private:
    GLuint fb, color_rb, depth_rb;
};

#endif /* ETOpenGLSurface_hpp */
