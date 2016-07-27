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
#include "NattyTimer.h"
#include "NattyUtils.h"

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


static U8 addrArray[4] = {112,93,116,188}; //no store in block
static U32 addrNum = 0xBC745D70; //{112,93,116,188};
static U16 addrPort = 8888;
sockaddr_struct serveraddr;
U32 ntyDataAccount = 0;


//extern char YxAppGetSimOperator(char simId);
//extern U32 YxAppDtcntMakeDataAcctId(char simId,char *apnName,char apnType,U8 *appId);

U8 ntyGetSocket(void *self);
U32 ntyGetAccountId(void *self);
U8 ntyGetReqType(void *self);
C_DEVID ntyGetDestDevId(void *self);





#define NTY_CRCTABLE_LENGTH			256
#define NTY_CRC_KEY		0x04c11db7ul
static U32 u32CrcTable[NTY_CRCTABLE_LENGTH] = {0};
void ntyGenCrcTable(void) {	
	U16 i,j;	
	U32 u32CrcNum = 0;	
	
	for (i = 0;i < NTY_CRCTABLE_LENGTH;i ++) {		
		u32CrcNum = (i << 24);		
		for (j = 0;j < 8;j ++) {			
			if (u32CrcNum & 0x80000000L) {				
				u32CrcNum = (u32CrcNum << 1) ^ NTY_CRC_KEY;
			} else {				
				u32CrcNum = (u32CrcNum << 1);			
			}		
		}		
		u32CrcTable[i] = u32CrcNum;	
	}
}

U32 ntyGenCrcValue(U8 *buf, int length) {	
	U32 u32CRC = 0xFFFFFFFF;		
	while (length -- > 0) {		
		u32CRC = (u32CRC << 8) ^ u32CrcTable[((u32CRC >> 24) ^ *buf++) & 0xFF];	
	}	
	return u32CRC;
}




void ntySetAddr(sockaddr_struct *addr, U32 addrNum, U16 port) {
	#if 0
	addr->addr[0] = *(U8*)(&addrNum);
	addr->addr[1] = *(((U8*)(&addrNum))+1);
	addr->addr[2] = *(((U8*)(&addrNum))+2);
	addr->addr[3] = *(((U8*)(&addrNum))+3);
	#else
	#if 0
	kal_prompt_trace(MOD_YXAPP, " addrNum : %x\n", addrNum);
	#endif
	memcpy(addr->addr, &addrNum, sizeof(U32));
	#endif

	addr->port = port;
	addr->addr_len = 0x04;
	addr->sock_type = SOC_SOCK_DGRAM;
}

static char ntyAppGetOperatorByPlmn(char *pPlmn)
{
	if(pPlmn)
	{
		if(memcmp(pPlmn,"46000",5)==0 || memcmp(pPlmn,"46002",5)==0 || memcmp(pPlmn,"46007",5)==0)
		{//china mobile
			return MSIM_OPR_CMCC;
		}
		else if(memcmp(pPlmn,"46001",5)==0 || memcmp(pPlmn,"46006",5)==0)
		{//china unicom
			return MSIM_OPR_UNICOM;
		}
		else if(memcmp(pPlmn,"46003",5)==0 || memcmp(pPlmn,"46005",5)==0)
		{//china telcom
			return MSIM_OPR_TELCOM;
		}
		else
			return MSIM_OPR_UNKOWN;
	}
	return MSIM_OPR_NONE;
}

char ntyAppGetSimOperator(char simId)
{
	char           plmn[MAX_PLMN_LEN+1]={0};
	mmi_sim_enum   mmiSim = MMI_SIM1;
	srv_nw_info_service_availability_enum servEnum;
//#if 0
	if(simId==NATTY_APP_SIM2)//sim2
		mmiSim = MMI_SIM2;
//#endif
#ifdef WIN32
	return MSIM_OPR_UNICOM;
#endif
	servEnum = srv_nw_info_get_service_availability(mmiSim);
	
	kal_prompt_trace(MOD_BT,"ntyAppGetSimOperator %d\n", servEnum);
	if(servEnum == SRV_NW_INFO_SA_FULL_SERVICE)
	{
		srv_nw_info_location_info_struct nw_info;
		if(srv_nw_info_get_location_info(mmiSim,&nw_info)) {
			kal_prompt_trace(MOD_BT,"nw_info.plmn:%s\n", nw_info.plmn);
			strncpy(plmn,nw_info.plmn,5);
		} else
			return MSIM_OPR_NONE;
		return ntyAppGetOperatorByPlmn(plmn);
	}
	else
		return MSIM_OPR_NONE;
}

