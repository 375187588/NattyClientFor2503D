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



#ifndef __NATTY_NETWORK_H__
#define __NATTY_NETWORK_H__

#include "soc_api.h"

#include "NattyAbstractClass.h"


#define CACHE_BUFFER_SIZE	1048
#define RECV_BUFFER_SIZE	CACHE_BUFFER_SIZE
#define SEND_BUFFER_SIZE	CACHE_BUFFER_SIZE
#define HTTP_BUFFER_SIZE	CACHE_BUFFER_SIZE


#define HEARTBEAT_TIMEOUT		25
#define P2P_HEARTBEAT_TIMEOUT	60
#define P2P_HEARTBEAT_TIMEOUT_COUNTR	5

#define  NATTY_APP_SIM1          1
#define  NATTY_APP_SIM2          2

#define  MAPN_WAP             0
#define  MAPN_NET             1
#define  MAPN_WIFI            2


#define MSIM_OPR_NONE         0
#define MSIM_OPR_CMCC         1
#define MSIM_OPR_UNICOM       2
#define MSIM_OPR_TELCOM       3
#define MSIM_OPR_WIFI         4
#define MSIM_OPR_UNKOWN       5



typedef void (*PROXY_CALLBACK)(int len);


typedef struct _NETWORK {
	const void *_;
	int	sockfd;
	sockaddr_struct addr;
	int length;	
	HANDLE_TIMER onAck;
	HANDLE_RECV onRecv;
	U32 ackNum;
	U8 buffer[CACHE_BUFFER_SIZE];
#if 1 //For mmi
	U32 accountId;
	kal_mutexid socketMutex;
	U8 busy;
#endif
	//void *timer;
} Network;


typedef struct _NETWORKOPERA {
	size_t size;
	void* (*ctor)(void *_self);
	void* (*dtor)(void *_self);
	int (*send)(void *_self, sockaddr_struct *to, U8 *buf, int len);
	int (*recv)(void *_self, U8 *buf, int len, sockaddr_struct *from);
	int (*resend)(void *_self);
	int (*connect)(void *_self);
} NetworkOpera;





void *ntyNetworkInstance(void);
int ntySendFrame(void *self, sockaddr_struct *to, U8 *buf, int len);
int ntyRecvFrame(void *self, U8 *buf, int len, sockaddr_struct *from);
int ntyConnect(void *self);
void ntySetAddr(sockaddr_struct *addr, U32 addrNum, U16 port);
void ntySetRecvProc(void *self, HANDLE_RECV func);
U8 ntyGetSocket(void *self);
U8* ntyGetBuffer(void *self);
void ntyGenCrcTable(void);
void ntySetOnAckProc(void *self, HANDLE_TIMER func);
U32 ntyGetAccountId(void *self);


#endif




