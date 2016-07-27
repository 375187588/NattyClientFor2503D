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



#ifndef __NATTY_HTTP_H__
#define __NATTY_HTTP_H__

#include "kal_public_api.h"
#include "NattyProtocol.h"
#include "NattyUtils.h"
#include "NattyAbstractClass.h"
#include "NattyNetwork.h"

#define SERVER_GAODE_GPS_TEST  "http://apilocate.amap.com/position?accesstype=1&imei=352315052834187&smac=E0:DB:55:E4:C7:49&mmac=22:27:1d:20:08:d5,-55,CMCC-EDU&macs=22:27:1d:20:08:d5,-55,CMCC-EDU|5c:63:bf:a4:bf:56,-86,TP-LINK|d8:c7:c8:a8:1a:13,-42,TP-LINK%20&output=xml&key=81040f256992a218a8a20ffb7f13ba9f"
#define GAODE_SERVER_NAME       "apilocate.amap.com"
#define GAODE_DEVELOP_KEY		"81040f256992a218a8a20ffb7f13ba9f"
#define GAODE_WIFI_LABS_LOCATION_URL	"http://apilocate.amap.com/position?accesstype=0&imei=352315052834187"
//macs=22:27:1d:20:08:d5,-55,CMCC-EDU|5c:63:bf:a4:bf:56,-86,TP-LINK|d8:c7:c8:a8:1a:13,-42,TP-LINK%20&output=xml&key=81040f256992a218a8a20ffb7f13ba9f

//Gao Cell Location Example
//http://apilocate.amap.com/position?accesstype=1&imei=352315052834187&cdma=0&bts=460,01,20854,48402,-33&nearbts=460,01,20854,48401,-22|460,01,20854,2521,-20|460,01,20854,1053,-19|460,01,20854,48403,-13|460,01,20854,1022,-13|460,01,20854,6021,-12&output=xml&key=81040f256992a218a8a20ffb7f13ba9f

typedef enum {
	NTY_HTTP_FAILED = -1,    //失败
	NTY_HTTP_SUCCESS = 0,    //成功
	NTY_HTTP_IGNORE = 1,     //不关心
	NTY_HTTP_WAITING = 2,     //异步(非阻塞)模式
} HTTP_RESULT;

typedef struct _NATTYHTTP {
	const void *_;
	kal_int8 socketid;
	U8 hostName[NTY_HTTP_DOMAIN_LENGTH];
	sockaddr_struct addr; 
	U8 buffer[HTTP_BUFFER_SIZE];
	//U8 recvBuffer[RECV_BUFFER_SIZE];
	//U8 sendBuffer[SEND_BUFFER_SIZE];
	int sendLength;
} NattyHttp;

typedef struct _NATTYHTTPHANDLE {
	size_t size;
	void* (*ctor)(void *_self);
	void* (*dtor)(void *_self);
	HTTP_RESULT (*create)(void *_self);
	HTTP_RESULT (*connect)(void *_self,  void *param); 
	U8 (*response)(void *_self, int length);
	kal_int32 (*request)(void *_self);
	U8 (*notify)(void *_self, void *param);
} NattyHttpHandle;


void *ntyHttpInstance(void);

sockaddr_struct *ntyGetHostAddr(void *self);
U8 ntyHostNameHaveParsed(void *self);
kal_int8 ntyGetHttpSocket(void *self);
int ntyHttpHandleNotify(void *self, void *param);
HTTP_RESULT ntyConnectServer(void *self);
U8 *ntyGetSendBuffer(void *self);




#endif



