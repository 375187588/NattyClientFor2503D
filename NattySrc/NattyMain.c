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

#include "DataAccountStruct.h"
#include "MMIDataType.h"
#include "CbmSrvGprot.h"
#include "cbm_api.h"
#include "TimerEvents.h"

#include "NattyMain.h"
#include "NattyClientDev.h"
#include "NattyUtils.h"
#include "NattyHttp.h"
#include "NattyLocation.h"


extern U8 ntyDevId[16];
extern U8 ntyAppId;
extern U32 ntyDataAccount;


static NW_APN_Detect_status_e g_eApnSetOk = NETWORK_DETECT_APN_NONE;
static int g_iDeActivedCount = 0;
static int g_iSetApnTryNum      = 0;/* Because when system intialized, the APN has been set one time.*/
static U8 flg_nty_init = FALSE;

extern int mmi_dtcnt_sim_prof_add(int iTryNum);
extern void mmi_dtcnt_sim_prof_force_delete(void);

int ntyDectAPN(void) {
	cbm_open_bearer(ntyDataAccount);
    return g_eApnSetOk;
}

void ntySetDevId(void) {
	int i, j;
	U8 imei[8] = {0};
	U8 flgIMeiError = FALSE;

	nvram_get_imei_value(8, imei, 1);
	memset(ntyDevId, 0, sizeof(ntyDevId));

	for (i = 0, j = 0;i < 8;i ++) {
		kal_prompt_trace(MOD_BT, "imei:%x \r\n", imei[i]);
		ntyDevId[j++] = ntyInt2Char(imei[i] & 0x0f);
		ntyDevId[j++] = ntyInt2Char((imei[i] >> 4) & 0x0f);
	}

	ntyDevId[j-1] = 0x0;

	#ifdef WIN32
    memset(ntyDevId, 0, sizeof(ntyDevId));
    #ifdef NETWORK_SH
    sprintf(ntyDevId, "354413056396945");
    #else
    sprintf(ntyDevId, "rsm1");
    #endif
    #endif
}


static mmi_ret ntyActivateNetworkCallback(mmi_event_struct *param) {
	srv_cbm_bearer_info_struct *cbm_evt = (srv_cbm_bearer_info_struct*)param;
	if (cbm_evt->state == SRV_CBM_ACTIVATED)
	{
        kal_prompt_trace(MOD_BT, "SRV_CBM_ACTIVATED !!!!!!!!!");
        //g_eApnSetOk = NETWORK_DETECT_APN_OK;
        if (flg_nty_init == FALSE)
        {
            flg_nty_init = TRUE;
            //nty init network
            ntyClientDevInit();
        }
		return MMI_RET_OK;
	}
	kal_prompt_trace(MOD_BT, "ntyActivateNetworkCallback %d", cbm_evt->state);
	if (cbm_evt->state == SRV_CBM_DEACTIVATED) 
	{
		kal_prompt_trace(MOD_BT, "SRV_CBM_DEACTIVATED:%d !!!!!!!!!",g_iDeActivedCount);
		g_iDeActivedCount++;

        if (flg_nty_init == TRUE)
        {
            flg_nty_init = FALSE;
			#if 0 //release
            kwp_deinit();
			#else
			ntyClientDevDeinit();
			#endif
        }
        if(g_iDeActivedCount > 10 &&
			(g_eApnSetOk != NETWORK_DETECT_APN_OK && g_eApnSetOk != NETWORK_DETECT_APN_FAIL))
		{
			g_iSetApnTryNum++;
			mmi_dtcnt_sim_prof_force_delete();
			if(mmi_dtcnt_sim_prof_add(g_iSetApnTryNum) == -1)
			{
				g_eApnSetOk = NETWORK_DETECT_APN_FAIL;
			}
			g_iDeActivedCount = 0;
		}

		return MMI_RET_OK;
	}

	return MMI_RET_OK;
}


