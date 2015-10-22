//
//  ETSerialPort.hpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 9/30/15.
//  Copyright © 2015 M.Melcher GmbH. All rights reserved.
//

#ifndef ETSerialPort_hpp
#define ETSerialPort_hpp

#include "Flio_Serial_Port.h"

class ETSerialPort;

typedef void ETSerialPortCallback(ETSerialPort*, char*);

class ETSerialPort : public Flio_Serial_Port
{
public:
    ETSerialPort(int x, int y, int w, int h, const char *name=0);
    void SetXMLCallback(ETSerialPortCallback *);
    void SetMsgCallback(ETSerialPortCallback *);
private:
    ETSerialPortCallback *pXMLCallback;
    char *pXMLBuffer;
    int pXMLBufferPos, pXMLBufferSize;
    ETSerialPortCallback *pMsgCallback;
    char *pMsgBuffer;
    int pMsgBufferPos, pMsgBufferSize;
};


#endif /* ETSerialPort_hpp */