U32 ntyAppDtcntMakeDataAcctId(char simId,char *apnName,char apnType,U8 *appId)
{
	U32    acc_id = 0, acct_id;
	S32    resId = ntyAppGetAccountByApn(apnType,simId,appId);
	if(resId>=0)
	{
		cbm_sim_id_enum sim_no = CBM_SIM_ID_SIM1;
		if(simId==NATTY_APP_SIM2)//sim2
			sim_no = CBM_SIM_ID_SIM2;
		
		acc_id = (U32)resId;
		acc_id = cbm_encode_data_account_id(acc_id, sim_no, appId[0], KAL_FALSE);
#if 1 //form coretek 20160617
		kal_prompt_trace(MOD_BT,"data_account111111 %d  \r\n",acc_id);
		srv_dtcnt_get_default_account(CBM_SIM_ID_SIM1+SRV_DTCNT_SIM_TYPE_1, &acct_id);
		kal_prompt_trace(MOD_BT,"acct_id111111 %d  \r\n",acct_id);
		cbm_register_app_id(appId);
		kal_prompt_trace(MOD_BT,"appid111111 %d  \r\n",appId);
		acct_id = cbm_encode_data_account_id(acc_id, CBM_SIM_ID_SIM1, *appId, MMI_FALSE); 
#endif
	}
	
	kal_prompt_trace(MOD_BT,"accountid:%d,appid:%d\r\n",acct_id,*appId);
	return acct_id;
}


S32 ntyAppAddOneApn(char simId,const U8 *apnName,const U8 *accountName,char *userName,char *password)
{
	extern srv_dtcnt_result_enum ntyapp_srv_dtcnt_get_acc_id_by_apn(S8 *apn,U32 *acc_id_out, S8 SimId);
	U32  accountid = 0;
	srv_dtcnt_result_enum result = SRV_DTCNT_RESULT_FAILED;
	srv_dtcnt_sim_type_enum  SimIdEnum = SRV_DTCNT_SIM_TYPE_1;
	srv_dtcnt_store_prof_data_struct prof_info;
	srv_dtcnt_prof_gprs_struct prof_gprs;
	memset(&prof_gprs, 0, sizeof(prof_gprs));
#if 1
	if(simId==NATTY_APP_SIM2)
		SimIdEnum = SRV_DTCNT_SIM_TYPE_2;
#endif
	if(SRV_DTCNT_RESULT_SUCCESS == (result = ntyapp_srv_dtcnt_get_acc_id_by_apn((S8*)apnName,&accountid,SimIdEnum)))
	{
		kal_prompt_trace(MOD_BT,"find one:%d,ldz\r\n",accountid);
		accountid = cbm_get_original_account(accountid);
	}
	if(result != SRV_DTCNT_RESULT_SUCCESS)
	{
		kal_prompt_trace(MOD_BT,"Add one:%d,apn:%s\r\n",accountid,(char*)apnName);

		prof_gprs.APN = apnName;
		prof_gprs.prof_common_header.Auth_info.AuthType = SRV_DTCNT_PROF_GPRS_AUTH_TYPE_NORMAL;
		prof_gprs.prof_common_header.sim_info = SimIdEnum;//SRV_DTCNT_SIM_TYPE_1;
		prof_gprs.prof_common_header.AccountName = accountName;
		prof_gprs.prof_common_header.acct_type = SRV_DTCNT_PROF_TYPE_USER_CONF;
	//	prof_gprs.prof_common_header.px_service = SRV_DTCNT_PROF_PX_SRV_HTTP;
		prof_gprs.prof_common_header.use_proxy = 0;
		prof_info.prof_data = &prof_gprs;
		prof_info.prof_type = SRV_DTCNT_BEARER_GPRS;
		prof_info.prof_fields = SRV_DTCNT_PROF_FIELD_ALL;
		result = srv_dtcnt_store_add_prof(&prof_info,&accountid);
		kal_prompt_trace(MOD_BT,"srv_dtcnt_store_add_prof %d\r\n", result);
		if(SRV_DTCNT_RESULT_SUCCESS == result)
		{
			kal_prompt_trace(MOD_BT,"update one:%d,ldz\r\n",accountid);
			srv_dtcnt_store_update_prof(accountid,&prof_info);
		//	accountid = cbm_encode_data_account_id(accountid, CBM_SIM_ID_SIM1, 0, 0);
		}
	}

	kal_prompt_trace(MOD_BT,"accoutid:%d,ldz\r\n",accountid);

	if(SRV_DTCNT_RESULT_SUCCESS == result)
		return (S32)accountid;
	return -1;
}


