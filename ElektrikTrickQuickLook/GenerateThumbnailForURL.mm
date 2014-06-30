#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include <stdlib.h>

#include "ETModelCG.h"
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
//  return noErr;
  @autoreleasepool{
    
    // FIXME: read the document here and abort if we get an error
    ETModelCG model;
    
    // FIXME: read this data from the XML file somehow
    CGSize canvasSize; canvasSize.width = 512; canvasSize.height = 512;
    
    // Thumbnail will be drawn in context
    CGContextRef cgContext = QLThumbnailRequestCreateContext(thumbnail, canvasSize, true, NULL);
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
      
      CGContextRestoreGState(cgContext);
      QLThumbnailRequestFlushContext(thumbnail, cgContext);
      CFRelease(cgContext);
    }
  }
  return noErr;
}

void CancelThumbnailGeneration(void *thisInterface, QLThumbnailRequestRef thumbnail)
{
    // Implement only if supported
}
