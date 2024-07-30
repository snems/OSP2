/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/*****************************************************************************
 * name:		l_crc.c
 *
 * desc:		CRC calculation
 *
 * $Archive: /MissionPack/CODE/botlib/l_crc.c $
 *
 *****************************************************************************/

#include "../qcommon/q_shared.h"


// FIXME: byte swap?

// this is a 16 bit, non-reflected CRC using the polynomial 0x1021
// and the initial and final xor values shown below...  in other words, the
// CCITT standard CRC used by XMODEM

#define CRC_INIT_VALUE	0xffff
#define CRC_XOR_VALUE	0x0000

// make int because qvm toolchain not support unit16_t initialization
static unsigned int crctableint[129] = 
{
  0x10210000,0x30632042,0x50a54084,0x70e760c6,0x91298108,0xb16ba14a,0xd1adc18c,0xf1efe1ce,
  0x02101231,0x22523273,0x429452b5,0x62d672f7,0x83189339,0xa35ab37b,0xc39cd3bd,0xe3def3ff,
  0x34432462,0x14010420,0x74c764e6,0x548544a4,0xb54ba56a,0x95098528,0xf5cfe5ee,0xd58dc5ac,
  0x26723653,0x06301611,0x66f676d7,0x46b45695,0xa77ab75b,0x87389719,0xe7fef7df,0xc7bcd79d,
  0x58e548c4,0x78a76886,0x18610840,0x38232802,0xd9edc9cc,0xf9afe98e,0x99698948,0xb92ba90a,
  0x4ad45af5,0x6a967ab7,0x0a501a71,0x2a123a33,0xcbdcdbfd,0xeb9efbbf,0x8b589b79,0xab1abb3b,
  0x7c876ca6,0x5cc54ce4,0x3c032c22,0x1c410c60,0xfd8fedae,0xddcdcdec,0xbd0bad2a,0x9d498d68,
  0x6eb67e97,0x4ef45ed5,0x2e323e13,0x0e701e51,0xefbeff9f,0xcffcdfdd,0xaf3abf1b,0x8f789f59,
  0x81a99188,0xa1ebb1ca,0xc12dd10c,0xe16ff14e,0x00a11080,0x20e330c2,0x40255004,0x60677046,
  0x939883b9,0xb3daa3fb,0xd31cc33d,0xf35ee37f,0x129002b1,0x32d222f3,0x52144235,0x72566277,
  0xa5cbb5ea,0x858995a8,0xe54ff56e,0xc50dd52c,0x24c334e2,0x048114a0,0x64477466,0x44055424,
  0xb7faa7db,0x97b88799,0xf77ee75f,0xd73cc71d,0x36f226d3,0x16b00691,0x76766657,0x56344615,
  0xc96dd94c,0xe92ff90e,0x89e999c8,0xa9abb98a,0x48655844,0x68277806,0x08e118c0,0x28a33882,
  0xdb5ccb7d,0xfb1eeb3f,0x9bd88bf9,0xbb9aabbb,0x5a544a75,0x7a166a37,0x1ad00af1,0x3a922ab3,
  0xed0ffd2e,0xcd4ddd6c,0xad8bbdaa,0x8dc99de8,0x6c077c26,0x4c455c64,0x2c833ca2,0x0cc11ce0,
  0xff3eef1f,0xdf7ccf5d,0xbfbaaf9b,0x9ff88fd9,0x7e366e17,0x5e744e55,0x3eb22e93,0x1ef00ed1,
};

unsigned short *crctable =(unsigned short*)crctableint;

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void CRC_Init(unsigned short *crcvalue)
{
	*crcvalue = CRC_INIT_VALUE;
} //end of the function CRC_Init
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void CRC_ProcessByte(unsigned short *crcvalue, byte data)
{
	*crcvalue = (*crcvalue << 8) ^ crctable[(*crcvalue >> 8) ^ data];
} //end of the function CRC_ProcessByte
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
unsigned short CRC_Value(unsigned short crcvalue)
{
	return crcvalue ^ CRC_XOR_VALUE;
} //end of the function CRC_Value
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
unsigned short CRC_ProcessString(const unsigned char *data, int length)
{
	unsigned short crcvalue;
	int i, ind;

	CRC_Init(&crcvalue);

	for (i = 0; i < length; i++)
	{
		ind = (crcvalue >> 8) ^ data[i];
		if (ind < 0 || ind > 256) ind = 0;
		crcvalue = (crcvalue << 8) ^ crctable[ind];
	} //end for
	return CRC_Value(crcvalue);
} //end of the function CRC_ProcessString
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void CRC_ContinueProcessString(unsigned short *crc, const char *data, int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		*crc = (*crc << 8) ^ crctable[(*crc >> 8) ^ data[i]];
	} //end for
} //end of the function CRC_ProcessString