static char ntyAppNetGetAppid(char apn, U8 *app_id)
{
#if 0
	kal_int8 ret = cbm_register_app_id(app_id);
	cbm_hold_bearer(*app_id);
	if(CBM_OK != ret)
		return 0;
#else
	cbm_app_info_struct app_info;
    kal_int8 ret = 0;
	app_info.app_icon_id = 0;
	app_info.app_str_id = 0;
	if(apn==MAPN_WAP)
	{
		app_info.app_type = DTCNT_APPTYPE_MRE_WAP;
		ret = cbm_register_app_id_with_app_info(&app_info, app_id);
	}
	else if(apn==MAPN_NET)
	{
		app_info.app_type = DTCNT_APPTYPE_EMAIL;//DTCNT_APPTYPE_MRE_NET | DTCNT_APPTYPE_NO_PX;
		ret = cbm_register_app_id_with_app_info(&app_info, app_id);
	}
	else//wifi
	{
		app_info.app_type = DTCNT_APPTYPE_MRE_NET | DTCNT_APPTYPE_NO_PX;
		ret = cbm_register_app_id_with_app_info(&app_info, app_id);
	}
	cbm_hold_bearer(*app_id);
	if(0 != ret)
		return 0;
#endif
	return 1;
}


static S32 ntyAppGetAccountByApn(char apnType,char simId,U8 *netAppid)
{
	char  *apnName = "cmnet";
	U8    *accountName = (U8*)L"China Mobile Net";
	S32   res = 0;
	char  checkSim = ntyAppGetSimOperator(simId);
	if(apnType == MAPN_NET)
	{
		if(MSIM_OPR_UNICOM==checkSim)
		{
			accountName = (U8*)L"Unicom Net";
			apnName = "uninet";
		}
	}
	else if(apnType == MAPN_WAP)
	{
		if(MSIM_OPR_UNICOM==checkSim)
		{
			apnName = "uniwap";
			accountName = (U8*)L"Unicom WAPNet";
		}
		else
		{
			apnName = "cmwap";
			accountName = (U8*)L"China Mobile WAPNet";
		}
	}
	res = ntyAppAddOneApn(simId,(const U8 *)apnName,(const U8 *)accountName,NULL,NULL);
	if(res != -1)
	{
		if((netAppid)&&(netAppid[0]==CBM_INVALID_APP_ID))
			ntyAppNetGetAppid(apnType,netAppid);
	}
	return res;
}


static int ntyUdpCreate(void* self)
{
    U8 ret;
    U8 sock;
    U8 val;
	U8 appid;
	kal_uint32 acct_id= 0;
    //sockaddr_struct addr;

	Network *network = self;
#if 1
	U8     apn_check = (U8)ntyAppGetSimOperator(NATTY_APP_SIM1);

    if((apn_check==MSIM_OPR_UNKOWN) || (apn_check==MSIM_OPR_NONE))
        return -1;
    //apn_check = (apn != MAPN_WAP && apn != MAPN_NET && apn != MAPN_WIFI);
	//if(apn_check)
	//	return 0;
	//yxNetContext_ptr.apn = apn;
    //yxNetContext_ptr.port = port;
#if 1
    kal_prompt_trace(MOD_BT, " YxAppDtcntMakeDataAcctId : %d\n", apn_check);
#endif
	ntyDataAccount = ntyAppDtcntMakeDataAcctId(NATTY_APP_SIM1,NULL, MAPN_NET,&appid);
	if(ntyDataAccount==0)
	{
		//yxNetContext_ptr.account_id = 0;
		//YxAppCloseAccount();
		kal_prompt_trace(MOD_BT, " YxAppCloseAccount : %d\n", ntyDataAccount);
		return 0;
	}
#else
	
	ntyDataAccount = cbm_encode_data_account_id(CBM_DEFAULT_ACCT_ID, CBM_SIM_ID_SIM1, 0, MMI_FALSE);
	kal_prompt_trace(MOD_BT,"data_account111111 %d  \r\n",ntyDataAccount);
	srv_dtcnt_get_default_account(CBM_SIM_ID_SIM1+SRV_DTCNT_SIM_TYPE_1, &acct_id);
	//srv_dtcnt_get_default_account(SRV_DTCNT_SIM_TYPE_1, &acct_id);
	kal_prompt_trace(MOD_BT,"acct_id111111 %d  \r\n",acct_id);
      cbm_register_app_id(&appid);
	kal_prompt_trace(MOD_BT,"appid111111 %d  \r\n",appid);
    acct_id = cbm_encode_data_account_id(ntyDataAccount, CBM_SIM_ID_SIM1, appid, MMI_FALSE);    

#endif
	kal_prompt_trace(MOD_BT, " aaa appid : %d\n", acct_id);
	network->accountId = ntyDataAccount;
	//ntyDataAccount = acct_id;
	//yxNetContext_ptr.account_id = account_id;

    //create udp socket
    network->sockfd = soc_create(SOC_PF_INET, SOC_SOCK_DGRAM, 0, MOD_MMI, network->accountId);
    if (sock < 0) {
        return -2;
    }
    
    val = SOC_READ | SOC_WRITE | SOC_CONNECT |SOC_CLOSE;
    ret = soc_setsockopt(network->sockfd, SOC_ASYNC, &val, sizeof(val));
    if (ret != SOC_SUCCESS) {
        soc_close(network->sockfd);
        kal_prompt_trace(MOD_BT, "set SOC_ASYNC failed");
        return -3;
    }

    val = KAL_TRUE;
    ret = soc_setsockopt(network->sockfd, SOC_NBIO, &val, sizeof(val));
    if (ret != SOC_SUCCESS) {
        soc_close(sock);
        kal_prompt_trace(MOD_BT, "set SOC_NBIO failed");
        return -4;
    }
  
    return network->sockfd;
}

