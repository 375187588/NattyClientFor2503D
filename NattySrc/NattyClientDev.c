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

//#include "YxBasicApp.h"

#include "NattyProtocol.h"
#include "NattyNetwork.h"
#include "NattyRBTree.h"
#include "NattyClientDev.h"
#include "NattyUtils.h"
#include "NattyUserDataProtocol.h"
#include "NattyMain.h"
#include "NattyHttp.h"

#include "TimerEvents.h"
#include "EventsGprot.h"
//#include "McfCommon.h"
#include "ps_public_enum.h"
#include "NwInfoSrvGprot.h"
#include "custom_data_account.h"
#include "DmSrvGprot.h"
#include "DtcntSrvGprot.h"
#include "DtcntSrvIprot.h"
#include "cbm_api.h"
#include "SimCtrlSrvGprot.h"

//#include "led_sn3770_display.h"


extern sockaddr_struct serveraddr;
static const C_DEVID devid = 0x01;
static C_DEVID friendId = 0x00;
static int level = LEVEL_LOGIN;
static int times = 0;

U8 ntyDevId[16] = {0xff};
U8 ntyAppId = 0;

int ntyGetNetworkLevel(void) {
	return level;
}

extern void heartbeatProc(void);

/*
 * p2p heartbeat ack
 * VERSION					1			BYTE
 * MESSAGE TYPE				1			BYTE (req, ack)
 * TYPE					1			BYTE 
 * DEVID					8			BYTE
 * ACKNUM					4			BYTE (Network Module Set Value)
 * DEST_DEVI				8			BYTE 
 * CRC 					4			BYTE (Network Module Set Value)
 * 
 * send to server addr
 */
int sendP2PHeartbeatAck(C_DEVID fromId, C_DEVID toId)  {
	U8 notify[NTY_LOGIN_ACK_LENGTH] = {0};
	int len = 0;
	sockaddr_struct friendaddr;
	void *pNetwork = ntyNetworkInstance();

	void *pTree = ntyRBTreeInstance();
	FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, toId);
	if (pFriend == NULL || pFriend->isP2P == 0) {
		printf(" Client Id : %lld, P2P is not Success\n", toId);
		return -1;
	}
	
	notify[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	notify[NTY_PROTO_MESSAGE_TYPE] = (U8)MSG_ACK;
	notify[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_HEARTBEAT_ACK;

#if 0	
	*(C_DEVID*)(&notify[NTY_PROTO_DEVID_IDX]) = fromId;
	*(C_DEVID*)(&notify[NTY_PROTO_DEST_DEVID_IDX]) = toId;
#else
	ntyU64Datacpy(&notify[NTY_PROTO_DEVID_IDX], fromId);
	ntyU64Datacpy(&notify[NTY_PROTO_DEST_DEVID_IDX], toId);
#endif
	len = NTY_PROTO_CRC_IDX + sizeof(U32);
#if 0
	struct sockaddr_in friendaddr;
	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = pFriend->addr;				
	friendaddr.sin_port = pFriend->port;
#else
	ntySetAddr(&friendaddr, pFriend->addr, pFriend->port);
#endif
	//pFriend->counter ++; //timeout count
	return ntySendFrame(pNetwork, &friendaddr, notify, len);
}


/*
 * p2p heartbeat Packet
 * VERSION					1			BYTE
 * MESSAGE TYPE				1			BYTE (req, ack)
 * TYPE					1			BYTE 
 * DEVID					8			BYTE
 * ACKNUM					4			BYTE (Network Module Set Value)
 * DEST_DEVI				8			BYTE 
 * CRC 					4			BYTE (Network Module Set Value)
 * 
 * send to server addr
 */
int sendP2PHeartbeat(C_DEVID fromId, C_DEVID toId)  {
	U8 notify[NTY_LOGIN_ACK_LENGTH] = {0};
	int len = 0;
	sockaddr_struct friendaddr;
	void *pNetwork = ntyNetworkInstance();

	void *pTree = ntyRBTreeInstance();
	FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, toId);
	if (pFriend == NULL || pFriend->isP2P == 0) {
		printf(" Client Id : %lld, P2P is not Success, state:%d\n", toId, pFriend->isP2P);
		return -1;
	}
	
	notify[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	notify[NTY_PROTO_MESSAGE_TYPE] = (U8)MSG_REQ;
	notify[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_HEARTBEAT_REQ;

#if 0	
	*(C_DEVID*)(&notify[NTY_PROTO_DEVID_IDX]) = fromId;
	*(C_DEVID*)(&notify[NTY_PROTO_DEST_DEVID_IDX]) = toId;
#else
	ntyU64Datacpy(&notify[NTY_PROTO_DEVID_IDX], fromId);
	ntyU64Datacpy(&notify[NTY_PROTO_DEST_DEVID_IDX], toId);
#endif
	
	len = NTY_PROTO_CRC_IDX + sizeof(U32);
#if 0
	struct sockaddr_in friendaddr;
	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = pFriend->addr; 			
	friendaddr.sin_port = pFriend->port;
#else
	ntySetAddr(&friendaddr, pFriend->addr, pFriend->port);
#endif


	pFriend->counter ++; //timeout count
	return ntySendFrame(pNetwork, &friendaddr, notify, len);
}



/*
 * Login Packet
 * VERSION					1			BYTE
 * MESSAGE TYPE				1			BYTE (req, ack)
 * TYPE					1			BYTE 
 * DEVID					8			BYTE
 * ACKNUM					4			BYTE (Network Module Set Value)
 * CRC 					4			BYTE (Network Module Set Value)
 * 
 * send to server addr
 */

int sendLoginPacket(void) {	
	int len;	
	U8 buf[CACHE_BUFFER_SIZE] = {0};
	U8 *pNum = (U8*)&devid;
	void *pNetwork = ntyNetworkInstance();

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;	
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_LOGIN_REQ;
#if 0
	*(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = devid;	
#else
	ntyU64Datacpy(buf+NTY_PROTO_LOGIN_REQ_DEVID_IDX, devid);
#if 0
	kal_prompt_trace(MOD_BT, " id:%x %x %x %x %x %x %x %x\n", *pNum,
		*(pNum+1), *(pNum+2), *(pNum+3), *(pNum+4), *(pNum+5), *(pNum+6),
		*(pNum+7));
#endif
#endif
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);				

	return ntySendFrame(pNetwork, &serveraddr, buf, len);
}

/*
 * Login Packet
 * VERSION					1			BYTE
 * MESSAGE TYPE				1			BYTE (req, ack)
 * TYPE					1			BYTE 
 * DEVID					8			BYTE
 * ACKNUM					4			BYTE (Network Module Set Value)
 * CRC 					4			BYTE (Network Module Set Value)
 * 
 * send to server addr
 */
int sendTimeCheckPacket(void) { 
	int len;	
	U8 buf[CACHE_BUFFER_SIZE] = {0};
	U8 *pNum = (U8*)&devid;
	void *pNetwork = ntyNetworkInstance();

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION; 
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ; 
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_TIME_CHECK_REQ;
#if 0
	*(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = devid;	
#else
	ntyU64Datacpy(buf+NTY_PROTO_LOGIN_REQ_DEVID_IDX, devid);

	kal_prompt_trace(MOD_BT, " sendTimeCheckPacket\n");

#endif
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);				

	return ntySendFrame(pNetwork, &serveraddr, buf, len);
}


/*
 * P2P Connect Req
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to friend addr
 *
 */

int sendP2PConnectReq(void* fTree, C_DEVID id) {
	//void *pRBTree = ntyRBTreeInstance();
	int len;	
	U8 buf[CACHE_BUFFER_SIZE] = {0};
	sockaddr_struct friendaddr;
	void *pNetwork = ntyNetworkInstance();

	FriendsInfo *client = ntyRBTreeInterfaceSearch(fTree, id);
	if (client == NULL || (client->isP2P == 1)){//
		printf(" Client is not exist or P2P State : %d\n", client->isP2P);
		return -1;
	} //

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_CONNECT_REQ;
#if 0	
	*(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]) = (C_DEVID) devid;
	*(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]) = id;
#else
	ntyU64Datacpy(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX], devid);
	ntyU64Datacpy(&buf[NTY_PROTO_DEST_DEVID_IDX], id);
