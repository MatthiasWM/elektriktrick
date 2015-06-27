//
//  QuickLookHelper.h
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#ifndef __Electrictrick__QuickLookHelper__
#define __Electrictrick__QuickLookHelper__

#include <QuickLook/QuickLook.h>


namespace ET
{
    void DrawBackgroupdGradient(CGContextRef, int, int);
    void DrawText(CGContextRef, int height, float x, float y, const char* text);
    void ReadPrefs();
};


#endif /* defined(__Electrictrick__QuickLookHelper__) */
