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



#include "NattyWlan.h"
#include "NattyUtils.h"
#include "NattyAbstractClass.h"

#include "TimerEvents.h"



WlanInfo_t wifi_list[WIFI_SCAN_NUMBER]={0x00};
U8 wifi_ctr=0;
U8 flg_wifi_init = FALSE;
U8 g_bWifiInfoStatus= WLAN_INFO_STATUS_NO;

WlanInfo_t wifi_snd_list[WIFI_SCAN_NUMBER]={0x00};
U8 wifi_snd_ctr=0;

//HANDLE_LOCATION ntyWifiStartCb;
HANDLE_LOCATION ntyWifiEndCb = NULL;



#define WIFI_ACCURATE_MODE_VALID_NUM            (5)
#define WIFI_NORMAL_MODE_VALID_NUM               (3)


#define WIFI_THRESHOLD_SEL_ACCURATE_VAL        (-80)
#define WIFI_THRESHOLD_SEL_NORMAL_VAL           (-90)

void ntySetWifiEndCallback(HANDLE_LOCATION cb) {
	ntyWifiEndCb = cb;
}

static void ntyWifiInitCallback (void *user_data, wlan_init_cnf_struct *cnf) 
{
	//kwp_debug_print("kwp_WIFI_init_cb state = %d", cnf->status);
	if (cnf->status == SCANONLY_SUCCESS) {
		//kwp_wlan_scan();
		nty_printf("WIFI init success");
	} else {
		nty_printf("WIFI init fail");
	}
}

U8 ntyWlanOpen(void) {
	wlan_init_req_struct init_req = {0};
	wlan_result_enum result;

	if(flg_wifi_init == 0) {
		result = wlan_init(&init_req, ntyWifiInitCallback, NULL);
		nty_printf("kwp_wlan_open result:%d", result);
		flg_wifi_init = 1;

		if (result == WLAN_RESULT_SUCCESS) {
			//kwp_debug_print("WLAN_init success");
			return 0;
		} else {
			//kwp_debug_print("WLAN_init fail");
			return 1;
		}
	}
}


static void ntyWifiDeinitCallback (void *user_data, wlan_deinit_cnf_struct *cnf) {
    return ;
}

U8 ntyWlanClose(void) {
	wlan_deinit_req_struct req = {0};
	
	if(flg_wifi_init) {
		wlan_deinit(&req, ntyWifiDeinitCallback, NULL);
		flg_wifi_init = 0;
	}
	return 0;
}