#endif
	len = NTY_PROTO_CRC_IDX+sizeof(U32);

#if 0
	struct sockaddr_in friendaddr;
	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = pFriend->addr; 			
	friendaddr.sin_port = pFriend->port;
	printf("sendP2PConnectReq:%d.%d.%d.%d:%d\n", *(unsigned char*)(&friendaddr.sin_addr.s_addr), *((unsigned char*)(&friendaddr.sin_addr.s_addr)+1),													
				*((unsigned char*)(&friendaddr.sin_addr.s_addr)+2), *((unsigned char*)(&friendaddr.sin_addr.s_addr)+3),													
				friendaddr.sin_port);
#else
	ntySetAddr(&friendaddr, client->addr, client->port);
#endif
	//msgAck |= SIGNAL_P2PCONNECT_REQ;
	if (client->addr == 0 || client->port == 0
		||client->addr == 0xFFFFFFFF || client->port == 0xFFFF) {
		client->isP2P = 0;
		return -2;
	}
	return ntySendFrame(pNetwork, &friendaddr, buf, len);
	//n = sendto(sockfd_local, buf, len, 0, (struct sockaddr *)&friendaddr, sizeof(friendaddr));

}

/*
 * P2P Connect Req
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to friend addr
 *
 */

int sendP2PConnectAck(C_DEVID friId, U32 ack) {
	int len;	
	U8 buf[CACHE_BUFFER_SIZE] = {0};	
	void *pNetwork = ntyNetworkInstance();
	
	sockaddr_struct friendaddr;
	void *pTree = ntyRBTreeInstance();	
	FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friId);

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_CONNECT_ACK;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	

#if 0
	*(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]) = ack+1;
	*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = (C_DEVID) devid;	
	*(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]) = friId;
#else
	ntyU32Datacpy(&buf[NTY_PROTO_ACKNUM_IDX], ack+1);
	ntyU64Datacpy(&buf[NTY_PROTO_DEVID_IDX], devid);
	ntyU64Datacpy(&buf[NTY_PROTO_DEST_DEVID_IDX], friId);
#endif
	len = NTY_PROTO_CRC_IDX+sizeof(U32);	
#if 0
	struct sockaddr_in friendaddr;
	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = pFriend->addr; 			
	friendaddr.sin_port = pFriend->port;
#else
	ntySetAddr(&friendaddr, pFriend->addr, pFriend->port);
#endif

	return ntySendFrame(pNetwork, &friendaddr, buf, len);
	
}

/*
 * P2P DataPacket Req
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * DEST DEVID				8			 BYTE (friend devid)
 * CONTENT COUNT				2			 BYTE 
 * CONTENT					*(CONTENT COUNT)	 BYTE 
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to friend addr
 * 
 */
int sendP2PDataPacketReq(C_DEVID friId, U8 *buf, int length) {
	void *pNetwork = ntyNetworkInstance();

	sockaddr_struct friendaddr;
	void *pTree = ntyRBTreeInstance();	
	FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friId);
#if 0
	struct sockaddr_in friendaddr;
	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = pFriend->addr; 			
	friendaddr.sin_port = pFriend->port;
