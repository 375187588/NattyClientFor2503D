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


#include "NattyLocation.h"

#include "TimerEvents.h"
#include "mdi_gps.h"


void ntyLocationEncodeWifiData(void) {
	NattyHttp *http = ntyHttpInstance();
	U8 *buffer = ntyGetSendBuffer(http);
	U8 CRLF[] = {0x0d,0x0a,0x00};
	int i = 0, j = 0;

	memset(buffer, 0, HTTP_BUFFER_SIZE);
	strcat(buffer, "GET ");
	strcat(buffer, GAODE_WIFI_LABS_LOCATION_URL);
	
	strcat(buffer, "&macs=");
	
	for (i = 0;i < wifi_snd_ctr;i ++) {
		U8 ch[5] = {0};
		for (j = 0;j < 5;j ++) {
			ntyHexToString(wifi_snd_list[i].mac[j], ch);
			strcat(buffer, ch);
			strcat(buffer, ":");
		}
		ntyHexToString(wifi_snd_list[i].mac[j], ch);
		strcat(buffer, ch);
		strcat(buffer, ",");
		
		sprintf(ch, "%d", wifi_snd_list[i].rssi);
		strcat(buffer, ch);
		strcat(buffer, ",");
		strcat(buffer, wifi_snd_list[i].ssid);
		if (i != wifi_snd_ctr) {
			strcat(buffer, "|");
		}
	}

	strcat(buffer, "&output=xml");
	strcat(buffer, "&key=");
	strcat(buffer, GAODE_DEVELOP_KEY);

	strcat(buffer," HTTP/1.1");
	strcat(buffer,CRLF);
	strcat(buffer,"Host: apilocate.amap.com");
	
	strcat(buffer,CRLF);
	strcat(buffer,"Cache-Control: no-cache");
	
	strcat(buffer,CRLF);
	strcat(buffer,"Pragma: no-cache");
	strcat(buffer,CRLF);
	strcat(buffer,CRLF);
	
	http->sendLength = strlen(buffer);

}

void ntyLocationWlanEnd(void *data) {
	Location *loc = ntyLocationInstance();
	nty_printf("ntyLocationWlanEnd");

	switch (g_bWifiInfoStatus) {
		case WLAN_INFO_STATUS_NO: 
		case WLAN_INFO_STATUS_1: {
			//setup gps location
			ntyStartGPS(loc);
			break;
		}
		case WLAN_INFO_STATUS_2: 
		case WLAN_INFO_STATUS_3: 
		case WLAN_INFO_STATUS_4: {
			//setup same data location, do no thing
			break;
		}
		case WLAN_INFO_STATUS_5: {
			NattyHttp *http = ntyHttpInstance();
			//setup wifi location
#if 1
			memset(wifi_snd_list, 0, WIFI_SCAN_NUMBER*sizeof(WlanInfo_t));
			memcpy(wifi_snd_list, wifi_list, wifi_ctr*sizeof(WlanInfo_t));
			wifi_snd_ctr = wifi_ctr;
#endif
			ntyLocationEncodeWifiData();
			ntyConnectServer(http);

			break;
		}
	}
}


void ntyLocationWlanStart(void *data) {
	Location *loc = data;
	//loc->u8LocType = LOCATION_CELL;
	loc->u8CurLocLevel = LOCATION_CELL;
	
	ntySetWifiEndCallback(ntyLocationWlanEnd);
	ntyTimerScanWifi();
#if 0 //Wifi Location Scan
	StartTimer(NATTY_WIFI_SCAN_TIMER, 50 * 1000, ntyWifiStartTimer);
#endif
	
	//StartTimer(NATTY_GPSSCAN_TIMER, 120*1000, ntyGPSStartScan);
}


void ntyLocationGPSEnd(void *data) {
	
}

void ntyLocationGPSStart(void *self) {
	Location *loc = self;
	if (loc->u8LocType == LOCATION_NULL && loc->u8CurLocLevel == LOCATION_FREE
		&& !ntyArrayIsEmpty(loc->u8Lat) && !ntyArrayIsEmpty(loc->u8Lon)) { //first gps
		loc->u8CurLocLevel = LOCATION_EPO;
		ntyLocationCellStart(self);

		nty_printf(" ntyLocationGPSStart 1111");
	}/* else if (ntyGetUtcSec() - loc->u32EpoTime >= 2 * 60 * 60 && loc->u32EpoTime != 0) { //epo timeout
		loc->u8CurLocLevel = LOCATION_EPO;
		ntyLocationCellStart(self);

		nty_printf(" ntyLocationGPSStart 2222");
	} */else {
		//ntySetAltitudeCallback(ntySetAltitude);
		loc->u8CurLocLevel = LOCATION_GPS;
		
		ntySetNMEAGGACallback(ntyParseNMEAGGA);
		ntySetNMEARMCCallback(ntyParseNMEARMC);
		ntySetPMTK741CmdCallback(ntySetPMTK741Cmd);
		
		ntyGpsSyncEnable();

		nty_printf(" ntyLocationGPSStart 3333");
	}
}

