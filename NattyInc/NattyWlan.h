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
#ifndef __NATTY_WLAN_H__
#define __NATTY_WLAN_H__


#include "NattyAbstractClass.h"

#include "..\..\Service\WlanSrv\ScanOnly\wlansrvscanonly.h"
#include "wndrv_mmi_scanonly_msg.h"


#define WIFI_SCAN_NUMBER  7

#define WIFI_THRESHOLD_MIN_VAL        (-110)
#define WIFI_THRESHOLD_MAX_VAL       (-10)


typedef struct {
    unsigned char ssid[32];
    unsigned char mac[6];
    signed char rssi;
} WlanInfo_t;

typedef enum{
	WLAN_INFO_STATUS_NO = 0x00, //No valid wifi info
	WLAN_INFO_STATUS_1, // valid wifi bad,
	WLAN_INFO_STATUS_2, // valid wifi general,
	WLAN_INFO_STATUS_3, // valid wifi good,
	WLAN_INFO_STATUS_4, // valid wifi very good,
	WLAN_INFO_STATUS_5,//Wlan is very good, needn't to check wifi.
	
} WLAN_INFO_e;

typedef struct wifi_node{
    WlanInfo_t wifi_info;
    struct wifi_node *next;
} WifiInfoNode_t;


typedef struct WLAN {
	WlanInfo_t *list;
	WlanInfo_t *snd_list;
	U8 *ctr;
	U8 *snd_ctr;
	U8 *initflg;
	U8 *status;
} Wlan;

typedef struct WLAN_OPERA {
	size_t size;
	void* (*ctor)(void *_self);
	void* (*dtor)(void *_self);
	void* (*open)(void *_self);
	void* (*close)(void *_self);
	void* (*scan)(void *_self);
} WlanOpera;

extern WlanInfo_t wifi_list[WIFI_SCAN_NUMBER];
extern U8 wifi_ctr;
extern U8 flg_wifi_init;
extern U8 g_bWifiInfoStatus;

extern WlanInfo_t wifi_snd_list[WIFI_SCAN_NUMBER];
extern U8 wifi_snd_ctr;

void ntySetWifiEndCallback(HANDLE_LOCATION cb);
void ntyTimerScanWifi(void);
void ntyWifiStartTimer(void);



#endif


