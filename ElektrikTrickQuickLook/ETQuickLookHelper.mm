//
//  QuickLookHelper.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/20/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETQuickLookHelper.h"
#import <Cocoa/Cocoa.h>

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
    CTFontRef font = CTFontCreateWithName( CFSTR("ArialMT"), height, NULL);
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


void ET::ReadPrefs()
{
    // Create an instance of NSUserDefaults
    NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
    NSString *author2 = [prefs stringForKey:@"Author"];
    
    // Store and retrieve a string
    [prefs setObject:@"Mike Beam 3rd" forKey:@"Author"];
    NSString *author = [prefs stringForKey:@"Author"];
    
    // Store and retrieve a number
    [prefs setFloat:1373.50 forKey:@"NASDAQ"];
    [prefs setInteger:2002 forKey:@"Year"];
    float level = [prefs floatForKey:@"NASDAQ"];
    int year = [prefs integerForKey:@"Year"];
    [prefs synchronize];

/*
 The following general guiding principles apply to the CFPreferences API:
 
 You should typically use CFPreferencesCopyAppValue to read preference keys.
 You should use CFPreferencesSetAppValue to write preference keys for “current user/any host.”
 If you need to write a by-host preference for the current user, use CFPreferencesSetValue. Make sure this is absolutely necessary.
*/
    
    CFStringRef appID = CFSTR("com.apple.anotherapp");
    CFStringRef defaultTextColorKey = CFSTR("defaultTextColor");
    CFStringRef colorBLUE = CFSTR("BLUE");
    
    // Set up the preference.
    CFPreferencesSetValue(defaultTextColorKey,
                          colorBLUE,
                          appID,
                          kCFPreferencesCurrentUser,
                          kCFPreferencesAnyHost);
    
    // Write out the preference data.
    CFPreferencesSynchronize(appID,
                             kCFPreferencesCurrentUser,
                             kCFPreferencesAnyHost);
}


