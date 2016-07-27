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

#include "NattyHttp.h"
#include "NattyLocation.h"

#include "mmi_frm_events_gprot.h"

kal_int32 ntyHttpSend(int sockId, U8 *buffer, int length) {
	kal_int32	ret_val;

	ret_val = soc_send(sockId, buffer, length, 0);
	
	if (ret_val > 0) {
		return ret_val;
	} else if (ret_val == SOC_WOULDBLOCK||ret_val == 0) {
		return 0;
	} else {
		return -1;
	}
}

kal_int32 ntyHttpRecv(int sockId, U8 *buffer, int length) {
	kal_int32	ret_val;

	memset(buffer, 0, length);
	ret_val = soc_recv(sockId, buffer, length, 0);
	
	if (ret_val > 0) {
		//parse http response
		return ret_val;
	} else if (ret_val == SOC_WOULDBLOCK||ret_val == 0) {
		return 0;
	} else {
		return -1;
	}
}

kal_int32 ntyHttpClose(int sockId) {
	soc_shutdown(sockId, SHUT_RDWR);
	soc_close(sockId);
	ClearProtocolEventHandler(MSG_ID_APP_SOC_GET_HOST_BY_NAME_IND);
}

kal_int32 ntyHttpRequest(void *self) {
	//SERVER_GPS_TEST
	NattyHttp *http = self;
	int i = 0;
	
#if 0
	U8 CRLF[] = {0x0d,0x0a,0x00};
	memset(http->buffer, 0, HTTP_BUFFER_SIZE);

	strcat(http->buffer,"GET ");
	strcat(http->buffer, SERVER_GAODE_GPS_TEST);
	strcat(http->buffer," HTTP/1.1");
	strcat(http->buffer,CRLF);
	strcat(http->buffer,"Host: apilocate.amap.com");
	strcat(http->buffer,CRLF);
	strcat(http->buffer,"Cache-Control: no-cache");
	strcat(http->buffer,CRLF);
	strcat(http->buffer,"Pragma: no-cache");
	//strcat(http->buffer,CRLF);
	//strcat(http->buffer,"key: 8c1d26efa39000a231cf8d073467ea36");
	strcat(http->buffer,CRLF);
	strcat(http->buffer,CRLF);
	http->sendLength = strlen(http->buffer);

	
#else
#endif
	nty_printf("length:%d\n", http->sendLength);

	for (i = 0;i < http->sendLength;i += 50) {
		nty_printf("%s", http->buffer+i);
	}

	return ntyHttpSend(http->socketid, http->buffer, http->sendLength);
}

U8 ntyHttpResponse(void *self, int length) {
	NattyHttp *http = self;
	int i = 0, j, k = 0;
	int len = strlen(http->buffer);
	Location *loc = ntyLocationInstance();

#if 0
	nty_printf("length:%d\n", len);
	for (i = 0;i < len;i += 20) {
		nty_printf("response:%s\n", http->buffer+i);
	}
#else
	const U8 *pattern_start = "<location>";
	const U8 *pattern_end = "</location>";

	int len_start = strlen(pattern_start);
	int len_end = strlen(pattern_end);

#define PATTERN_COUNT 			16
#define LOCATION_INFO_COUNT		64

	U32 start_matches[PATTERN_COUNT] = {0};
	U32 end_matches[PATTERN_COUNT] = {0};
	U8 location[LOCATION_INFO_COUNT] = {0};


	ntyKMP(http->buffer, len, pattern_start, len_start, start_matches);
	ntyKMP(http->buffer, len, pattern_end, len_end, end_matches);

	for (i = 0;i < PATTERN_COUNT;i ++) {
		if (start_matches[i] != 0 && start_matches[i] < end_matches[i]) {
			for (j = start_matches[i]+len_start;j < end_matches[i];j ++) {
				location[(k >= LOCATION_INFO_COUNT ? 0 : k++)] = http->buffer[j];
			}
			nty_printf("location:%s\n", location);

			k = 0;
			while (location[k++] != ',');
				
			memcpy(loc->u8Lon, location, k-1);
			memcpy(loc->u8Lat, location+k, strlen(location)-k);
			
			
			nty_printf("lat:%s, lon:%s\n", loc->u8Lat, loc->u8Lon);
		}
	}

	loc->u32LocTime = ntyGetUtcTime();
	if (loc->u8CurLocLevel == LOCATION_EPO) {
		loc->u8LocType = LOCATION_CELL;
		loc->u32EpoTime = ntyGetUtcTime();
		ntyStartGPS(loc);
	} else if (loc->u8CurLocLevel == LOCATION_CELL) {
		loc->u8LocType = LOCATION_CELL;
	} else if (loc->u8CurLocLevel == LOCATION_WLAN) {
		loc->u8LocType = LOCATION_WLAN;
	}
#endif
	return 0;
}