void ntySendMsgToMMIMod(U8 command,U8 *param,U8 length)
{
	int i = 0;
	ilm_struct *ilm_ptr = NULL;//__GEMINI__
	U8 *p = (U8*)construct_local_para(length+2,TD_CTRL);
	memset(p, 0, length+2);
	
	p[0] = command;
#if 0
	for (i = 0;i < length;i ++) {
		kal_prompt_trace(MOD_BT, "cmd:%x, cmd[%d]: %x\r\n", command, i, param[i] );
	}
#endif
	memcpy(p+1, param, length);
	p[length+1] = '\0';
	
	ilm_ptr = allocate_ilm(MOD_MMI);//MOD_L4C
	ilm_ptr->msg_id = MSG_ID_MMI_JAVA_UI_TEXTFIELD_HIDE_REQ;
	ilm_ptr->local_para_ptr = (local_para_struct*)p;
	ilm_ptr->peer_buff_ptr = NULL;

	//kal_prompt_trace(MOD_BT, "ntySendMsgToMMIMod 111111111\r\n");

	ilm_ptr->src_mod_id  = MOD_MMI;//MOD_L4C;
	ilm_ptr->dest_mod_id = MOD_MMI;
	ilm_ptr->sap_id = MOD_MMI;//MMI_L4C_SAP;
	msg_send_ext_queue(ilm_ptr);
}

static void ntyMMIProcMsgHdler(void *msg) {
	U8    *command = (U8*)msg;
	U8 data[PROXY_DATA_BUFFER_SIZE] = {0};

	switch(command[0]) {
		case NTY_PROTO_PROXY_DATA_SENDFRAME: {		
			int i = 0;
			C_DEVID friId = ntyU8ArrayToU64(command+1);//*(C_DEVID*)(command+1);
#if 0
			for (i = 0;i < sizeof(C_DEVID);i ++) {
				kal_prompt_trace(MOD_BT, "1111 cmd[%d]: %x", i+1, command[i+1]);
			}
#endif
			strcpy(data, command+1+sizeof(C_DEVID));

			//kal_prompt_trace(MOD_BT, " req friId:%x, data:%s \r\n", friId, data);
			
			ntyProcessUserDataPacket(data, friId);
			break;
		}
		case NTY_PROTO_PROXY_ACK: {
			//int i = 0;
			C_DEVID friId = ntyU8ArrayToU64(command+1);//*(C_DEVID*)(command+1);
			U32 ack = ntyU8ArrayToU32(command+1+sizeof(C_DEVID));
#if 0
			for (i = 0;i < sizeof(C_DEVID);i ++) {
				kal_prompt_trace(MOD_BT, "2222 cmd[%d]: %x", i+1, command[i+1]);
			}
#endif				
			//kal_prompt_trace(MOD_BT, " ack ack:%d \r\n", ack);

			sendProxyDataPacketAck(friId, ack);
			break;
		}
		
		case NTY_PROTO_OPERA_GET_POWER_VALUE: {
			U8 u8Power = ntyGetBatteryLevel();
			C_DEVID friId = ntyU8ArrayToU64(command+1);

			sprintf(data, "Set Power %d", u8Power);
			kal_prompt_trace(MOD_BT, " data:%s \r\n", data);
			
			sendProxyUserDataPacket(friId, data, strlen(data));
			kal_prompt_trace(MOD_BT, " GET_POWER_VALUE complete\r\n");
			break;
		}
		case NTY_PROTO_OPERA_GET_SIGNAL_VALUE: {
			U8 u8Signal = ntyGetSignalLevel();
			C_DEVID friId = ntyU8ArrayToU64(command+1);

			sprintf(data, "Set Signal %d", u8Signal);
			kal_prompt_trace(MOD_BT, " data:%s \r\n", data);
			
			sendProxyUserDataPacket(friId, data, strlen(data));
			kal_prompt_trace(MOD_BT, " GET_SIGNAL_VALUE complete\r\n");
			break;
		}
		case NTY_MSG_CALL_CTRL: {
			kal_prompt_trace(MOD_BT, " com:%s\r\n", command+1);
			ntyLaunchCall(command+1);
			
			break;
		}
	}
}


void ntyMMiRegisterMsg(void) {
	SetProtocolEventHandler(ntyMMIProcMsgHdler, MSG_ID_MMI_JAVA_UI_TEXTFIELD_HIDE_REQ);
}

extern void ntyGpsSyncEnable(void);