void ntyWifiScanCallback (void *user_data, wlan_scan_cnf_struct *cnf) {
	U8 i,j,k;
	signed char rssi_tmp;
	U8 index[WIFI_SCAN_NUMBER] = {0xff};
	U8 flg_get = 0;
	U8 iCount = 0;
	kal_int8       s8SelVal = 0;  

	nty_printf("kwp_wifi_scan_cb");
	if (SCANONLY_SUCCESS == cnf->status) {
		if (cnf->scan_ap_num > WIFI_SCAN_NUMBER) {
			wifi_ctr = WIFI_SCAN_NUMBER;
			rssi_tmp = cnf->scan_ap[0].rssi;
		} else {
			wifi_ctr = cnf->scan_ap_num;
		}

		for( i =0 ; i < cnf->scan_ap_num; i++) {
			scanonly_scan_ap_info_struct scanApTemp = {0x00};

			for( j = i+1; j < cnf->scan_ap_num; j++) {
				// Set the rssi of the Same MAC Item to -120,and set the rssi of no ssid item to -120.
				if( (memcmp(cnf->scan_ap[i].bssid,cnf->scan_ap[j].bssid,sizeof(cnf->scan_ap[j].bssid)) == 0) ||
					(cnf->scan_ap[j].ssid_len == 0)) {
					cnf->scan_ap[j].rssi = -120;
				} else if(cnf->scan_ap[i].rssi < cnf->scan_ap[j].rssi) {
					memcpy(&scanApTemp,&(cnf->scan_ap[i]),sizeof(scanonly_scan_ap_info_struct));
					memcpy(&(cnf->scan_ap[i]),&(cnf->scan_ap[j]),sizeof(scanonly_scan_ap_info_struct));
					memcpy(&(cnf->scan_ap[j]),&scanApTemp,sizeof(scanonly_scan_ap_info_struct));
				}
			}
		}

		iCount = 0;
#if 0
		if(kw_config.set_info.uiLocateMode ==  LOCATION_MODE_ACCURATE)
			s8SelVal = WIFI_THRESHOLD_SEL_ACCURATE_VAL;
		else
			s8SelVal = WIFI_THRESHOLD_SEL_NORMAL_VAL;
#else
		s8SelVal = WIFI_THRESHOLD_SEL_NORMAL_VAL;
#endif
		for( i = 0; i < wifi_ctr;i++) {
			for( j = i; j < cnf->scan_ap_num; j++) {
				//Take item which match rssi condition
				if(cnf->scan_ap[j].rssi > s8SelVal && cnf->scan_ap[j].rssi < WIFI_THRESHOLD_MAX_VAL) {
					iCount++;
					wifi_list[i].rssi = cnf->scan_ap[j].rssi;
					memcpy(&wifi_list[i].mac, &cnf->scan_ap[j].bssid, sizeof(wifi_list[i].mac));
					memcpy(&wifi_list[i].ssid, &cnf->scan_ap[j].ssid, sizeof(wifi_list[i].ssid));
					break;
				}
			}
			nty_printf("ssid:%s, rssi:%d", wifi_list[i].ssid, wifi_list[i].rssi);
			nty_printf("max:%x%x%x%x%x%x", wifi_list[i].mac[0],wifi_list[i].mac[1],wifi_list[i].mac[2],
				wifi_list[i].mac[3],wifi_list[i].mac[4],wifi_list[i].mac[5]);
		}

		wifi_ctr = iCount;	
	}

	if(cnf != NULL)
		nty_printf("nty scan_cb num:%d,totalNum:%d",wifi_ctr,cnf->scan_ap_num);
	else		
		nty_printf("nty scan_cb num:%d",wifi_ctr);

	//kwp_timer_stop(KWP_TIMER_SCAN_WIFI);

      //If only one wifi item, we think the condition isn't enough.
	if(wifi_ctr >= 2) {
		U8   ucSameWifiItemNum = 0;
		U16  ucBeyondLitimtDB = 0;
		U16  ucValidWifiNum = 0;
		U16  ucValidWifiThreshold= 0;
#if 0		
		kwp_timer_t*kwpTimer = kwp_get_timer_by_id(KWP_TIMER_MONITOR_WIFI);

		if(kw_config.set_info.uiLocateMode ==  LOCATION_MODE_ACCURATE)
			ucValidWifiThreshold = WIFI_ACCURATE_MODE_VALID_NUM;
		else
			ucValidWifiThreshold = WIFI_NORMAL_MODE_VALID_NUM;
#else
		ucValidWifiThreshold = WIFI_NORMAL_MODE_VALID_NUM;
#endif
		for( i = 0 ; i < wifi_ctr; i++) {
			if(wifi_list[i].rssi >= WIFI_THRESHOLD_MIN_VAL && 
				wifi_list[i].rssi < WIFI_THRESHOLD_MAX_VAL) {
				ucValidWifiNum++;	
			}
							
			for( j = 0; j < wifi_snd_ctr;j++) {
				if( (memcmp(wifi_list[i].mac,wifi_snd_list[j].mac,sizeof(wifi_list[i].mac)) == 0)
					&& (wifi_list[i].rssi >= WIFI_THRESHOLD_MIN_VAL 
					&& wifi_list[i].rssi < WIFI_THRESHOLD_MAX_VAL)) {
					ucSameWifiItemNum++;
					if( wifi_list[i].rssi>  wifi_snd_list[j].rssi)
						ucBeyondLitimtDB +=(wifi_list[i].rssi- wifi_snd_list[j].rssi);
					else
						ucBeyondLitimtDB +=(wifi_snd_list[j].rssi - wifi_list[i].rssi);

				}
			}
		}
		//For test
		//ucSameWifiItemNum = 2;
		
		nty_printf("KWP_WIFI_scan_cb SameNum:%d,db:%d,validNum:%d",ucSameWifiItemNum,ucBeyondLitimtDB,ucValidWifiNum);
		if(ucSameWifiItemNum > 4) {
			//Todo: Think not change Position 
			g_bWifiInfoStatus = WLAN_INFO_STATUS_4;
		} else if(ucSameWifiItemNum >= 3 && ucBeyondLitimtDB < 6*ucSameWifiItemNum) {
			//Todo: Think not change Position 
			g_bWifiInfoStatus = WLAN_INFO_STATUS_3;
		} else if(ucSameWifiItemNum >= 3) {
			g_bWifiInfoStatus = WLAN_INFO_STATUS_2;			
		} else if(ucSameWifiItemNum > 1) {
			if(ucValidWifiNum >= ucValidWifiThreshold)
				g_bWifiInfoStatus = WLAN_INFO_STATUS_5;
			else	
				g_bWifiInfoStatus = WLAN_INFO_STATUS_1;
			
		} else {
			if(ucValidWifiNum >= ucValidWifiThreshold)
				g_bWifiInfoStatus = WLAN_INFO_STATUS_5;
			else	
				g_bWifiInfoStatus = WLAN_INFO_STATUS_NO;
		}
		//if(kwpTimer != NULL && kwpTimer->data.handle_enable == FALSE)
	} else {
		g_bWifiInfoStatus = WLAN_INFO_STATUS_NO;
	}

	ntyWlanClose();	
	nty_printf("NTY_WIFI_scan End!! : %d", g_bWifiInfoStatus);
	if (ntyWifiEndCb != NULL) {
		nty_printf("ntyWifiEndCb");
		(*ntyWifiEndCb)(NULL);
	}
}

U8 ntyWlanScan(void) {
	wlan_result_enum ret;
	wlan_scan_req_struct req = {0};
	
	ret = wlan_scan(&req, ntyWifiScanCallback, NULL);
	//kwp_debug_print("WLAN scan result = %d", ret);

	if (ret == WLAN_RESULT_SUCCESS) {
		return 1;
	}
	else {
		return 0;
	}
}

void ntyTimerScanWifi(void) {
	nty_printf("ntyTimerScanWifi");
	if (!flg_wifi_init) {
		ntyWlanOpen();
	}
	ntyWlanScan();
}

void ntyWifiStartTimer(void) {
	StopTimer(NATTY_WIFI_SCAN_TIMER);
	ntyTimerScanWifi();
	StartTimer(NATTY_WIFI_SCAN_TIMER, 50 * 1000, ntyWifiStartTimer);
}