static U8 u8OnAckCounter = 0;

static void ntyMessageOnAck(void) {
#if 0
	void* pTimer = ntyNetworkTimerInstance();
	Network *network = ntyNetworkInstance();

	kal_prompt_trace(MOD_BT,"ntyMessageOnAck");
	if (++u8OnAckCounter > 3) {
		u8OnAckCounter = 0;
		
		ntyStopTimer(pTimer);
		if (network && network->onAck)
			network->onAck();
	} else {
		ntyStartTimer(pTimer, ntyMessageOnAck);	
		ntyNetworkResendFrame(network);
	}
#else
	kal_prompt_trace(MOD_BT,"ntyMessageOnAck");
#endif
}



static void* ntyNetworkCtor(void *_self) {
	Network *network = _self;
	network->onAck = ntyMessageOnAck;
	network->ackNum = 1;

	network->socketMutex = kal_create_mutex("Socket Mutex");
	
	return network;
}

static void* ntyNetworkDtor(void *_self) {
	return _self;
}


static kal_bool ntyNetworkMMISocketNotify(void* inMsg) {
	kal_int8 ret_val;
	app_soc_notify_ind_struct *soc_notify = (app_soc_notify_ind_struct *) inMsg;

	if(soc_notify == NULL)
	{
		kal_prompt_trace(MOD_BT,"soc_notify == NULL \r\n");
		return KAL_FALSE;
	}

	kal_prompt_trace(MOD_BT, " event_type : %d\n", soc_notify->event_type);

	return KAL_TRUE;
}



static U32 ntyNetworkResendFrame(void *_self) {
	U32 ret;
	Network *network = _self;

	
	if (network->busy) {
		return -15;
	}
	kal_take_mutex(network->socketMutex);
	network->busy = 1;
	kal_give_mutex(network->socketMutex);
	
	ret = soc_sendto(network->sockfd, network->buffer, (kal_int32)network->length, 0, &network->addr);

	kal_take_mutex(network->socketMutex);
	network->busy = 0;
	kal_give_mutex(network->socketMutex);
	//kwp_debug_print("ntp send len %d", ret);
    if (0 > ret)
    {
        if (SOC_WOULDBLOCK == ret)
        {
            kal_prompt_trace(MOD_BT,"SOC_WOULDBLOCK");
			//SetProtocolEventHandler(network->onRecv, MSG_ID_APP_SOC_NOTIFY_IND);
        }
        else
        {
            kal_prompt_trace(MOD_BT,"send data failed");
        }
        
    }
	return ret;
}


