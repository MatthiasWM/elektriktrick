//
//  QuickLookHelper.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETQuickLookHelper.h"


void ET::DrawBackgroupdGradient(CGContextRef cgContext, int width, int height)
{
    // generate background
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    CGContextClip(cgContext);
    CGPoint startPoint = CGPointMake(width/2, 0);
    CGPoint endPoint = CGPointMake(width/2, height);
    CGFloat locations[2] = {0, 0.95};
    // unhighlighted outline color
    CGFloat components[8] = {0.9, 0.9, 0.9, 1.0,  0.6, 0.6, 0.9, 1.0 };
    CGGradientRef gradient = CGGradientCreateWithColorComponents(colorspace, components, locations, 2);
    CGContextDrawLinearGradient(cgContext, gradient, startPoint, endPoint, 0);
    // CGContextDrawRadialGradient(cgContext, <#CGGradientRef gradient#>, <#CGPoint startCenter#>, <#CGFloat startRadius#>, <#CGPoint endCenter#>, <#CGFloat endRadius#>, <#CGGradientDrawingOptions options#>)
    CGGradientRelease(gradient);
}


void ET::DrawText(CGContextRef cgContext, int height, float x, float y, const char* text)
{
    CGContextSetRGBFillColor(cgContext, 1.0, 0, 0, 0.8);
    CFStringRef string = CFStringCreateWithCString(kCFAllocatorDefault, text, kCFStringEncodingUTF8);
    CTFontRef font = CTFontCreateWithName( CFSTR("Arial"), height, NULL);
    CGContextRef context = cgContext;
    // Initialize the string, font, and context
    
    CGColorSpaceRef rgbColorSpace = CGColorSpaceCreateDeviceRGB();
    CGFloat components[] = { 0.2, 0.2, 0.2, 0.8 };
    CGColorRef textColor = CGColorCreate(rgbColorSpace, components);
    CGColorSpaceRelease(rgbColorSpace);
    
    CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
    CFTypeRef values[] = { font, textColor };
    
    CFDictionaryRef attributes =
    CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys,
                       (const void**)&values, sizeof(keys) / sizeof(keys[0]),
                       &kCFTypeDictionaryKeyCallBacks,
                       &kCFTypeDictionaryValueCallBacks);
    
    CFAttributedStringRef attrString =
    CFAttributedStringCreate(kCFAllocatorDefault, string, attributes);
    
    CFRelease(string);
    CFRelease(attributes);
    
    CTLineRef line = CTLineCreateWithAttributedString(attrString);
    
    CGContextSetTextPosition(context, x, y);
    CTLineDraw(line, context);
    CFRelease(line);
}