U8 ntyHttpNotify(void *self, void* inMsg) {
	NattyHttp *http = self;
	kal_int8 ret_val;
	app_soc_notify_ind_struct *soc_notify = (app_soc_notify_ind_struct *) inMsg;
	
	nty_printf(" soc_notify->result %d,socket_id %d,event_type %d\r\n",soc_notify->result,soc_notify->socket_id,soc_notify->event_type);
	
	if(soc_notify == NULL) {
		nty_printf("soc_notify == NULL \r\n");
		return -2;
	}

	switch( soc_notify->event_type)
	{
		case SOC_CONNECT: {
			//send http
			ntyHttpRequest(http);
			break;
		}
		case SOC_WRITE: {
			nty_printf("SOC_WRITE ÒÑ·¢ËÍ\r\n");
			break;
		}
		case SOC_READ: {
			//recv response
			ret_val = ntyHttpRecv(http->socketid,http->buffer, HTTP_BUFFER_SIZE);
			ntyHttpResponse(http, ret_val);
			break;
		}
		case SOC_CLOSE: {
			ntyHttpClose(http->socketid);
			nty_printf("SOC_CLOSE\r\n");
			break;
		}
		case SOC_ACCEPT: {
			nty_printf("SOC_ACCEPT\r\n");
			break;
		}
		//default:
		//{			 
		//	break;
		//}
	}
    return 0;
}



HTTP_RESULT ntyHttpConnect(void *self) {
	kal_int8 ret_val;	
	NattyHttp *http = (NattyHttp*)self;
	int socketId = http->socketid;
	sockaddr_struct *add = &http->addr;
		
	nty_printf("ntyHttpConnect %d,%d,%d,%d,%d",add->addr[0],add->addr[1],add->addr[2],add->addr[3],add->port );

	ret_val = soc_connect(socketId, add);
	if(ret_val == SOC_SUCCESS) {
		nty_printf("SOC_SUCCESS_gps_connect\r\n");
		return NTY_HTTP_SUCCESS;
	} else if(ret_val == SOC_WOULDBLOCK) {
		//ClearProtocolEventHandler(MSG_ID_APP_SOC_NOTIFY_IND);
		//SetProtocolEventHandler(gps_dsm_soc_socket_notify, MSG_ID_APP_SOC_NOTIFY_IND);
		nty_printf("SOC_WOULDBLOCK\r\n");
	} else {
		nty_printf("DSM_SOC_ERR\r\n");
		return NTY_HTTP_FAILED;
	}

}	 

static void ntyMMISocketAddrCopy(sockaddr_struct *dest, sockaddr_struct *src) {
	memcpy(dest, src, sizeof(sockaddr_struct));
}

void ntyGetHostByNameIndCallback(void *inMsg) {
	NattyHttp *http = ntyHttpInstance();
	app_soc_get_host_by_name_ind_struct *dns_ind;
	//sockaddr_struct server_ip_addr;

	nty_printf("card_gps_get_host_by_name_ind_plateform \r\n");
	ClearProtocolEventHandler(MSG_ID_APP_SOC_GET_HOST_BY_NAME_IND);
	if (!inMsg) {
		return;
	}
	dns_ind = (app_soc_get_host_by_name_ind_struct*) inMsg;
	nty_printf("card_gps_get_host_by_name_ind_plateform: len=%d: addr=%d.%d.%d.%d \r\n", dns_ind->addr_len, dns_ind->addr[0], dns_ind->addr[1], dns_ind->addr[2], dns_ind->addr[3]);
	/* Check if the result is OK */
	if (dns_ind->result == KAL_TRUE) {
		memset(&http->addr, 0x00, sizeof(sockaddr_struct));
		memcpy(http->addr.addr, dns_ind->addr, dns_ind->addr_len);
		http->addr.addr_len = dns_ind->addr_len;
		nty_printf("card_gps_get_host_by_name_ind_plateform srv_socketid\r\n");
#ifdef stu_card
		http->addr.port=80;
#else
		http->addr.port=80;
#endif
		http->addr.sock_type=0;

		ntyMMISocketAddrCopy(&http->addr, &http->addr);
		ntyHttpConnect(http);
		//gps_connect(http->socketid, &http->addr);
	} else {
	 //gprs_active= KAL_FALSE;
	 //gps_closeSocket(srv_socketid);
	 //StartTimer(ABREPEAT_BUILD_CACHE_DELAY_TIMER, 10*1000, card_conncet_to_platform);
	}
	 //plateform_connecting=0;
}