static int ntyNetworkSendFrame(void *_self, sockaddr_struct *to, U8 *buf, int len) {
	//ntyStartTimer();
	U32 ret;
	U32 crc = 0;
	Network *network = _self;	
	void* pTimer = ntyNetworkTimerInstance();
	if (buf[NTY_PROTO_MESSAGE_TYPE] != MSG_ACK) {
#if 1
		ntyStartTimer(pTimer, network->onAck);	
#else
		ntyStartTimer(pTimer, ntyMessageOnAck);	
#endif
		//StartTimer(NATTY_NETWORK_COMFIRMED_TIMER, RESEND_TIMEOUT, network->onAck);
		network->ackNum ++;
	}

	
	memcpy(&network->addr, to, sizeof(sockaddr_struct));
	memset(network->buffer, 0, CACHE_BUFFER_SIZE);
	memcpy(network->buffer, buf, len);
	
	if (buf[NTY_PROTO_MESSAGE_TYPE] == MSG_REQ) {
		ntyU32Datacpy(&network->buffer[NTY_PROTO_ACKNUM_IDX], network->ackNum);
	} else if (buf[NTY_PROTO_MESSAGE_TYPE] == MSG_ACK){
		U32 msgAck = ntyU8ArrayToU32(buf+NTY_PROTO_MESSAGE_TYPE);
		kal_prompt_trace(MOD_BT, "msgAck %d", msgAck);
	}
	network->length = len;
	crc = ntyGenCrcValue(network->buffer, len-sizeof(U32));
#if 0
	kal_prompt_trace(MOD_BT, " id:%x %x %x %x %x %x %x %x\n", network->buffer[NTY_PROTO_DEVID_IDX],
		network->buffer[NTY_PROTO_DEVID_IDX+1], network->buffer[NTY_PROTO_DEVID_IDX+2], network->buffer[NTY_PROTO_DEVID_IDX+3],
		network->buffer[NTY_PROTO_DEVID_IDX+4], network->buffer[NTY_PROTO_DEVID_IDX+5], network->buffer[NTY_PROTO_DEVID_IDX+6],
		network->buffer[NTY_PROTO_DEVID_IDX+7]);
#endif
	ntyU32Datacpy(&network->buffer[len-sizeof(U32)], crc);
	network->buffer[network->length] = '\0';
	//printf("ntyNetworkSendFrame : %x\n", buf[NTY_PROTO_TYPE_IDX]);
	//kal_prompt_trace(MOD_BT, "ntyU32Datacpy sock:%d, length:%d", network->sockfd, network->length);
	if (network->busy) {
		return -15;
	}
	kal_take_mutex(network->socketMutex);
	network->busy = 1;
	kal_give_mutex(network->socketMutex);
	
	ret = soc_sendto(network->sockfd, network->buffer, (kal_int32)network->length, 0, &network->addr);

	kal_take_mutex(network->socketMutex);
	network->busy = 0;
	kal_give_mutex(network->socketMutex);
#if 1
	kal_prompt_trace(MOD_BT,"sendto : %d.%d.%d.%d:%d size:%d --> %x\n", network->addr.addr[0], network->addr.addr[1],	
								network->addr.addr[2], network->addr.addr[3],  network->addr.port, ret, buf[NTY_PROTO_TYPE_IDX]);
	
#endif
	//kwp_debug_print("ntp send len %d", ret);
    if (0 > ret)
    {
        if (SOC_WOULDBLOCK == ret)
        {
        	//SetProtocolEventHandler(network->onRecv, MSG_ID_APP_SOC_NOTIFY_IND);
            kal_prompt_trace(MOD_BT, "SOC_WOULDBLOCK");
        }
        else
        {
            kal_prompt_trace(MOD_BT, "send data failed");
        }
        
    }
	return ret;
}

static int ntyNetworkRecvFrame(void *_self, U8 *buf, int len, sockaddr_struct *from) {
	//ntyStartTimer();
	int ret;
	int n = 0;
	int clientLen = sizeof(sockaddr_struct);
	
	sockaddr_struct addr = {0};
	U32 ackNum;
	void* pTimer = ntyNetworkTimerInstance();

	Network *network = _self;

	if (network->busy) {
		return -15;
	}
	kal_take_mutex(network->socketMutex);
	network->busy = 1;
	kal_give_mutex(network->socketMutex);
	
	ret = soc_recvfrom(network->sockfd, buf, CACHE_BUFFER_SIZE, 0, &addr);

	kal_take_mutex(network->socketMutex);
	network->busy = 0;
	kal_give_mutex(network->socketMutex);
	//ackNum = *(U32*)(&buf[NTY_PROTO_ACKNUM_IDX]);
	ackNum = ntyU8ArrayToU32(buf+NTY_PROTO_ACKNUM_IDX);

	memcpy(from, &addr, clientLen);
	if (buf[NTY_PROTO_MESSAGE_TYPE] == MSG_ACK) { //recv success		
		if (ackNum == network->ackNum + 1) {
			kal_prompt_trace(MOD_BT, "ackNum check right");
		} else {
			kal_prompt_trace(MOD_BT, "ackNum check Error");
		}
	} else if (buf[NTY_PROTO_MESSAGE_TYPE] == MSG_RET) {
		
		//void* pTimer = ntyNetworkTimerInstance();
		//ntyStopTimer(pTimer);
		//have send object
	} else if (buf[NTY_PROTO_MESSAGE_TYPE] == MSG_UPDATE) {
		//void* pTimer = ntyNetworkTimerInstance();
		//ntyStopTimer(pTimer);
	}
	ntyStopTimer(pTimer);

	return ret;
	
}

