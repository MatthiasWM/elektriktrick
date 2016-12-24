

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

#if 1

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options)
{
#if 1
    OSStatus ret = noErr;
    QLDEBUG("*********\n*** GeneratePreviewForURL\n\n")

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
                        ET::DrawText(cgContext, 18, 10.0f, 10.0f, "www.elektriktrick.com  QL:v1.0.3");
                        
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
    QLDEBUG("\n*** done\n*********\n")
    return ret;

#else

    @autoreleasepool {
        QLDEBUG("*********\n*** TESTING 2!\n\n")


#if 1
        NSMutableString *html=[[NSMutableString alloc] init];
/*
        [html appendString:@"<html>"];
        [html appendString:@"<body>"];
        [html appendString:@"<h1>First Last</h1>"];
        [html appendString:@"</body>"];
        [html appendString:@"</html>"];
*/
        [html appendString:@"<?xml version=\"1.0\"?>\n"];
        [html appendString:@"<COLLADA>\n"];
        [html appendString:@"<asset>\n"];
        [html appendString:@"<up_axis>Z_UP</up_axis>\n"];
        [html appendString:@"</asset>\n"];
        [html appendString:@"<library_materials>\n"];
        [html appendString:@"<material id=\"Color\">\n"];
        [html appendString:@"<instance_effect url=\"#phongEffect\">\n"];
        [html appendString:@"<emission>\n"];
        [html appendString:@"<color>0.502 0.502 0.0 1.0</color>\n"];
        [html appendString:@"</emission>\n"];
        [html appendString:@"</instance_effect>\n"];
        [html appendString:@"</material>\n"];
        [html appendString:@"</library_materials>\n"];
        [html appendString:@"<library_geometries>\n"];
        [html appendString:@"<geometry id=\"geom-Box01\" name=\"Box01\">\n"];
        [html appendString:@"<mesh>\n"];
        [html appendString:@"<source id=\"positions\">\n"];
        [html appendString:@"<float_array id=\"positions-array\" count=\"24\">\n"];
        [html appendString:@"0 0 0\n"];
        [html appendString:@"0 0 1\n"];
        [html appendString:@"0 1 0\n"];
        [html appendString:@"1 0 0\n"];
        [html appendString:@"0 1 1\n"];
        [html appendString:@"1 0 1\n"];
        [html appendString:@"1 1 0\n"];
        [html appendString:@"1 1 1\n"];
        [html appendString:@"</float_array>\n"];
        [html appendString:@"<technique_common>\n"];
        [html appendString:@"<accessor source=\"#positions-array\" count=\"8\" stride=\"3\">\n"];
        [html appendString:@"<param name=\"X\" type=\"float\"/>\n"];
        [html appendString:@"<param name=\"Y\" type=\"float\"/>\n"];
        [html appendString:@"<param name=\"Z\" type=\"float\"/>\n"];
        [html appendString:@"</accessor>\n"];
        [html appendString:@"</technique_common>\n"];
        [html appendString:@"</source>\n"];
        [html appendString:@"<source id=\"normals\">\n"];
        [html appendString:@"<float_array id=\"normals-array\" count=\"18\">\n"];
        [html appendString:@"1 0 0\n"];
        [html appendString:@"-1 0 0\n"];
        [html appendString:@"0 1 0\n"];
        [html appendString:@"0 -1 0\n"];
        [html appendString:@"0 0 1\n"];
        [html appendString:@"0 0 -1\n"];
        [html appendString:@"</float_array>\n"];
        [html appendString:@"<technique_common>\n"];
        [html appendString:@"<accessor source=\"#normals-array\" count=\"8\" stride=\"3\">\n"];
        [html appendString:@"<param name=\"X\" type=\"float\"/>\n"];
        [html appendString:@"<param name=\"Y\" type=\"float\"/>\n"];
        [html appendString:@"<param name=\"Z\" type=\"float\"/>\n"];
        [html appendString:@"</accessor>\n"];
        [html appendString:@"</technique_common>\n"];
        [html appendString:@"</source>\n"];
        [html appendString:@"<vertices id=\"vertices\">\n"];
        [html appendString:@"<input semantic=\"POSITION\" source=\"#positions\"/>\n"];
        [html appendString:@"</vertices>\n"];
        [html appendString:@"<triangles name=\"sample_triangles\" count=\"24\" color=\"Color\">\n"];
        [html appendString:@"<input semantic=\"VERTEX\" source=\"#vertices\" offset=\"0\"/>\n"];
        [html appendString:@"<input semantic=\"NORMAL\" source=\"#normals\" offset=\"1\"/>\n"];
        [html appendString:@"<p>\n"];
        [html appendString:@"0 5 3 5 2 5\n"];
        [html appendString:@"2 5 3 5 0 5\n"];
        [html appendString:@"2 5 3 5 6 5\n"];
        [html appendString:@"6 5 3 5 2 5\n"];
        [html appendString:@"5 0 3 0 7 0\n"];
        [html appendString:@"7 0 3 0 5 0\n"];
        [html appendString:@"7 0 3 0 6 0\n"];
        [html appendString:@"6 0 3 0 7 0\n"];
        [html appendString:@"1 4 5 4 7 4\n"];
        [html appendString:@"7 4 5 4 1 4\n"];
        [html appendString:@"7 4 1 4 4 4\n"];
        [html appendString:@"4 4 1 4 7 4\n"];
        [html appendString:@"0 1 1 1 4 1\n"];
        [html appendString:@"4 1 1 1 0 1\n"];
        [html appendString:@"4 1 0 1 2 1\n"];
        [html appendString:@"2 1 0 1 4 1\n"];
        [html appendString:@"1 3 5 3 3 3\n"];
        [html appendString:@"3 3 5 3 1 3\n"];
        [html appendString:@"3 3 1 3 0 3\n"];
        [html appendString:@"0 3 1 3 3 3\n"];
        [html appendString:@"4 2 7 2 6 2\n"];
        [html appendString:@"6 2 7 2 4 2\n"];
        [html appendString:@"6 2 4 2 2 2\n"];
        [html appendString:@"2 2 4 2 6 2\n"];
        [html appendString:@"</p>\n"];
        [html appendString:@"</triangles>\n"];
        [html appendString:@"</mesh>\n"];
        [html appendString:@"</geometry>\n"];
        [html appendString:@"</library_geometries>\n"];
        [html appendString:@"<library_visual_scenes>\n"];
        [html appendString:@"<visual_scene id=\"MaxScene\">\n"];
        [html appendString:@"<node id=\"node-Box01\" name=\"Box01\">\n"];
        [html appendString:@"<instance_geometry url=\"#geom-Box01\">\n"];
        [html appendString:@"</instance_geometry>\n"];
        [html appendString:@"</node>\n"];
        [html appendString:@"</visual_scene>\n"];
        [html appendString:@"</library_visual_scenes>\n"];
        [html appendString:@"<scene>\n"];
        [html appendString:@"<instance_visual_scene url=\"#MaxScene\"/>\n"];
        [html appendString:@"</scene>\n"];
        [html appendString:@"</COLLADA>\n"];

        QLPreviewRequestSetDataRepresentation(
                                              preview,
                                              (__bridge CFDataRef)[html dataUsingEncoding:NSUTF8StringEncoding],
                                              CFSTR("org.khronos.collada.digital-asset-exchange"),
                                              NULL
                                              );
#else

        /*
         QLPreviewRequestSetDataRepresentation(preview, (__bridge CFDataRef)rtfData,
         CFSTR("org.khronos.collada.digital-asset-exchange"),
         NULL);
         and return the 3D data in Collada XML format
         */
#endif

    }
    QLDEBUG("\n*** done\n*********\n")
    return noErr;

#endif
}

