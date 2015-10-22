//
//  ETSerialPort.cpp
//  Elektriktrick
//
//  Created by Matthias Melcher on 9/30/15.
//  Copyright © 2015 M.Melcher GmbH. All rights reserved.
//

#include "ETSerialPort.h"

ETSerialPort::ETSerialPort(int x, int y, int w, int h, const char *name)
: Flio_Serial_Port(x, y, w, h, name)
{
}