static int ntyNetworkConnect(void *_self) {
	Network *network = _self;	

	mmi_frm_set_protocol_event_handler(MSG_ID_APP_SOC_NOTIFY_IND, network->onRecv, MMI_TRUE);
	//SetProtocolEventHandler(network->onRecv, MSG_ID_APP_SOC_NOTIFY_IND);
#if 1 //Socket Init	
	network->sockfd = ntyUdpCreate(_self);
	if (network->sockfd < 0) {
		//error(" ERROR opening socket");
		kal_prompt_trace(MOD_BT, "ERROR opening socket");
		return -1;
	}
	
	//init Server addr
	kal_prompt_trace(MOD_BT, " opening socket %d, %x\n", network->sockfd, *((U32*)addrArray));
	ntySetAddr(&serveraddr, addrNum, addrPort);

	return network->sockfd;
#endif
}



static const NetworkOpera ntyNetworkOpera = {
	sizeof(Network),
	ntyNetworkCtor,
	ntyNetworkDtor,
	ntyNetworkSendFrame,
	ntyNetworkRecvFrame,
	ntyNetworkResendFrame,
	ntyNetworkConnect,
};

const void *pNtyNetworkOpera = &ntyNetworkOpera;

static void *pNetworkOpera = NULL;

void *ntyNetworkInstance(void) {
	if (pNetworkOpera == NULL) {
		pNetworkOpera = New(pNtyNetworkOpera);
	}
	return pNetworkOpera;
}

void ntyNetworkRelease(void *self) {	
	Delete(self);
}


int ntySendFrame(void *self, sockaddr_struct *to, U8 *buf, int len) {
	const NetworkOpera *const * pNetworkOpera = self;

	//kal_prompt_trace(MOD_YXAPP, " SendFrame:%x, addr:%x\n", buf[NTY_PROTO_TYPE_IDX], buf);
	if (self && (*pNetworkOpera) && (*pNetworkOpera)->send) {
		return (*pNetworkOpera)->send(self, to, buf, len);
	}
	return -1;
}

int ntyRecvFrame(void *self, U8 *buf, int len, sockaddr_struct *from) {
	const NetworkOpera *const * pNetworkOpera = self;

	if (self && (*pNetworkOpera) && (*pNetworkOpera)->recv) {
		return (*pNetworkOpera)->recv(self, buf, len, from);
	}
	return -2;
}

int ntyConnect(void *self) {
	const NetworkOpera *const * pNetworkOpera = self;

	if (self && (*pNetworkOpera) && (*pNetworkOpera)->recv) {
		return (*pNetworkOpera)->connect(self);
	}
	return -3;
}

U8 ntyGetSocket(void *self) {
	Network *network = self;
	return network->sockfd;
}

U8 ntyGetReqType(void *self) {
	Network *network = self;
	return network->buffer[NTY_PROTO_TYPE_IDX];
}

C_DEVID ntyGetDestDevId(void *self) {
	Network *network = self;
	return *(C_DEVID*)(&network->buffer[NTY_PROTO_DEST_DEVID_IDX]);
}

U32 ntyGetAccountId(void *self) {
	Network *network = self;
	return *(C_DEVID*)(&network->accountId);
}

void ntySetRecvProc(void *self, HANDLE_RECV func) {
	Network *network = self;
	network->onRecv = func;
}

void ntySetOnAckProc(void *self, HANDLE_TIMER func) {
	Network *network = self;
	network->onAck = func;
}

U8* ntyGetBuffer(void *self) {
	Network *network = self;
	return network->buffer;
}