void ntyLocationEncodeCellData(void) {
	NattyHttp *http = ntyHttpInstance();
	U8 *buffer = ntyGetSendBuffer(http);
	U8 CRLF[] = {0x0d,0x0a,0x00};	
	int i = 0, j = 0;

	memset(buffer, 0, HTTP_BUFFER_SIZE);
	strcat(buffer, "GET ");
	strcat(buffer, GAODE_WIFI_LABS_LOCATION_URL);

	strcat(buffer, "&cdma=0");
	strcat(buffer, "&bts=");
	{
		U8 cellInfo[64] = {0};
		sprintf(cellInfo, "%d,0%d,%d,%d,-%d", nty_cur_cell_info.mcc, 
			nty_cur_cell_info.mnc, nty_cur_cell_info.lac, nty_cur_cell_info.ci,
			nty_cur_cell_info.rxlev);
		strcat(buffer, cellInfo);
	}
	
	strcat(buffer, "&nearbts=");
	for (i = 0;i < nty_cell_nbr_num;i ++) {
		//nty_nbr_cell_info[i].
		U8 cellInfo[64] = {0};
		sprintf(cellInfo, "%d,0%d,%d,%d,-%d", nty_nbr_cell_info[i].mcc, 
			nty_nbr_cell_info[i].mnc, nty_nbr_cell_info[i].lac, nty_nbr_cell_info[i].ci,
			nty_nbr_cell_info[i].rxlev);
		strcat(buffer, cellInfo);
		if (i != (nty_cell_nbr_num-1)) {
			strcat(buffer, "|");
		}
	}

	strcat(buffer, "&output=xml");
	strcat(buffer, "&key=");
	strcat(buffer, GAODE_DEVELOP_KEY);

	strcat(buffer," HTTP/1.1");
	strcat(buffer,CRLF);
	strcat(buffer,"Host: apilocate.amap.com");
	
	strcat(buffer,CRLF);
	strcat(buffer,"Cache-Control: no-cache");
	
	strcat(buffer,CRLF);
	strcat(buffer,"Pragma: no-cache");
	strcat(buffer,CRLF);
	strcat(buffer,CRLF);
	
	http->sendLength = strlen(buffer);
}

// sid,nid, bid,,,singnal
void ntyLocationCellEnd(void *data) {
	NattyHttp *http = ntyHttpInstance();

	ntyCellClose();
	ntyLocationEncodeCellData();
	ntyConnectServer(http);
}

void ntyLocationCellStart(void *data) {
	Location *loc = data;
	//loc->u8LocType = LOCATION_CELL;
	loc->u8CurLocLevel = (loc->u8CurLocLevel != LOCATION_EPO ? LOCATION_CELL : LOCATION_EPO);
	
	ntySetCellEndCallback(ntyLocationCellEnd);
	ntyLBSStartScan();
}


void* ntyLocationCtor(void *_self) {
	Location *loc = (Location*)_self;

	loc->u32LocTime = 0;
	loc->u8LocType = LOCATION_NULL;
	loc->u8CurLocLevel = LOCATION_FREE;
	memset(loc->u8Lat, 0, LON_LAT_LENGTH);
	memset(loc->u8Lon, 0, LON_LAT_LENGTH);

	return loc;
}

void* ntyLocationDtor(void *_self) {
	;
}


static const LocationHandle ntyLocationHandle = {
	sizeof(Location),
	ntyLocationCtor,
	ntyLocationDtor,
	ntyLocationWlanStart,
	ntyLocationWlanEnd,
	ntyLocationGPSStart,
	ntyLocationGPSEnd,
	ntyLocationCellStart,
	ntyLocationCellEnd,
};

const void *pNtyLocationHandle = &ntyLocationHandle;
static void *pLocationHandle = NULL;

void *ntyLocationInstance(void) {
	if (pLocationHandle == NULL) {
		pLocationHandle = New(pNtyLocationHandle);
	}
	return pLocationHandle;
}

void ntyLocationRelease(void *self) {	
	Delete(self);
}

void ntyStartWlanScan(void *self) {
	const LocationHandle *const * plocHandle = self;

	if (self && (*plocHandle) && (*plocHandle)->wstart) {
		(*plocHandle)->wstart(self);
	}
}

void ntyStartGPS(void *self) {
	const LocationHandle *const * plocHandle = self;

	if (self && (*plocHandle) && (*plocHandle)->gstart) {
		nty_printf(" ntyStartGPS 11111");
		(*plocHandle)->gstart(self);
	}
	//nty_printf(" ntyStartGPS 2222");
}

void ntyStartCell(void *self) {
	const LocationHandle *const * plocHandle = self;

	if (self && (*plocHandle) && (*plocHandle)->cstart) {
		(*plocHandle)->cstart(self);
	}
}



void ntySetAltitude(void *data) {
	double altitude = *(double*)data;
	Location *loc = ntyLocationInstance();
	//loc->u8Altitude, 
	sprintf(loc->u8Altitude, "%f", altitude);
	nty_printf(" altitude : %s", loc->u8Altitude);
}

void ntyParseNMEAGGA(void *data) {
	mdi_gps_nmea_gga_struct *gpgga = data;
	Location *loc = ntyLocationInstance();
	//loc->u8Altitude, 
	sprintf(loc->u8Altitude, "%f", gpgga->altitude);
	nty_printf(" altitude : %s", loc->u8Altitude);
}

void ntyParseNMEARMC(void *data) {
	mdi_gps_nmea_rmc_struct *gprmc = data;
	Location *loc = ntyLocationInstance();

	nty_printf("status:%d", gprmc->status);
	if (gprmc->status == 'A') {
		sprintf(loc->u8Lat, "%f", gprmc->latitude);
		sprintf(loc->u8Lon, "%f", gprmc->longitude);
		loc->u8LocType = LOCATION_GPS;

		ntyGpsScanStop();
	}
	nty_printf("latitude=[%s]", loc->u8Lat);
    nty_printf("longitude=[%s]", loc->u8Lon);
}

void ntySetPMTK741Cmd(void *data) {
	U8 *buffer = data;
	Location *loc = ntyLocationInstance();
	
	sprintf((char *)buffer, "$PMTK741,%s,%s,%s,", loc->u8Lat, loc->u8Lon, loc->u8Altitude);
}