void ntyProtocolInit(void) {
	cbm_app_info_struct app_info;
	srv_cbm_result_error_enum ret;
	kal_uint8 appid = 14;
	
	kal_prompt_trace(MOD_BT, "ntyProtocolInit \r\n");
	ntySetDevId();
#if 0
	app_info.app_icon_id = 0;
	app_info.app_str_id = 4804; //STR_DI_DLS_NETWORK_PROBLEM
	app_info.app_type = DTCNT_APPTYPE_NONE;
	cbm_register_app_id_with_app_info(&app_info, &ntyAppId);
	kal_prompt_trace(MOD_BT,"update ntyDevId:%d \r\n", ntyAppId);

	ret = srv_cbm_register_bearer_info(ntyAppId, CBM_DEACTIVATED | CBM_ACTIVATED, CBM_PS, ntyActivateNetworkCallback, NULL);
	kal_prompt_trace(MOD_BT,"update srv_cbm_register_bearer_info:%d \r\n", ret);
	ntyDataAccount = cbm_encode_data_account_id(CBM_DEFAULT_ACCT_ID, (cbm_sim_id_enum)CBM_SIM_ID_SIM1, ntyAppId, KAL_FALSE);
#elif 0
	app_info.app_icon_id = 0;
	app_info.app_str_id = 4804; //STR_DI_DLS_NETWORK_PROBLEM
	app_info.app_type = DTCNT_APPTYPE_NONE;
	cbm_register_app_id_with_app_info(&app_info, &ntyAppId);

	srv_dtcnt_get_acc_id_by_apn("cmnet", &ntyDataAccount);
	cbm_register_app_id(&appid);

	cbm_hold_bearer(appid);
	ntyDataAccount = cbm_set_app_id(ntyDataAccount, appid);
	cbm_deregister_app_id(appid);

	ntyClientDevInit();
#else
	ntyMMiRegisterMsg();
	
	ntyWatchConfigInit();
	//ntySetDefaultSystemTime();

	//ntyGpsSyncEnable();
	//mmiKeypadInit();
	ntyClientDevInit();
#endif
	kal_prompt_trace(MOD_BT,"update ntyDataAccount:%d \r\n", ntyDataAccount);
}


extern void card_gps_cell_info();

extern void led_test_SampleData(void);
extern void led_display_time(void);
extern void led_display(void);

extern void mmiKeypadInit(void);

extern kal_bool pah8002_init(void);
extern void pah8002_task(void);

extern int ds3553_step_count(void);
extern void ds3553_sensor_init(void);
extern kal_bool TMDA_init(void);
extern kal_uint8 ds3553_chip_init(void);
extern void ds3553_i2c_init(void);
extern void ds3553_close(void);
extern void ds3553_open(void);
extern void test_eint(void);
extern void ntyWifiStartTimer(void);
extern void ntyGPSStartScan(void);
extern void ntyLBSStartScan(void);

//int steps = 0;


void led_test_back(void)
{
	StopTimer(LED_TEST_TIMER);
	led_display();
	StartTimer(LED_TEST_TIMER,100, led_test_back);
}

void gps_timer_task_scan(void) {
	NattyHttp *http = ntyHttpInstance();
	Location *loc = ntyLocationInstance();
	
	StopTimer(NATTY_GPSSCAN_TIMER);
#if 0 //http test
	ntyConnectServer(http);
#endif
	nty_printf("Start GPS Location ...");
#if 0
	ntyStartGPS(loc);
#else
	ntyStartWlanScan(loc);
#endif
	StartTimer(NATTY_GPSSCAN_TIMER, 300 * 1000, gps_timer_task_scan);
}


void NattyEntry(void) {
	
#if 1
	srv_gpio_setting_set_bl_time(6);
#endif
	
#if 1 //Natty Network Communication
	ntyProtocolInit();
#endif
		
#if 1 //Panel LED Display	
	sn3770_init();
	StartTimer(LED_TEST_TIMER, 100, led_test_back);
#endif
	
#if 1 //Keypad Init
	mmiKeypadInit();
#endif
	
	
#if 1 //G-Sensor
	ds3553_sensor_init();
#endif
	
#if 1 //PAH8802
	pah8002_init();
#endif

#if 0 //GPS Scan
	//StartTimer(NATTY_GPSSCAN_TIMER, 120 *1000, ntyGPSStartScan);
#else
	StartTimer(NATTY_GPSSCAN_TIMER, 150 * 1000, gps_timer_task_scan);
#endif
	
}