#else
	ntySetAddr(&friendaddr, pFriend->addr, pFriend->port);
#endif


	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2PDATAPACKET_REQ;
#if 0	
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = (C_DEVID) devid;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = friId;
#else
	ntyU64Datacpy(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX], devid);
	ntyU64Datacpy(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX], friId);
#endif
	*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)length;
	length += NTY_PROTO_DATAPACKET_CONTENT_IDX;
	length += sizeof(U32);

	return ntySendFrame(pNetwork, &friendaddr, buf, length);

}

/*
 * P2P DataPacket Ack
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * DEST DEVID				8			 BYTE (friend devid)
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to server addr, proxy to send one client
 * 
 */

int sendP2PDataPacketAck(C_DEVID friId, U32 ack) {
	int len;	
	U8 buf[CACHE_BUFFER_SIZE] = {0}; 
	void *pNetwork = ntyNetworkInstance();					

	sockaddr_struct friendaddr;
	void *pTree = ntyRBTreeInstance();	
	FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friId);
#if 0
	struct sockaddr_in friendaddr;
	friendaddr.sin_family = AF_INET;
	friendaddr.sin_addr.s_addr = pFriend->addr; 			
	friendaddr.sin_port = pFriend->port;
#else
	ntySetAddr(&friendaddr, pFriend->addr, pFriend->port);
#endif


	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_ACK; 
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2PDATAPACKET_ACK;

#if 0
	*(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]) = ack+1;
	*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = (C_DEVID) devid;		
	*(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]) = friId;
#else
	ntyU32Datacpy(&buf[NTY_PROTO_ACKNUM_IDX], ack+1);
	ntyU64Datacpy(&buf[NTY_PROTO_DEVID_IDX], devid);
	ntyU64Datacpy(&buf[NTY_PROTO_DEST_DEVID_IDX], friId);
#endif
	len = NTY_PROTO_CRC_IDX+sizeof(U32);

	return ntySendFrame(pNetwork, &friendaddr, buf, len);
}


/*
 * Server Proxy Data Transport
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * DEST DEVID				8			 BYTE (friend devid)
 * CONTENT COUNT				2			 BYTE 
 * CONTENT					*(CONTENT COUNT)	 BYTE 
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to server addr, proxy to send one client
 * 
 */

int sendProxyDataPacketReq(C_DEVID friId, U8 *buf, int length) {
	void *pNetwork = ntyNetworkInstance();

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_DATAPACKET_REQ;

	kal_prompt_trace(MOD_BT, "buf:%s, length:%d\n", buf+NTY_PROTO_DATAPACKET_CONTENT_IDX, length);
#if 0
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX]) = (C_DEVID) devid;
	*(C_DEVID*)(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX]) = friId;
#else
	ntyU64Datacpy(&buf[NTY_PROTO_DATAPACKET_DEVID_IDX], devid);
	ntyU64Datacpy(&buf[NTY_PROTO_DATAPACKET_DEST_DEVID_IDX], friId);
#endif
	ntyU16Datacpy(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX], (U16)length);
	//*(U16*)(&buf[NTY_PROTO_DATAPACKET_CONTENT_COUNT_IDX]) = (U16)length;
	length += NTY_PROTO_DATAPACKET_CONTENT_IDX;
	length += sizeof(U32);

	return ntySendFrame(pNetwork, &serveraddr, buf, length);
}

int sendProxyUserDataPacket(C_DEVID friId, U8 *buf, int length) {
	U8 data[CACHE_BUFFER_SIZE] = {0};


	memcpy(data+NTY_PROTO_DATAPACKET_CONTENT_IDX, buf, length);
	return sendProxyDataPacketReq(friId, data, length);
}

/*
 * Server Proxy Data Transport
 * VERSION					1			 BYTE
 * MESSAGE TYPE			 	1			 BYTE (req, ack)
 * TYPE				 	1			 BYTE 
 * DEVID					8			 BYTE (self devid)
 * ACKNUM					4			 BYTE (Network Module Set Value)
 * DEST DEVID				8			 BYTE (friend devid)
 * CRC 				 	4			 BYTE (Network Module Set Value)
 * 
 * send to server addr, proxy to send one client
 * 
 */

int sendProxyDataPacketAck(C_DEVID friId, U32 ack) {
	void *pNetwork = ntyNetworkInstance();
	//int len, i;	
	U8 buf[NTY_PROTO_DATAPACKET_ACK_LENGTH] = {0}; 

	memset(buf, 0, NTY_PROTO_DATAPACKET_ACK_LENGTH);
	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8)MSG_ACK; 
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_DATAPACKET_ACK;

	kal_prompt_trace(MOD_BT, "sendProxyDataPacketAck \r\n");
#if 0
	*(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]) = ack+1;
	*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = (C_DEVID) devid;	
	*(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]) = friId;
#else
	ntyU64Datacpy(&buf[NTY_PROTO_DEVID_IDX], devid);
	ntyU32Datacpy(&buf[NTY_PROTO_ACKNUM_IDX], ack);
	ntyU64Datacpy(&buf[NTY_PROTO_DEST_DEVID_IDX], friId);
#endif

	//kal_prompt_trace(MOD_BT, "sendProxyDataPacketAck 1111\r\n");
#if 0
	for (i = 0;i < NTY_PROTO_CRC_IDX;i++) {
		kal_prompt_trace(MOD_BT, "buf[%d]:%x\r\n", i, buf[i]);
	}
#endif	
	//len = NTY_PROTO_CRC_IDX+sizeof(U32);				

	return ntySendFrame(pNetwork, &serveraddr, buf, NTY_PROTO_CRC_IDX+sizeof(U32));
	
}

