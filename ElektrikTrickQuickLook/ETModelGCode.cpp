//
//  ETModelGCode.cpp
//  Electrictrick
//
//  Created by Matthias Melcher on 6/26/14.
//  Copyright (c) 2014 M.Melcher GmbH. All rights reserved.
//

#include "ETModelGCode.h"
#include "ETEdge.h"
#include "ETVector.h"


ETModel *ETModelGCode::Create(uint8_t *buf, size_t size)
{
    // gcode lines evetually start with "M#" or "G#", where # is some decimal integer
    signed int i;
    for (i=0; i<size-4; i++) {
        if (i==0 || buf[i-1]=='\r' || buf[i-1]=='\n') {
            // find a GCode command
            if (buf[i]=='G' || buf[i]=='M') {
                if (isdigit(buf[i+1])) return new ETModelGCode();
            }
            // alternatively, find a typical GCode comment (weak!)
            if (buf[i]==';' && buf[i+1]==' ' && isalnum(buf[i+2])) {
                return new ETModelGCode();
            }
        }
    }
    return 0L;
}


ETModelGCode::ETModelGCode()
:   ETWireframeModel()
{
}


ETModelGCode::~ETModelGCode()
{
}

void test();

/**
 * GCodeloader.
 *
 * GCode is an ancient file format to control CNC machines. In the 3D printing
 * world, it is used as the lowest common denominator to send motion data to 
 * a three-axis printer. The format was extended with many control codes
 * for multiple extruders, heated nozzles and beds, and whatever else comes 
 * to the developers minds. 
 *
 * FOr QuickLook, we limit ourselves to G0 and G1 which move the extruder 
 * around, for example:
 *
 *   G1 X-0.55 Y20.0 Z0.2 F840.0
 *
 * \return no error code, it fails silently, but creates an intact (possibly incomplete) dataset
 *
 * \FIXME: Adam.gcode contains G-Commands separated by a semicolon. 
 *         It also contains NUL characters and crashes the QL
 *         It contains lines with the G command missing, followed by X, Y, and Z commands
 */
int ETModelGCode::Load()
{
//    test();
    
    
    bool absPos = true;
    bool ignoreFirst = true;
    double cx = 0.0, cy = 0.0, cz = 0.0;
    NEdge = 8192;
    edge = (ETEdge*)calloc(NEdge, sizeof(ETEdge));
    
    char buf[1024];
    
    for (;;) {
        if (FEof()) break;
        buf[0] = 0;
        FGetS(buf, 1023);
        if (buf[0]=='G') {
            int gCode = atoi(buf+1);
            if (gCode==0 || gCode==1) { // move
                if (NEdge==nEdge) {
                    NEdge += 8192;
                    edge = (ETEdge*)realloc(edge, NEdge*sizeof(ETEdge));
                }
                ETEdge &e = edge[nEdge];
                e.p0.set(cx, cy, cz);
                char *arg = strchr(buf+2, 'X');
                if (arg) cx = (absPos?0:cx) + atof(arg+1);
                arg = strchr(buf+2, 'Y');
                if (arg) cy = (absPos?0:cx) + atof(arg+1);
                arg = strchr(buf+2, 'Z');
                if (arg) cz = (absPos?0:cx) + atof(arg+1);
                e.p1.set(cx, cy, cz);
                e.attr = 0;
                if (gCode==1) e.attr |= 1;
                // ignore the first segments if they move from the origin
                if (ignoreFirst) {
                    if (e.p0.x!=0.0 || e.p0.y!=0.0 || e.p0.z!=0.0)
                        ignoreFirst = false;
                } else {
                    nEdge++;
                }
            } else switch (gCode) {
                case 90: // Set to Absolute Positioning
                    absPos = true; break;
                case 91: // Set to Relative Positioning
                    absPos = false; break;
                default:
                    break;
            }
        }
    }
    // ignore the last segment. On some machines it moves away from the model.
    if (nEdge>0)
        nEdge--;
    return 1;
}

/*
NSSearchPathForDirectoriesInDomains
 NSUserDefaults or CFPreferences
*/

#include <CommonCrypto/CommonCryptor.h>
//#include <ZipKit/ZipKit.h>

#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


void test()
{
    const size_t offset = 0;
    
    const char *inName = "/Users/matt/Desktop/Machine Shop/Data 3D/Adam_Scheibe.3w";
    const char *outName = "/Users/matt/Desktop/Machine Shop/Data 3D/Adam_Scheibe.zip";
    
    fprintf(stderr, "ElektriktrickQL: testing decryption of %s\n", inName);
    
    // get the status of the given file. We need the file size.
    struct stat st;
    int ret = stat(inName, &st);
    if (ret==-1) {
        // ERROR, can't get file status
        fprintf(stderr, "ElektriktrickQL: no file status for %s\n", inName);
        return;
    }
    
    size_t dataInSize = st.st_size, dataOutSize = 0;
    unsigned char *dataIn = (unsigned char*)calloc(dataInSize, 1);
    unsigned char *dataOut = (unsigned char*)calloc(dataInSize, 1);

    FILE *in = fopen(inName, "rb");
    fread(dataIn, dataInSize, 1, in);
    fclose(in);
    
    CCCryptorStatus status;
    status = CCCrypt(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    status = CCCrypt(kCCDecrypt,
                     kCCAlgorithmAES,
                     kCCOptionPKCS7Padding,
                     "@xyzprinting.com", 16,
                     0L,
                     dataIn+offset, dataInSize-offset,
                     dataOut, dataInSize,
                     &dataOutSize);
    printf("Status: %d (%ld bytes in, %ld bytes out)\n", status, dataInSize, dataOutSize);

    if (dataOutSize) {
        FILE *out = fopen(outName, "wb");
        if (!out) {
            fprintf(stderr, "ElektriktrickQL: can't write %s\n", outName);
        }
        fwrite(dataIn, offset, 1, out);
        fwrite(dataOut, dataOutSize, 1, out);
        fclose(out);
    }
    fprintf(stderr, "\n----\n\n");
}

#if 0
// use this to decipher XYZPrinter GCode files (or the C++ equivalent):

private static void decode() throws Exception {
    String key = "@xyzprinting.com";
    byte[] kbb = key.getBytes("UTF-8");
    
    InputStream is = new FileInputStream("/tmp/davinci.3w");
    ByteArrayOutputStream bos = new ByteArrayOutputStream();
    int c;
    while ((c = is.read()) != -1)
        bos.write(c);
        byte[] bytes = bos.toByteArray();
        Cipher aes = Cipher.getInstance("AES/CBC/PKCS5Padding");
        SecretKeySpec spec = new SecretKeySpec(kbb, "AES");
        byte[] iv = new byte[16];   // Zero IV
        aes.init(Cipher.DECRYPT_MODE, spec, new IvParameterSpec(iv));
        FileOutputStream os = new FileOutputStream("/tmp/davinci.zip");
        for (int i = 0x2000; i < bytes.length; i += 0x2010) {
            os.write(aes.doFinal(bytes, i, Math.min(bytes.length - i, 0x2010)));
        }
    os.close();
}

#endif


