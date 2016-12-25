

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <QuickLook/QuickLook.h>

#include <Foundation/NSDictionary.h>
#include <AppKit/AppKit.h>

#include <stdlib.h>
#include <stdio.h>

#include "ETModelSTL.h"
#include "ETTriangle.h"

#include "ETQuickLookHelper.h"

#define QLDEBUG(a) fprintf(stderr, a);

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
#if 1
    // This is the main branch. Set the above definition to 0 to test the 3D implementation

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
                        
                        model->Prepare2DDrawing();
                        model->CGDraw2D(cgContext, canvasSize.width, canvasSize.height);
                        ET::DrawText(cgContext, 18, 10.0f, 10.0f, "www.elektriktrick.com  QL:v1.1.0");
                        
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

#else
    // Testing QuickLook using the macOS 10.7 dae extension

    @autoreleasepool {

        // All this does is generate a cube.
        NSMutableString *dae=[[NSMutableString alloc] init];
        [dae appendString:@"<?xml version=\"1.0\"?>\n"];
        [dae appendString:@"<COLLADA>\n"];
        [dae appendString:@"<asset>\n"];
        [dae appendString:@"<up_axis>Z_UP</up_axis>\n"];
        [dae appendString:@"</asset>\n"];
        [dae appendString:@"<library_materials>\n"];
        [dae appendString:@"<material id=\"Color\">\n"];
        [dae appendString:@"<instance_effect url=\"#phongEffect\">\n"];
        [dae appendString:@"<emission>\n"];
        [dae appendString:@"<color>0.502 0.502 0.0 1.0</color>\n"];
        [dae appendString:@"</emission>\n"];
        [dae appendString:@"</instance_effect>\n"];
        [dae appendString:@"</material>\n"];
        [dae appendString:@"</library_materials>\n"];
        [dae appendString:@"<library_geometries>\n"];
        [dae appendString:@"<geometry id=\"geom-Box01\" name=\"Box01\">\n"];
        [dae appendString:@"<mesh>\n"];
        [dae appendString:@"<source id=\"positions\">\n"];
        [dae appendString:@"<float_array id=\"positions-array\" count=\"24\">\n"];
        [dae appendString:@"0 0 0\n"];
        [dae appendString:@"0 0 1\n"];
        [dae appendString:@"0 1 0\n"];
        [dae appendString:@"1 0 0\n"];
        [dae appendString:@"0 1 1\n"];
        [dae appendString:@"1 0 1\n"];
        [dae appendString:@"1 1 0\n"];
        [dae appendString:@"1 1 1\n"];
        [dae appendString:@"</float_array>\n"];
        [dae appendString:@"<technique_common>\n"];
        [dae appendString:@"<accessor source=\"#positions-array\" count=\"8\" stride=\"3\">\n"];
        [dae appendString:@"<param name=\"X\" type=\"float\"/>\n"];
        [dae appendString:@"<param name=\"Y\" type=\"float\"/>\n"];
        [dae appendString:@"<param name=\"Z\" type=\"float\"/>\n"];
        [dae appendString:@"</accessor>\n"];
        [dae appendString:@"</technique_common>\n"];
        [dae appendString:@"</source>\n"];
        [dae appendString:@"<source id=\"normals\">\n"];
        [dae appendString:@"<float_array id=\"normals-array\" count=\"18\">\n"];
        [dae appendString:@"1 0 0\n"];
        [dae appendString:@"-1 0 0\n"];
        [dae appendString:@"0 1 0\n"];
        [dae appendString:@"0 -1 0\n"];
        [dae appendString:@"0 0 1\n"];
        [dae appendString:@"0 0 -1\n"];
        [dae appendString:@"</float_array>\n"];
        [dae appendString:@"<technique_common>\n"];
        [dae appendString:@"<accessor source=\"#normals-array\" count=\"8\" stride=\"3\">\n"];
        [dae appendString:@"<param name=\"X\" type=\"float\"/>\n"];
        [dae appendString:@"<param name=\"Y\" type=\"float\"/>\n"];
        [dae appendString:@"<param name=\"Z\" type=\"float\"/>\n"];
        [dae appendString:@"</accessor>\n"];
        [dae appendString:@"</technique_common>\n"];
        [dae appendString:@"</source>\n"];
        [dae appendString:@"<vertices id=\"vertices\">\n"];
        [dae appendString:@"<input semantic=\"POSITION\" source=\"#positions\"/>\n"];
        [dae appendString:@"</vertices>\n"];
        [dae appendString:@"<triangles name=\"sample_triangles\" count=\"24\" color=\"Color\">\n"];
        [dae appendString:@"<input semantic=\"VERTEX\" source=\"#vertices\" offset=\"0\"/>\n"];
        [dae appendString:@"<input semantic=\"NORMAL\" source=\"#normals\" offset=\"1\"/>\n"];
        [dae appendString:@"<p>\n"];
        [dae appendString:@"0 5 3 5 2 5\n"];
        [dae appendString:@"2 5 3 5 0 5\n"];
        [dae appendString:@"2 5 3 5 6 5\n"];
        [dae appendString:@"6 5 3 5 2 5\n"];
        [dae appendString:@"5 0 3 0 7 0\n"];
        [dae appendString:@"7 0 3 0 5 0\n"];
        [dae appendString:@"7 0 3 0 6 0\n"];
        [dae appendString:@"6 0 3 0 7 0\n"];
        [dae appendString:@"1 4 5 4 7 4\n"];
        [dae appendString:@"7 4 5 4 1 4\n"];
        [dae appendString:@"7 4 1 4 4 4\n"];
        [dae appendString:@"4 4 1 4 7 4\n"];
        [dae appendString:@"0 1 1 1 4 1\n"];
        [dae appendString:@"4 1 1 1 0 1\n"];
        [dae appendString:@"4 1 0 1 2 1\n"];
        [dae appendString:@"2 1 0 1 4 1\n"];
        [dae appendString:@"1 3 5 3 3 3\n"];
        [dae appendString:@"3 3 5 3 1 3\n"];
        [dae appendString:@"3 3 1 3 0 3\n"];
        [dae appendString:@"0 3 1 3 3 3\n"];
        [dae appendString:@"4 2 7 2 6 2\n"];
        [dae appendString:@"6 2 7 2 4 2\n"];
        [dae appendString:@"6 2 4 2 2 2\n"];
        [dae appendString:@"2 2 4 2 6 2\n"];
        [dae appendString:@"</p>\n"];
        [dae appendString:@"</triangles>\n"];
        [dae appendString:@"</mesh>\n"];
        [dae appendString:@"</geometry>\n"];
        [dae appendString:@"</library_geometries>\n"];
        [dae appendString:@"<library_visual_scenes>\n"];
        [dae appendString:@"<visual_scene id=\"MaxScene\">\n"];
        [dae appendString:@"<node id=\"node-Box01\" name=\"Box01\">\n"];
        [dae appendString:@"<instance_geometry url=\"#geom-Box01\">\n"];
        [dae appendString:@"</instance_geometry>\n"];
        [dae appendString:@"</node>\n"];
        [dae appendString:@"</visual_scene>\n"];
        [dae appendString:@"</library_visual_scenes>\n"];
        [dae appendString:@"<scene>\n"];
        [dae appendString:@"<instance_visual_scene url=\"#MaxScene\"/>\n"];
        [dae appendString:@"</scene>\n"];
        [dae appendString:@"</COLLADA>\n"];

        QLPreviewRequestSetDataRepresentation(
                                              preview,
                                              (__bridge CFDataRef)[dae dataUsingEncoding:NSUTF8StringEncoding],
                                              CFSTR("org.khronos.collada.digital-asset-exchange"),
                                              NULL
                                              );
    }

    return noErr;

#endif
}


void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview)
{
    // Implement only if supported
}

