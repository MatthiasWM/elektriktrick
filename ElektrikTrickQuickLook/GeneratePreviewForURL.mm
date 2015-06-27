

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include <stdlib.h>

#include "ETModelSTL.h"
#include "ETTriangle.h"

#include "ETQuickLookHelper.h"

extern "C" {
    OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options);
    void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview);
}



/* -----------------------------------------------------------------------------
 Generate a preview for file
 
 This function's job is to create preview for designated file
 ----------------------------------------------------------------------------- */

// http://www.germinara.it/download/FGOUTLOOK2011Manual.pdf


OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options)
{
    OSStatus ret = noErr;
    
    @autoreleasepool {
        
        char filename[2048];
        if (CFURLGetFileSystemRepresentation(url, true, (UInt8*)filename, 2047)) {

            ETModel *model = ETModel::ModelForFileType(filename);
            if (model) {

                if (model->Load()) {
                    
                    // FIXME: read this data from the XML file somehow
                    ET::ReadPrefs();
                    CGSize canvasSize; canvasSize.width = 1000; canvasSize.height = 800;
                    
                    // Preview will be drawn in a vectorized context
                    CGContextRef cgContext = QLPreviewRequestCreateContext(preview, canvasSize, true, NULL);
                    if (cgContext) {
                        CGContextSaveGState(cgContext);
                        
                        ET::DrawBackgroupdGradient(cgContext, canvasSize.width, canvasSize.height);
                        
                        model->PrepareDrawing();
                        model->Draw(cgContext, canvasSize.width, canvasSize.height);
                        ET::DrawText(cgContext, 18, 10.0f, 10.0f, "www.elektriktrick.com  QL:v1.0.2");
                        
                        CGContextRestoreGState(cgContext);
                        QLPreviewRequestFlushContext(preview, cgContext);
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


void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview)
{
    // Implement only if supported
}