void ntyGetHostByName(char *domainName) {	
	S32 ret;
	//S8 char_plmn[SRV_MAX_PLMN_LEN + 1];
	kal_uint32 acct_id= CBM_INVALID_NWK_ACCT_ID;
	kal_uint8 addr_len = 0;
	sockaddr_struct *add;
	void *pNetwork = ntyNetworkInstance();
	
	acct_id = ntyGetAccountId(pNetwork);;
	nty_printf("acct_id %d\r\n",acct_id);
	
	ret = soc_gethostbyname(KAL_FALSE, MOD_MMI, 1, domainName, add->addr, &addr_len,0,acct_id);
	kal_prompt_trace(MOD_IDLE,"card_get_host_by_name_plateform22222 %d\r\n",ret);

	if (ret == SOC_SUCCESS) {          // success
		nty_printf("SOC_SUCCESS_gethostbyname \r\n");
		//gps_connect();
	}
	else if (ret == SOC_WOULDBLOCK)  {       // block
		nty_printf("SOC_WOULDBLOCK_gethostbyname \r\n");
		SetProtocolEventHandler(ntyGetHostByNameIndCallback, MSG_ID_APP_SOC_GET_HOST_BY_NAME_IND);
	}
}


void* ntyHttpCtor(void *_self) {
	NattyHttp *http = (NattyHttp*)_self;

	memset(http->hostName, 0, NTY_HTTP_DOMAIN_LENGTH);
	memset(&http->addr, 0, sizeof(sockaddr_struct));
	memset(http->buffer, 0, HTTP_BUFFER_SIZE);
	http->socketid = -1;
	
}

void* ntyHttpDtor(void *_self) {
	;
}

HTTP_RESULT ntyHttpCreate(void *_self) {
	NattyHttp *http = (NattyHttp*)_self;
	kal_uint32 acct_id = 0;
	kal_uint8   val; 
	void *pNetwork = ntyNetworkInstance();

	acct_id = ntyGetAccountId(pNetwork);
	if (acct_id == 0) {
		nty_printf("acct_id don't init \r\n");
		return NTY_HTTP_FAILED;
	}

	if (!ntyHostNameHaveParsed(http)) {
		strcpy(http->hostName, GAODE_SERVER_NAME);
		ntyGetHostByName(http->hostName);
	}

	http->socketid = soc_create(SOC_PF_INET, SOC_SOCK_STREAM, 0, MOD_MMI, acct_id);

	if(http->socketid >= 0) {
		val = KAL_TRUE;
		soc_setsockopt((kal_uint8)http->socketid, SOC_NBIO, &val, sizeof(val));

		val = SOC_WRITE | SOC_READ | SOC_CONNECT | SOC_CLOSE;
		soc_setsockopt((kal_uint8)http->socketid,SOC_ASYNC, &val, sizeof(val));
		nty_printf("{%s:%d} --> socketid : %d\r\n", __FILE__, __LINE__, http->socketid);
		return NTY_HTTP_SUCCESS;
	} else {
		return NTY_HTTP_FAILED;
	}
}


static const NattyHttpHandle ntyNattyHandle = {
	sizeof(NattyHttp),
	ntyHttpCtor,
	ntyHttpDtor,
	ntyHttpCreate,
	ntyHttpConnect,
	ntyHttpResponse,
	ntyHttpRequest,
	ntyHttpNotify,
};

const void *pNtyHttpHandle = &ntyNattyHandle;

static void *pHttpHandle = NULL;

void *ntyHttpInstance(void) {
	if (pHttpHandle == NULL) {
		pHttpHandle = New(pNtyHttpHandle);
	}
	return pHttpHandle;
}

void ntyHttpRelease(void *self) {	
	Delete(self);
}

/*
 *
 */
sockaddr_struct *ntyGetHostAddr(void *self) {
	NattyHttp *http = (NattyHttp*)self;
	return (&http->addr);
}

U8 ntyHostNameHaveParsed(void *self) {
	NattyHttp *http = (NattyHttp*)self;
	if (strlen(http->hostName) == 0 || http->addr.addr[0] == 0) {
		return 0; //no parse
	} 

	return 1;
}

kal_int8 ntyGetHttpSocket(void *self) {
	NattyHttp *http = (NattyHttp*)self;
	if (http == NULL) {
		return -1;
	}
	return http->socketid;
}

U8 *ntyGetSendBuffer(void *self) {
	NattyHttp *http = (NattyHttp*)self;
	return http->buffer;
}

HTTP_RESULT ntyConnectServer(void *self) {
	HTTP_RESULT result;
	NattyHttp *http = ntyHttpInstance();

	result = ntyHttpCreate(http);
	if (NTY_HTTP_FAILED == result) {
		nty_printf("Http Create Failed\n");
		return NTY_HTTP_FAILED;
	}

	result = ntyHttpConnect(http);
	if (NTY_HTTP_FAILED == result) {
		nty_printf("Http Connect Failed\n");
		return NTY_HTTP_FAILED;
	}
	return NTY_HTTP_SUCCESS;
}

int ntyHttpHandleNotify(void *self, void *param) {
	const NattyHttpHandle *const * pHttpHandle = self;
	if (self && (*pHttpHandle) && (*pHttpHandle)->notify) {
		return (*pHttpHandle)->notify(self, param);
	}
	return -1;
}