int sendP2PConnectNotify(C_DEVID fromId, C_DEVID toId) {
	U8 notify[NTY_LOGIN_ACK_LENGTH] = {0};
	int len = 0;
	void *pNetwork = ntyNetworkInstance();
	
	notify[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	notify[NTY_PROTO_MESSAGE_TYPE] = (U8)MSG_REQ;
	notify[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_NOTIFY_REQ;

#if 0	
	*(C_DEVID*)(&notify[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]) = fromId;
	*(C_DEVID*)(&notify[NTY_PROTO_P2P_NOTIFY_DEST_DEVID_IDX]) = toId;
#else
	ntyU64Datacpy(&notify[NTY_PROTO_P2P_NOTIFY_DEVID_IDX], fromId);
	ntyU64Datacpy(&notify[NTY_PROTO_P2P_NOTIFY_DEST_DEVID_IDX], toId);
#endif
	len = NTY_PROTO_P2P_NOTIFY_CRC_IDX + sizeof(U32);

	printf("send P2P Connect Notify\n");
	return ntySendFrame(pNetwork, &serveraddr, notify, len);
}


int sendP2PConnectNotifyAck(C_DEVID friId, U32 ack) {
	int len;	
	U8 buf[CACHE_BUFFER_SIZE] = {0}; 
	void *pNetwork = ntyNetworkInstance();
	
	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_P2P_NOTIFY_ACK;
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_ACK; 
	
#if 0
	*(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]) = ack+1;
	*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = (C_DEVID) devid;	
	*(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]) = friId;
#else
	ntyU32Datacpy(&buf[NTY_PROTO_ACKNUM_IDX], ack+1);
	ntyU64Datacpy(&buf[NTY_PROTO_DEVID_IDX], devid);
	ntyU64Datacpy(&buf[NTY_PROTO_DEST_DEVID_IDX], friId);
#endif
	len = NTY_PROTO_CRC_IDX+sizeof(U32);				

	return ntySendFrame(pNetwork, &serveraddr, buf, len);
	
}




/*
 * Send Packet Type : 
 * LOGIN
 * Heart Beat
 * LOGOUT
 * 
 * at same time
 * Heart Beat / P2P Heart Beat / Send Packet / LOGOUT
 * 
 */

