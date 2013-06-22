#ifndef _COM_PORT_H
#define _COM_PORT_H

#include "stdafx.h"

enum SERIAL_PORT_TYPE
{
	PORT_DA2,PORT_BV,PORT_UNKNOWN
};

SERIAL_PORT_TYPE GetPortType(const unsigned long portnum);

#endif
