/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2016
 * 
 *
 
****       *****
  ***        *
  ***        *                         *               *
  * **       *                         *               *
  * **       *                         *               *
  *  **      *                        **              **
  *  **      *                       ***             ***
  *   **     *       ******       ***********     ***********    *****    *****
  *   **     *     **     **          **              **           **      **
  *    **    *    **       **         **              **           **      *
  *    **    *    **       **         **              **            *      *
  *     **   *    **       **         **              **            **     *
  *     **   *            ***         **              **             *    *
  *      **  *       ***** **         **              **             **   *
  *      **  *     ***     **         **              **             **   *
  *       ** *    **       **         **              **              *  *
  *       ** *   **        **         **              **              ** *
  *        ***   **        **         **              **               * *
  *        ***   **        **         **     *        **     *         **
  *         **   **        **  *      **     *        **     *         **
  *         **    **     ****  *       **   *          **   *          *
*****        *     ******   ***         ****            ****           *
                                                                       *
                                                                      *
                                                                  *****
                                                                  ****


 *
 */



#ifndef __NATTY_LOCATION_H__
#define __NATTY_LOCATION_H__


#include "NattyWlan.h"
#include "NattyGPS.h"
#include "NattyCell.h"
#include "NattyHttp.h"
#include "NattyUtils.h"


#define LON_LAT_LENGTH		16

typedef enum {
	LOCATION_FREE = 0x00,
	LOCATION_WLAN = 0x01,
	LOCATION_GPS = 0x02,
	LOCATION_CELL = 0x03,
	LOCATION_EPO = 0x04,
	LOCATION_NULL = 0xFF,
} LOCALTION_TYPE;

typedef struct LOCATION {
	const void *_;
	U32 u32LocTime;
	U32 u32EpoTime;
	U8 u8Lon[LON_LAT_LENGTH];
	U8 u8Lat[LON_LAT_LENGTH];
	U8 u8Altitude[LON_LAT_LENGTH];
	U8 u8LocType;
	U8 u8CurLocLevel;
} Location;

typedef struct LOCATIONHANDLE {
	size_t size;
	void* (*ctor)(void *_self);
	void* (*dtor)(void *_self);
	void (*wstart)(void *_self);
	void (*wend)(void *_self);
	void (*gstart)(void *_self);
	void (*gend)(void *_self);
	void (*cstart)(void *_self);
	void (*cend)(void *_self);
} LocationHandle;

//typedef void (*)(void *user_data);
void ntyLocationWlanStart(void *data);

void ntyLocationCellStart(void *data);

void *ntyLocationInstance(void);
void ntyStartGPS(void *self);
void ntySetAltitude(void *data);
void ntySetPMTK741Cmd(void *data);
void ntyParseNMEAGGA(void *data);
void ntyParseNMEARMC(void *data);


#endif