extern kal_uint8 u8LedDisplayMode;
static U8 ntyUdpCallback(void * data)
{
    app_soc_notify_ind_struct * ind = (app_soc_notify_ind_struct *)data;
	//U8 sock = 0;
    int ret;
    Network *network = ntyNetworkInstance();
	NattyHttp *http = ntyHttpInstance();
	
	U8 buf[CACHE_BUFFER_SIZE] = {0};
	sockaddr_struct addr;

    if (NULL == ind) {
        return MMI_FALSE;
    }
    //kwp_debug_print("kwp_udp_callback: event_type=%d", ind->event_type);
	//sock = ntyGetSocket(network);
	
	kal_prompt_trace(MOD_BT, " ind->event_type ；%d, sock:%d, socket:%d, http:%d\n", 
						ind->event_type, ind->socket_id, ntyGetSocket(network), ntyGetHttpSocket(http));
	if (ind->socket_id == ntyGetHttpSocket(http)) {
		ret = ntyHttpHandleNotify(http, ind);
		nty_printf("nty notify result:%d\n", ret);
	} else if (ind->socket_id == ntyGetSocket(network)) {
        switch (ind->event_type) {
        case SOC_WRITE:
            break;
        case SOC_READ: {
			
            	ret = ntyRecvFrame(network, buf, CACHE_BUFFER_SIZE, &addr);
                //ret = soc_recvfrom(kwp_sock, protocol_recv_buf, PROTO_BUFF_SIZE, 0, &kwp_fromaddr);
                //kwp_debug_print("soc_recvfrom len=%d from %d.%d.%d.%d", ret,
                //                kwp_fromaddr.addr[0],kwp_fromaddr.addr[1],kwp_fromaddr.addr[2],kwp_fromaddr.addr[3]);
				kal_prompt_trace(MOD_BT,"%d.%d.%d.%d:%d size:%d --> %x\n", addr.addr[0], addr.addr[1],	
						addr.addr[2], addr.addr[3],	 addr.port, ret, buf[NTY_PROTO_TYPE_IDX]);
				if (ret > 0) {
					kal_prompt_trace(MOD_BT,"ntyRecvFrame 11111");
					if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_LOGIN_ACK) {
					//NTY_PROTO_LOGIN_ACK
						int i = 0;
						
						int count = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]);
						void *pTree = ntyRBTreeInstance();

						for (i = 0;i < count;i ++) {
							C_DEVID friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]);

							FriendsInfo *friendInfo = ntyRBTreeInterfaceSearch(pTree, friendId);
							if (NULL == friendInfo) {
								#if 0
								FriendsInfo *pFriend = (FriendsInfo*)malloc(sizeof(FriendsInfo));
								#else
								FriendsInfo *pFriend = (FriendsInfo*)OslMalloc(sizeof(FriendsInfo));
								
								#endif
								if (pFriend == NULL) {
									//kal_prompt_trace(MOD_YXAPP," malloc FriendsInfo failed");
									break;
								}
								pFriend->sockfd = ntyGetSocket(network);
								pFriend->isP2P = 0;
								pFriend->counter = 0;
								pFriend->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
								pFriend->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
								ntyRBTreeInterfaceInsert(pTree, friendId, pFriend);
							} else {
								friendInfo->sockfd = ntyGetSocket(network);
								friendInfo->isP2P = 0;
								friendInfo->counter = 0;
								friendInfo->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
								friendInfo->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
							}					
						}
#if 0							
						level = LEVEL_P2PCONNECT_NOTIFY;			
#else
						level = LEVEL_TIMECHECK;
#endif
					}else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_TIME_CHECK_ACK) {
						ntySetSystemTime(buf);	
						if (u8LedDisplayMode == 0) {
							u8LedDisplayMode = 1;
						}
						StartTimer(NATTY_HEARTBEAT_TIMER, 180 * 1000, heartbeatProc);
						level = LEVEL_DATAPACKET;
					} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_HEARTBEAT_ACK) {
					//NTY_PROTO_HEARTBEAT_ACK
						//kal_prompt_trace(MOD_BT,"NTY_PROTO_HEARTBEAT_ACK");
#if 1 //Update By WangBoJing From stack over
						if (buf[NTY_PROTO_MESSAGE_TYPE] == MSG_UPDATE) {
							int i = 0;
							
							U16 count = ntyU8ArrayToU16(&buf[NTY_PROTO_LOGIN_ACK_FRIENDS_COUNT_IDX]);
							void *pTree = ntyRBTreeInstance();

							kal_prompt_trace(MOD_BT,"NTY_PROTO_HEARTBEAT_ACK MSG_UPDATE");
							for (i = 0;i < count;i ++) {
								FriendsInfo *friendInfo = NULL;
								C_DEVID friendId = ntyU8ArrayToU64(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_DEVID_IDX(i)]);

								kal_prompt_trace(MOD_BT,"friendId :%d", friendId);
								friendInfo = ntyRBTreeInterfaceSearch(pTree, friendId);
								if (NULL == friendInfo) {
									#if 0
									FriendsInfo *pFriend = (FriendsInfo*)malloc(sizeof(FriendsInfo));
									#else
									FriendsInfo *pFriend = (FriendsInfo*)OslMalloc(sizeof(FriendsInfo));
									#endif
									if (pFriend == NULL) {
										kal_prompt_trace(MOD_BT," malloc FriendsInfo failed");
										break;
									}
									pFriend->sockfd = ntyGetSocket(network);
									pFriend->isP2P = 0;
									pFriend->counter = 0;
#if 0
									pFriend->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
									pFriend->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
#else
									pFriend->addr = ntyU8ArrayToU32(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
									pFriend->port = ntyU8ArrayToU16(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
#endif
									ntyRBTreeInterfaceInsert(pTree, friendId, pFriend);
								} else {
									friendInfo->sockfd = ntyGetSocket(network);
									friendInfo->isP2P = 0;
									friendInfo->counter = 0;
#if 0
									friendInfo->addr = *(U32*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
									friendInfo->port = *(U16*)(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
#else
									friendInfo->addr = ntyU8ArrayToU32(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_ADDR_IDX(i)]);
									friendInfo->port = ntyU8ArrayToU16(&buf[NTY_PROTO_LOGIN_ACK_FRIENDSLIST_PORT_IDX(i)]);
#endif
								}					
							}
							//level = LEVEL_P2PCONNECT_NOTIFY;
						}
#endif						
					}else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_DATAPACKET_REQ) {
						U32 ack0 = 0;
						U16 recByteCount0 = 0;
						C_DEVID friId = 0;
						U8 data[CACHE_BUFFER_SIZE] = {0};//NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_IDX
						
						friId = ntyU8ArrayToU64(buf+NTY_PROTO_DEVID_IDX);
						//kal_prompt_trace(MOD_BT,"friId:%d, recv:%d, ack:%d\n", friId, recByteCount0, ack0);
						ack0 = ntyU8ArrayToU32(buf+NTY_PROTO_ACKNUM_IDX);
						//kal_prompt_trace(MOD_BT,"friId:%d, recv:%d, ack:%d\n", friId, recByteCount0, ack0);
						recByteCount0 = ntyU8ArrayToU16(buf+NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_COUNT_IDX);
#if 1
						memcpy(data, &friId, sizeof(C_DEVID));//
						ack0 += 1;
						memcpy(data+sizeof(C_DEVID), &ack0, sizeof(U32));
						data[sizeof(C_DEVID)+sizeof(U32)] = '\0';

						ntySendMsgToMMIMod(NTY_PROTO_PROXY_ACK, data, sizeof(C_DEVID)+sizeof(U32)+1);

						memset(data, 0, CACHE_BUFFER_SIZE);
						memcpy(data, &friId, sizeof(C_DEVID));//sizeof(C_DEVID)
						kal_prompt_trace(MOD_BT,"buf:%x %x %x %x\n", buf[NTY_PROTO_DATAPACKET_CONTENT_IDX], buf[NTY_PROTO_DATAPACKET_CONTENT_IDX+1],
							buf[NTY_PROTO_DATAPACKET_CONTENT_IDX+2], buf[NTY_PROTO_DATAPACKET_CONTENT_IDX+3]);
						memcpy(data+sizeof(C_DEVID), buf+NTY_PROTO_DATAPACKET_CONTENT_IDX, recByteCount0);
						data[sizeof(C_DEVID)+recByteCount0] = '\0';
					
						ntySendMsgToMMIMod(NTY_PROTO_PROXY_DATA_SENDFRAME, data, sizeof(C_DEVID)+recByteCount0+1);
#else
						kal_prompt_trace(MOD_BT,"buf:%x %x %x %x\n", buf[NTY_PROTO_DATAPACKET_CONTENT_IDX], buf[NTY_PROTO_DATAPACKET_CONTENT_IDX+1],
							buf[NTY_PROTO_DATAPACKET_CONTENT_IDX+2], buf[NTY_PROTO_DATAPACKET_CONTENT_IDX+3]);
						mmi_frm_set_protocol_event_handler(MSG_ID_APP_SOC_NOTIFY_IND, network->onRecv, MMI_TRUE);
#endif
						//
					} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_DATAPACKET_ACK) {
						kal_prompt_trace(MOD_BT," send success\n");
					}