#else

OSStatus GeneratePreviewForURL(void *thisInterface, QLPreviewRequestRef preview, CFURLRef url, CFStringRef contentTypeUTI, CFDictionaryRef options)
{
    QLDEBUG("*********\n*** TESTING 4!\n\n")
    @autoreleasepool {

        // Load the strings dictionary from the URL
        NSDictionary *localizationDict = [NSDictionary dictionaryWithContentsOfURL:(__bridge NSURL *)url];
        QLDEBUG("*** 0\n")
        if (!localizationDict) return noErr;
        QLDEBUG("*** 1\n")

        // The above might have taken some time, so before proceeding make sure the user didn't cancel the request
        if (QLPreviewRequestIsCancelled(preview)) return noErr;
        QLDEBUG("*** 2\n")

        // Set up for producing attributed string output
        NSDictionary *keyAttrs = @{ NSFontAttributeName : [NSFont fontWithName:@"Helvetica-Oblique" size:11.0],
                                    NSForegroundColorAttributeName : [NSColor grayColor] };
        NSDictionary *valAttrs = @{ NSFontAttributeName : [NSFont fontWithName:@"Helvetica-Bold" size:18.0] };
        NSAttributedString *newline = [[NSAttributedString alloc] initWithString:@"\n"];
        NSMutableAttributedString *output = [[NSMutableAttributedString alloc] init];
        QLDEBUG("*** 3\n")

        // Iterate through pairs in the dictionary to add formatted output
        [localizationDict enumerateKeysAndObjectsUsingBlock:^(id key, id val, BOOL *stop) {
            NSAttributedString *keyString = [[NSAttributedString alloc] initWithString:key attributes:keyAttrs];
            NSAttributedString *valString = [[NSAttributedString alloc] initWithString:val attributes:valAttrs];
            [output appendAttributedString:valString];
            [output appendAttributedString:newline];
            [output appendAttributedString:keyString];
            [output appendAttributedString:newline];
            [output appendAttributedString:newline];
        }];
        QLDEBUG("*** 4\n")

        // Get RTF representation of the attributed string
        NSData *rtfData = [output RTFFromRange:NSMakeRange(0, output.length) documentAttributes:nil];

        QLDEBUG("*** 5\n")

        // Pass preview data to QuickLook
        QLPreviewRequestSetDataRepresentation(preview,
                                              (__bridge CFDataRef)rtfData,
                                              kUTTypeRTF,
                                              NULL);
    }
    QLDEBUG("\n*** done\n*********\n")
    return noErr;
}

#endif



void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview)
{
    // Implement only if supported
}

