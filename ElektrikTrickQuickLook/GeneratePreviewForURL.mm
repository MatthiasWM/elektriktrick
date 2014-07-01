

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include <stdlib.h>

#include "ETModelCG.h"
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
  @autoreleasepool{
    
    // FIXME: read the document here and abort if we get an error
    ETModelCG model;
    
    // FIXME: read this data from the XML file somehow
    CGSize canvasSize; canvasSize.width = 1000; canvasSize.height = 800;
    
    // Preview will be drawn in a vectorized context
    CGContextRef cgContext = QLPreviewRequestCreateContext(preview, canvasSize, true, NULL);
    if (cgContext) {
      CGContextSaveGState(cgContext);
      
      ET::DrawBackgroupdGradient(cgContext, canvasSize.width, canvasSize.height);
      
      char filename[2048];
      if (CFURLGetFileSystemRepresentation(url, true, (UInt8*)filename, 2047)) {
        if (model.Load(filename)) {
          model.PrepareDrawing();
          model.Draw(cgContext, canvasSize.width, canvasSize.height);          
        }
      }
      // TODO: do we want to draw an error string onto the preview?
      ET::DrawText(cgContext, 18, 10.0f, 10.0f, "www.elektriktrick.com  QL:v1.0.1");
      
      CGContextRestoreGState(cgContext);
      QLPreviewRequestFlushContext(preview, cgContext);
      CFRelease(cgContext);
    }
  }
  
  return noErr;
}


void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview)
{
  // Implement only if supported
}