#if 0					
					else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_NOTIFY_ACK) {
					//NTY_PROTO_P2P_NOTIFY_ACK
						//P2PConnect Notify Success
						//void *pTree = ntyRBTreeInstance();
						//sendP2PConnectReq(pTree, friendId);
						//
						void *pTree = ntyRBTreeInstance();
						FriendsInfo *friendInfo = NULL;
						friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]);
						friendInfo = ntyRBTreeInterfaceSearch(pTree, friendId);
#if 1
						kal_prompt_trace(MOD_BT, "%d.%d.%d.%d:%d\n", *(unsigned char*)(&friendInfo->addr), *((unsigned char*)(&friendInfo->addr)+1),	
						*((unsigned char*)(&friendInfo->addr)+2), *((unsigned char*)(&friendInfo->addr)+3),	 friendInfo->port
						);
						
						kal_prompt_trace(MOD_BT, " Start to Connect P2P client\n");
#endif
						level = LEVEL_P2PCONNECTFRIEND;
					}  else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_CONNECT_REQ) {
						//NTY_PROTO_P2P_CONNECT_REQ

						U32 ack = *(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]);	
						void *pTree = ntyRBTreeInstance();	
						FriendsInfo *pFriend = NULL;
						
						friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]);

						pFriend = ntyRBTreeInterfaceSearch(pTree, friendId);
						if (pFriend != NULL) {
							pFriend->sockfd = ntyGetSocket(network);
							pFriend->addr = *((U32*)addr.addr);//addr.sin_addr.s_addr;
							pFriend->port = addr.port;//addr.sin_port;
							pFriend->isP2P = 0;
							pFriend->counter = 0;

							//kal_prompt_trace(MOD_YXAPP, " P2P client:%lld request connect\n", friendId);
						} else {
						#if 0
							FriendsInfo *friendInfo = (FriendsInfo*)malloc(sizeof(FriendsInfo));
						#else
							FriendsInfo *friendInfo = (FriendsInfo*)OslMalloc(sizeof(FriendsInfo));
						#endif
							if (pFriend == NULL) {
								//kal_prompt_trace(MOD_YXAPP," malloc FriendsInfo failed");
								break;
							}
							friendInfo->sockfd = ntyGetSocket(network);
							friendInfo->addr = *((U32*)addr.addr);//addr.sin_addr.s_addr;
							friendInfo->port = addr.port;//addr.sin_port;
							friendInfo->isP2P = 0;
							friendInfo->counter = 0;
							ntyRBTreeInterfaceInsert(pTree, friendId, friendInfo);
						}

						sendP2PConnectAck(friendId, ack);	
						level = LEVEL_P2PDATAPACKET;				
					} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_CONNECT_ACK) {
						//NTY_PROTO_P2P_CONNECT_ACK
						
						//level = LEVEL_P2PDATAPACKET;
						void *pTree = ntyRBTreeInstance();	
						FriendsInfo *pFriend = NULL;
						friendId = *(C_DEVID*)(&buf[NTY_PROTO_LOGIN_REQ_DEVID_IDX]);	

						pFriend = ntyRBTreeInterfaceSearch(pTree, friendId);
						if (pFriend != NULL) {
							pFriend->isP2P = 1; 
						}				
						
						//kal_prompt_trace(MOD_YXAPP, " P2P client %lld connect Success\n", friendId);
						level = LEVEL_P2PDATAPACKET;
					} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_NOTIFY_REQ) {	
						//NTY_PROTO_P2P_NOTIFY_REQ

						void *pTree = ntyRBTreeInstance();
						FriendsInfo *pFriend = NULL;
						U32 ack = *(U32*)(&buf[NTY_PROTO_P2P_NOTIFY_ACKNUM_IDX]);
						friendId =  *(C_DEVID*)(&buf[NTY_PROTO_P2P_NOTIFY_DEVID_IDX]);
						//kal_prompt_trace(MOD_YXAPP, " P2P Connect Notify: %lld\n", friendId);

						pFriend = ntyRBTreeInterfaceSearch(pTree, friendId);
						if (pFriend != NULL) {
							pFriend->sockfd = ntyGetSocket(network);
							pFriend->addr = *(U32*)(&buf[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]);
							pFriend->port = *(U16*)(&buf[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]);
							pFriend->isP2P = 0;
							pFriend->counter = 0;
							
						} else {
							#if 0
							FriendsInfo *pFriend = (FriendsInfo*)malloc(sizeof(FriendsInfo));
							#else
							FriendsInfo *pFriend = (FriendsInfo*)OslMalloc(sizeof(FriendsInfo));
							
							#endif
							if (pFriend == NULL) {
								break;
							}
							pFriend->sockfd = ntyGetSocket(network);
							pFriend->addr = *(U32*)(&buf[NTY_PROTO_P2P_NOTIFY_IPADDR_IDX]);
							pFriend->port = *(U16*)(&buf[NTY_PROTO_P2P_NOTIFY_IPPORT_IDX]);
							pFriend->isP2P = 0;
							pFriend->counter = 0;
							ntyRBTreeInterfaceInsert(pTree, friendId, pFriend);
						}
						//send ack to src devid
						sendP2PConnectNotifyAck(friendId, ack);
						//just now send p2p connect req
						//sendP2PConnectReq(pTree, friendId);
						level = LEVEL_P2PCONNECTFRIEND;
						
					}
					 else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2PDATAPACKET_REQ) {
						U8 data[CACHE_BUFFER_SIZE] = {0};
						U16 recByteCount = *(U16*)(&buf[NTY_PROTO_DATAPACKET_NOTIFY_CONTENT_COUNT_IDX]);
						C_DEVID friId = *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]);
						U32 ack = *(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]);
						
						void *pTree = ntyRBTreeInstance();
						FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, friendId);
						if (pFriend != NULL) {
							pFriend->isP2P = 1;
						}
						
						memcpy(data, buf+NTY_PROTO_DATAPACKET_CONTENT_IDX, recByteCount);
						//sendP2PDataPacketReq(friId, data);
						sendP2PDataPacketAck(friId, ack);
					} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2PDATAPACKET_ACK) {
						level = LEVEL_P2PDATAPACKET;
					} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_HEARTBEAT_REQ) {
						C_DEVID fromId = *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]);
						C_DEVID selfId = *(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]);
						void *pTree = ntyRBTreeInstance();
						FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, fromId);
						if (pFriend != NULL) {
							pFriend->counter = 0;
							pFriend->isP2P = 1;
						}
						sendP2PHeartbeatAck(selfId, fromId);
					} else if (buf[NTY_PROTO_TYPE_IDX] == NTY_PROTO_P2P_HEARTBEAT_ACK) {
						C_DEVID fromId = *(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]);
						C_DEVID selfId = *(C_DEVID*)(&buf[NTY_PROTO_DEST_DEVID_IDX]);
						void *pTree = ntyRBTreeInstance();
						FriendsInfo *pFriend = ntyRBTreeInterfaceSearch(pTree, fromId);
						if (pFriend != NULL) {
							pFriend->counter = 0;
							pFriend->isP2P = 1;
						}
					}
