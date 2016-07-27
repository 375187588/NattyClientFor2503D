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

//this code from haichao
#ifndef __NATTY_GPS_H__
#define __NATTY_GPS_H__

#include "NattyUtils.h"
#include "NattyAbstractClass.h"


typedef struct {
    unsigned char time[12];
    unsigned char satellite_num[4];
    unsigned char longitude[12];
    unsigned char latitude[12];
    unsigned char gps_status[1];
} GPSINFO_t;

typedef struct {
    double  latitude;
    double  longitude;
    kal_uint16 station_id;
    kal_uint8     sat_in_view;
    kal_uint32    uiTime;
} GPSINFO_t_EX;

#define GPS_LOC_SCAN_TIMEOUT            95 //uinit:second.1.5Min
#define GPS_LOC_MONITOR_TIMEOUT        	45   //Uint:second
#define GPS_LOCATION_CMD_MAX_LEN		256

typedef void (*ntyGpsSyncDatetimeCallBack)(applib_time_struct *time);


U8 ntyGPSOpen(void);
U8 ntyGPSClose(void);

void ntyGPSStartScan(void);
void ntyGpsSyncEnable(void);
void ntyGpsScanStop(void);

//void ntySetAltitudeCallback(HANDLE_LOCATION cb);

void ntySetNMEAGGACallback(HANDLE_LOCATION cb);
void ntySetNMEARMCCallback(HANDLE_LOCATION cb);
void ntySetPMTK741CmdCallback(HANDLE_LOCATION cb);


#endif



