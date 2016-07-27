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


#ifndef __NATTY_MAIN_H__
#define __NATTY_MAIN_H__

typedef enum 
{
	NETWORK_DETECT_APN_NONE = 0,
	NETWORK_DETECT_APN_OK,
	NETWORK_DETECT_APN_DOING,
	NETWORK_DETECT_APN_FAIL,
	NETWORK_DETECT_APN_STATUS_MAX
} NW_APN_Detect_status_e;


#define PROXY_DATA_BUFFER_SIZE			(1024-48)

#define NTY_PROTO_PROXY_DATA_SENDFRAME					0xA0
#define NTY_PROTO_PROXY_ACK								0xA1


#define NTY_PROTO_OPERA_GET_POWER_VALUE					0xB0
#define NTY_PROTO_OPERA_GET_SIGNAL_VALUE				0xB1
#define NTY_PROTO_OPERA_GET_PHONEBOOK					0xB2

#define NTY_PROTO_OPERA_SET_PHONEBOOK					0xC0

#define NTY_PROTO_OPERA_START_LOCATION					0xD0


void ntySendMsgToMMIMod(U8 command,U8 *param,U8 length);


#endif