#endif
				}
                    //kwp_debug_print("soc_recvfrom rx ret = %d", ret);
				break;
            } 
        case SOC_CLOSE: {				
				break;
            }
        default: {
				break;
            }
        }
    }
    
    return MMI_TRUE;
}

static void ntyOnAckTimeOut(void) {
	kal_prompt_trace(MOD_BT," ntyOnAckTimeOut\n");
}
void test_buf(void) {
	C_DEVID testId = 0x05;
	C_DEVID tempId;
	U8 testBuf[16] = {0};
	int i = 0;
	U8* pNum = (U8*)(&testId);
#if 1
	kal_prompt_trace(MOD_BT, " test_buf testId:%x %x %x %x %x %x %x %x\n", *pNum, *(pNum+1), *(pNum+2), *(pNum+3), 
		*(pNum+4), *(pNum+5), *(pNum+6), *(pNum+7));

	ntyU64Datacpy(testBuf, testId);
	kal_prompt_trace(MOD_BT, " test_buf %x %x %x %x %x %x %x %x\n", *testBuf, *(testBuf+1), *(testBuf+2), *(testBuf+3), 
		*(testBuf+4), *(testBuf+5), *(testBuf+6), *(testBuf+7));
#else
	ntyU64Datacpy(testBuf, testId);
#endif
	memset(testBuf, 0, 16);
	testBuf[0] = 0x05;
	testBuf[1] = 0x00;
	testBuf[2] = 0x00;
	testBuf[3] = 0x00;
	testBuf[4] = 0x00;
	testBuf[5] = 0x00;
	testBuf[6] = 0x00;
	testBuf[7] = 0x00;

	tempId = *(C_DEVID*)testBuf;
#if 1
	kal_prompt_trace(MOD_MMI, " test_buf C_DEVID:%x\n", tempId);
#endif

}

void heartbeat(void) {
	int len, n;	
	U8 buf[NTY_HEARTBEAT_ACK_LENGTH] = {0};	
	//U8 *buf = (U8*)OslMalloc(NTY_HEARTBEAT_ACK_LENGTH*sizeof(U8));
	void *pNetwork = ntyNetworkInstance();

	memset(buf, 0, NTY_HEARTBEAT_ACK_LENGTH);

	buf[NEY_PROTO_VERSION_IDX] = NEY_PROTO_VERSION;	
	buf[NTY_PROTO_MESSAGE_TYPE] = (U8) MSG_REQ;	
	buf[NTY_PROTO_TYPE_IDX] = NTY_PROTO_HEARTBEAT_REQ;		
	//*(C_DEVID*)(&buf[NTY_PROTO_DEVID_IDX]) = devid;
	//memcpy(&buf[NTY_PROTO_DEVID_IDX], &devid, sizeof(C_DEVID));
	ntyU64Datacpy(&buf[NTY_PROTO_DEVID_IDX], devid);
	
	len = NTY_PROTO_LOGIN_REQ_CRC_IDX+sizeof(U32);
	n = ntySendFrame(pNetwork, &serveraddr, buf, len);

	
}

void heartbeatProc(void) {
	StopTimer(NATTY_HEARTBEAT_TIMER);
	heartbeat();
	StartTimer(NATTY_HEARTBEAT_TIMER, 180 * 1000, heartbeatProc);
}

