#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include <stdlib.h>

#include "ETModelSTL.h"
#include "ETTriangle.h"

#include "ETQuickLookHelper.h"

extern "C" {
    OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize);
    void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail);
}

/* -----------------------------------------------------------------------------
 Generate a thumbnail for file
 
 This function's job is to create thumbnail for designated file as fast as possible
 ----------------------------------------------------------------------------- */

OSStatus GenerateThumbnailForURL(void *thisInterface, QLThumbnailRequestRef thumbnail, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options, CGSize maxSize)
{
    OSStatus ret = noErr;
    
    @autoreleasepool {
        
        char filename[2048];
        if (CFURLGetFileSystemRepresentation(url, true, (UInt8*)filename, 2047)) {
            
            ETModel *model = ETModel::ModelForFileType(filename);
            if (model) {
                
                if (model->Load()) {
                    
                    // FIXME: read this data from the XML file somehow
                    CGSize canvasSize; canvasSize.width = 1000; canvasSize.height = 800;
                    
                    // Preview will be drawn in a vectorized context
                    CGContextRef cgContext = QLThumbnailRequestCreateContext(thumbnail, canvasSize, true, NULL);
                    if (cgContext) {
                        CGContextSaveGState(cgContext);
                        
                        ET::DrawBackgroupdGradient(cgContext, canvasSize.width, canvasSize.height);
                        
                        model->Prepare2DDrawing();
                        model->CGDraw2D(cgContext, canvasSize.width, canvasSize.height);
                        
                        CGContextRestoreGState(cgContext);
                        QLThumbnailRequestFlushContext(thumbnail, cgContext);
                        CFRelease(cgContext);
                    }
                } else {
                    ret = coreFoundationUnknownErr;
                }
            } else {
                ret = coreFoundationUnknownErr;
            }
            delete model;
        } else {
            ret = coreFoundationUnknownErr;
        }
    }
    return ret;
}

void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail)
{
    // Implement only if supported
}