static U8 u8TimeCheck = 0;
void sendProc(void) {
	StopTimer(NATTY_NETWORK_INIT_TIMER);
	if (level > LEVEL_TIMECHECK) {	
		return ;
	}
#if 0
	kal_prompt_trace(MOD_YXAPP, " sendProc : %x\n", level);
#endif
	if (level == LEVEL_LOGIN) {
		sendLoginPacket();
	} else if (level == LEVEL_TIMECHECK) {
		if (++u8TimeCheck >= 5) {
			ntyColdRestart();
		}
		sendTimeCheckPacket();
	} else if (level == LEVEL_P2PCONNECT_NOTIFY) {
		void *pTree = ntyRBTreeInstance();
		ntyFriendsTreeTraversalNotify(pTree, devid, sendP2PConnectNotify);
		level = LEVEL_DEFAULT;
	} else if (level == LEVEL_P2PCONNECTFRIEND) { 
		//
		//printf("LEVEL_P2PCONNECTFRIEND times : %d, friendId:%lld\n", times, friendId);
		if (times++ < 3) {
			void *pTree = ntyRBTreeInstance();
			sendP2PConnectReq(pTree, friendId);
		} else {
			times = 0;
			level = LEVEL_DATAPACKET;
		}
	} else if (level == LEVEL_P2PCONNECT) {
		//
		//printf("LEVEL_P2PCONNECT times : %d, friendId:%lld\n", times, friendId);
		if (times ++ < 3) {
			void *pTree = ntyRBTreeInstance();
			ntyFriendsTreeTraversal(pTree, sendP2PConnectReq);			
		} else {
			times = 0;
			level = LEVEL_DATAPACKET;
		}
	}

	StartTimer(NATTY_NETWORK_INIT_TIMER, 15 * 1000, sendProc);
}


#if 1 //Code From Coretek junping.qi@coretek.com
static BOOL GSM_CHECK_C = MMI_FALSE;
static BOOL check_nosim = MMI_FALSE;

void ntyGetGsmNetworkStatus(void)
{
	srv_nw_info_service_availability_enum service_availability = srv_nw_info_get_service_availability(MMI_SIM1);
	srv_sim_ctrl_ua_cause_enum cause = srv_sim_ctrl_get_unavailable_cause(MMI_SIM1);

	switch (cause)
	{
		case SRV_SIM_CTRL_UA_CAUSE_DISCONNECTED:
		case SRV_SIM_CTRL_UA_CAUSE_ACCESS_ERROR:
		case SRV_SIM_CTRL_UA_CAUSE_UBCHV1_BLOCKED:
			kal_prompt_trace(MOD_BT,"SIM卡错误 <<<666>>>");
		    if(GSM_CHECK_C == MMI_TRUE)
           	{
			   //GSM_CHECK_F = MMI_TRUE;
           	}
			break;	 
		case SRV_SIM_CTRL_UA_CAUSE_NOT_INSERTED:
			check_nosim = MMI_TRUE;
			kal_prompt_trace(MOD_IDLE,"没插入SIM卡 <<<555>>>");
            if(GSM_CHECK_C == MMI_TRUE)
           	{
			   //GSM_CHECK_F = MMI_TRUE;
           	}
			break;
	
		default:
			break;

	}

	if(service_availability == SRV_NW_INFO_SA_FULL_SERVICE)
	{
		kal_prompt_trace(MOD_IDLE,"网络正常 <<<777>>>");
		check_nosim = MMI_FALSE;
		GSM_CHECK_C = MMI_TRUE; //检查SIM是否有效状态
	}
	else if(check_nosim == MMI_FALSE)
	{
		kal_prompt_trace(MOD_IDLE,"无服务紧急电话 <<<111>>>");
	}
}


static char card_disapp_times=0;
static char sim_not_in=0;
static char sim_is_exit=0;	 
#define GPS_LOCATION_CMD_MAX_LEN 320

#endif



void ConnectProc(void) {
	int res = 0;
	void *pNetwork = ntyNetworkInstance();

	//StopTimer(NATTY_CONNECT_TIMER); 
#if 1
	if(srv_nw_info_get_service_availability(MMI_SIM1)!=SRV_NW_INFO_SA_FULL_SERVICE)
	{
		card_disapp_times++;
		sim_not_in=1;
		kal_prompt_trace(MOD_BT," no signal \n");
		StartTimer(NATTY_DETECT_SIM_STATUE_TIMER,3*1000, ConnectProc);
		if(card_disapp_times>=6)
		{
			//card_disapp_times=0;
			sim_is_exit=1;
		}

		if(srv_ucm_is_any_call())
			card_disapp_times=0;
		
		if(card_disapp_times>=30) {
			kal_prompt_trace(MOD_BT," network connect failed and restart \n");
			ntyColdRestart();
		}
		return;
	}
#endif
	ntyMMILmcGetICCIDReq(); //get iccid
	ntyMMILmcGetIMSIReq(); //get imsi
	ntySetVolumeLevelReq();

	res = ntyConnect(pNetwork); //creat sim apn / socket
	if (res >= 0) { //Connect Success
		//sendLoginPacket();
		//sendTimeCheckPacket();
		sendProc();
	} else {
		StartTimer(NATTY_NETWORK_INIT_TIMER, 5 * 1000, ConnectProc);
	}
	
#if 1
	kal_prompt_trace(MOD_MMI, " ntyConnect : %d\n", res);
#endif
	//return res;
}
extern void card_gps_cell_info();

int ntyClientDevInit(void) {
#if 1
	void *pNetwork = ntyNetworkInstance();
	ntySetRecvProc(pNetwork, ntyUdpCallback); //set recv proc
	//ntySetOnAckProc(pNetwork, ntyOnAckTimeOut);

	ntyGenCrcTable();// crc
#if 1
	kal_prompt_trace(MOD_BT, " ntyClientDevInit \n");
#endif

	ConnectProc();

	
#else
	//test_buf();
	card_gps_cell_info();
#endif
	
}

int ntyClientDevDeinit(void)
{
    mmi_frm_clear_protocol_event_handler(MSG_ID_APP_SOC_NOTIFY_IND, NULL);
#if 0
    if (kwp_sock >= 0) 
    {
        kwp_debug_print("close socket id %d", kwp_sock);
        kwp_udp_close(&kwp_sock);
    }
    kwp_network_overtime();

    memset(dl_voice_msg, 0, sizeof(dl_voice_msg));
#endif 
    return TRUE;
}



