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



#include "NattyGPS.h"
#include "NattyWlan.h"

#include "mdi_gps.h"
#include "TimerEvents.h"


static GPSINFO_t_EX gps_info_ex = {0};

static unsigned char g_gps_fixed_flag = 0;
static unsigned char mmi_fm_gps_open_flag = 0;
static S16 local_gps_port = 0;
static unsigned char g_gpsSyncEnableFlag = 0;
static unsigned int g_gpsSyncCount = 0;
static unsigned int g_gpsSuspendCount = 0;
static ntyGpsSyncDatetimeCallBack ntyGpsSyncDatetimeCb = NULL;

void ntyGpsSyncCallback(void);
//extern void *ntyLocationInstance(void);

HANDLE_LOCATION ntySetNMEAGGACb = NULL;
HANDLE_LOCATION ntySetNMEARMCCb = NULL;
HANDLE_LOCATION ntySetPMTK741CmdCb = NULL;




typedef enum {
	GSS_IDLE,
	GSS_LOCATIONING,
	GSS_LOCATIONED,
	GSS_SUSPEND,
} GPSSYNCSTATUS_E;
GPSSYNCSTATUS_E g_gpsSyncStatus;

U32 g_uiGpsScanStartTime = 0;
U32 g_uiGpsValidTime = 0;
U8   g_bFinishedGpsLoc = TRUE;
static U8 g_bEnableGpsSchedule = FALSE;

#define LIMIT_UPLOAD_THRESHOLD      (5.0f)  //Uint:m
#define EARTH_RADIUS                (6378.137*1000)  //Uint :m
#define Pi                          (3.14159265)

static U32 g_uiGetGPSCount  = 0;

void ntySetNMEAGGACallback(HANDLE_LOCATION cb) {
	ntySetNMEAGGACb = cb;
}

void ntySetNMEARMCCallback(HANDLE_LOCATION cb) {
	ntySetNMEARMCCb = cb;
}

void ntySetPMTK741CmdCallback(HANDLE_LOCATION cb) {
	ntySetPMTK741CmdCb = cb;
}

static double ntyRad(double d) {
	return d * Pi / 180.0;
} 

static double ntyGetDistance(double lng1,double lat1, double lng2,  double lat2) {
	double radLat1 = ntyRad(lat1);
	double radLat2 = ntyRad(lat2);
	double a = radLat1 - radLat2;
	double b = ntyRad(lng1) - ntyRad(lng2);

	double s = 2 * asin(sqrt(pow(sin(a/2),2) +cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));
	s = s * EARTH_RADIUS;
	
	return s;
}

#if 1 //Code From Coretek

kal_char* ntyGPSItoa(kal_char* str, const kal_uint32 val, const kal_uint32 n_digits) {
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    kal_char *str_p;
    kal_char buffer[14];
    kal_uint32 buffer_i;
    kal_uint32 t_val;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    buffer_i = 0;
    t_val = val;
    while (t_val > 0) {
        buffer[buffer_i++] = (kal_uint8)('0' + (t_val % 10));
        t_val /= 10;
    }

    while (buffer_i < n_digits && buffer_i < sizeof(buffer)) {
        buffer[buffer_i++] = '0';
    }

    /* reverse & copy to str */
    str_p = str;
    while (buffer_i > 0) {
        buffer_i--;
        *(str_p++) = buffer[buffer_i];
    }

    if ( str_p == str ) {
        *(str_p++) = '0';
    }

    *str_p = '\0';

    return str;
}


U8 ntyLocatSendGPSParam(void)
{
	kal_uint8 checksum, gps_command[GPS_LOCATION_CMD_MAX_LEN]={0};
	kal_int32 i, length, gps_wn, gsm_wn, diff_wn, age_tow;
	applib_time_struct time;
	kal_uint32 written;

    kal_char time_name[GPS_LOCATION_CMD_MAX_LEN];
    kal_char* gps_command_tail;
	applib_time_struct t;
	kal_uint32 cmp_time;
	
	applib_dt_get_rtc_time(&t);
	cmp_time=applib_dt_sec_local_to_utc(applib_dt_mytime_2_utc_sec(&t, KAL_FALSE));	

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    
	//kal_prompt_trace(MOD_IDLE,"locat_send_gps_param start \n");
	gps_command_tail = time_name;
	//GetDateTime(&time);
	 applib_dt_utc_sec_2_mytime(cmp_time, &time, MMI_TRUE);
	ntyGPSItoa(gps_command_tail, time.nYear, 0);
	gps_command_tail += strlen(gps_command_tail);
	strcat(gps_command_tail, ",");
	gps_command_tail++;
	ntyGPSItoa(gps_command_tail, time.nMonth, 2);
	gps_command_tail += 2;
	strcat(gps_command_tail, ",");
	gps_command_tail++;
	ntyGPSItoa(gps_command_tail, time.nDay, 2);
	gps_command_tail += 2;
	strcat(gps_command_tail, ",");
	gps_command_tail++;
	ntyGPSItoa(gps_command_tail, time.nHour, 2);
	gps_command_tail += 2;
	strcat(gps_command_tail, ",");
	gps_command_tail++;
	ntyGPSItoa(gps_command_tail, time.nMin, 2);
	gps_command_tail += 2;
	strcat(gps_command_tail, ",");
	gps_command_tail++;
	ntyGPSItoa(gps_command_tail, time.nSec, 2);
	gps_command_tail += 2;
	strcat(gps_command_tail, "*");
	
	length = strlen((const char *)time_name);
	
#if 0
	sprintf((char *)gps_command, "$PMTK741,%s,%s,%s,", g_gps_lbs.lat, g_gps_lbs.lon,g_gps_lbs.altitude);
#else
	if (ntySetPMTK741CmdCb != NULL) {
		(*ntySetPMTK741CmdCb)(gps_command);
	}
#endif
	strcat((char *)gps_command, time_name);
	nty_printf("gps cmd : %s", gps_command);
	
   	length = strlen((const char *)gps_command);
  // kal_prompt_trace(MOD_IDLE,"locat_send_gps_param gps_command length=%d\n",length);
   if(length > GPS_LOCATION_CMD_MAX_LEN - 5) return;

   checksum = 0;
   for (i = 1; i < length - 1; i++)
   {
	   checksum ^= gps_command[i];		  
   }

   if((checksum >> 4) > 9)
   {
	   gps_command[length] = (checksum >> 4) - 10 + 'A';
   }
   else
   {
	   gps_command[length] = (checksum >> 4) + '0';
   }
   if((checksum & 0xf) > 9)
   {
	   gps_command[length + 1] = (checksum & 0xf) - 10 + 'A'; 
   }
   else
   {
	   gps_command[length + 1] = (checksum & 0xf) + '0';
   }
   gps_command[length + 2] = 0x0D;
   gps_command[length + 3] = 0x0A;
   gps_command[length + 4] = '\0';
   nty_printf("locat_send_gps_param %s \n",gps_command);

   mdi_gps_uart_write(GPS_Get_UART_Port(), gps_command, length + 4,&written);  
}

#endif


static void ntyGPSParseGPGGA(mdi_gps_nmea_gga_struct *gpgga) 
{
#if 0
	//if (g_bEnableGpsSchedule)
	{
		U32 uiGpsLocCostTime = 0;
		double dDistance = 0.0f;
		nty_printf("sat_in_view = %d", gps_info_ex.sat_in_view);
		uiGpsLocCostTime = ntyGetUtcSec() - g_uiGpsScanStartTime;
		if((gpgga->latitude != 0.0 || gpgga->longitude != 0.0))
		{
			g_uiGetGPSCount++;
			//dDistance = GetDistance(gps_info_ex.longitude,gps_info_ex.latitude,gpgga->longitude,gpgga->latitude);

			//if(dDistance > LIMIT_UPLOAD_THRESHOLD) 
			{
				gps_info_ex.uiTime    = g_uiGpsScanStartTime + uiGpsLocCostTime;
				gps_info_ex.latitude   = gpgga->latitude * ((gpgga->north_south == 'N') ? 1: -1);
				gps_info_ex.longitude = gpgga->longitude * ((gpgga->east_west == 'E') ? 1: -1);
				gps_info_ex.station_id = gpgga->station_id;
				gps_info_ex.sat_in_view = gpgga->sat_in_view;
			}
			//Need get 4 time, we will think the data is enough
			if(g_uiGetGPSCount > 3)
			{
				g_uiGpsValidTime = gps_info_ex.uiTime;
				g_bFinishedGpsLoc      = TRUE;
			}
			nty_printf("GPS Valid data,latitude:%f,longitude:%f,ScanGpsCost:%d s,Q:%d", 
			    gpgga->latitude,gpgga->longitude,uiGpsLocCostTime,gpgga->quality);
		}
		else if( (uiGpsLocCostTime > GPS_LOC_MONITOR_TIMEOUT ) && (g_uiGpsValidTime != 0 )
		&& ((ntyGetUtcSec() - g_uiGpsValidTime) < (2*60*60)))//Beyond warm start time. 2 Hour
		{
			g_bFinishedGpsLoc      = TRUE;    	
		}
		else if(uiGpsLocCostTime > GPS_LOC_SCAN_TIMEOUT-5)
		{
			g_bFinishedGpsLoc      = TRUE;
		}

		if(g_bFinishedGpsLoc || (uiGpsLocCostTime%5 == 0))
		{
	       nty_printf("GPS,latitude:%f, longitude:%f,dist:%f,ScanGpsCost:%d s,delLocT:%d s, Q:%d", 
			gpgga->latitude,gpgga->longitude,dDistance,uiGpsLocCostTime,(ntyGetUtcSec() - g_uiGpsValidTime),gpgga->quality);
 		}

		if(g_bFinishedGpsLoc)
		{
			ntyGPSClose();
			StopTimer(NATTY_GPSTIMEOUT_TIMER);
#if 0
			kwp_timer_stop(KWP_TIMER_SCAN_GPS);	

			//Think that the position is as same as previous one
			//if(dDistance < LIMIT_UPLOAD_THRESHOLD && g_uiGpsValidTime >= g_uiGpsScanStartTime) 
			{
			//	kwp_timer_stop(KWP_TIMER_MONITOR_GPS);
			}
			//else
			{
				kwp_timer_start_at_once(KWP_TIMER_MONITOR_GPS);

			}
#endif
		}
	}
#endif
#if 0
	Location *loc = ntyLocationInstance();

	if('1' == gpgga->quality) // 有效定位
	{	
		sprintf(g_gps_lbs.altitude, "%f", gpgga->altitude);
		//g_gps_lbs.altitude = gga_data->altitude;

	}
#endif

	if (ntySetNMEAGGACb != NULL && '1' == gpgga->quality) {
		(*ntySetNMEAGGACb)(gpgga);
	}
}


const unsigned char dayOfMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
static void ntyGPSParseGPRMC(mdi_gps_nmea_rmc_struct *gprmc) {
	//kwp_debug_print("kwp_gps_prase_gprmc");
#if 0
	if (gprmc->status == 'A')
	/*gprmc->utc_date.year = 16;
	gprmc->utc_date.month = 3;
	gprmc->utc_date.day = 17;
	gprmc->utc_time.hour = 11;
	gprmc->utc_time.minute = 42;
	gprmc->utc_time.second = 16;
	gprmc->east_west = 'E';
	gprmc->longitude = 113.945;*/
	{
		struct tm l_time;
		int eastOrWest = 1;
#if 0
		kwp_time_t local_time;
		kwp_time_t cur_time;
#else
		ntyTime local_time;
		ntyTime	cur_time;

		//applib_dt_get_rtc_time(&dt);
#endif
		unsigned char k;
		unsigned int total_sec;
		char tmp_zone = 0;
		U32 tmp_day;
		if (gprmc->east_west == 'E') 
			eastOrWest = 1;
		else if (gprmc->east_west == 'W')
			eastOrWest = -1;
		tmp_day = (30 + gprmc->utc_date.year) * 365 + (27 + gprmc->utc_date.year) / 4 + 1;
		//kwp_debug_print("tmp_day1 = [%d]",tmp_day);
		for (k=0; k<(gprmc->utc_date.month-1); k++) {
			tmp_day += dayOfMonth[k];
			//kwp_debug_print("tmp_day2 = [%d], index = [%d]",tmp_day,k);
		}
		//kwp_debug_print("tmp_day3 = [%d]",tmp_day);
		tmp_day += (gprmc->utc_date.day - 1);
		//kwp_debug_print("tmp_day4 = [%d]",tmp_day);
		if (gprmc->utc_date.month > 2) {
			if ((2000+gprmc->utc_date.year)%4 == 0) {
				tmp_day += 1;
			}
		}
		//kwp_debug_print("tmp_day = [%d]",tmp_day);
		total_sec = tmp_day * 24 * 60 * 60;
		total_sec += (gprmc->utc_time.hour * 3600 + gprmc->utc_time.minute * 60 + gprmc->utc_time.second);
#if 0		
		kwp_get_rtc_time(&cur_time);
#else
		ntyGetRtcTime(&cur_time);
#endif
		if (cur_time.nYear <= 2004) {
			if (gprmc->longitude > 7.5) {
				tmp_zone = ((gprmc->longitude - 7.5) / 15 + 1) * eastOrWest;
			}
		} else {
			//tmp_zone = kw_config.set_info.uiTimeZoneSet - 12;
			tmp_zone = 8;
		}
		ntyLocalTime((U32)total_sec, 3600 * (tmp_zone) * eastOrWest, &l_time);
		//kwp_debug_print("year=[%d],month=[%d],day=[%d],hour=[%d],min=[%d],sec=[%d]",l_time.tm_year,l_time.tm_mon,l_time.tm_mday,l_time.tm_hour,l_time.tm_min,l_time.tm_sec);
		local_time.nYear= l_time.tm_year+1900;
		local_time.nMonth= l_time.tm_mon+1;
		local_time.nDay = l_time.tm_mday;
		local_time.nHour = l_time.tm_hour;
		local_time.nMin = l_time.tm_min;
		local_time.nSec = l_time.tm_sec;
		//kwp_debug_print("year=[%d],month=[%d],day=[%d],hour=[%d],min=[%d],sec=[%d]",local_time.year,local_time.mon,local_time.day,local_time.hour,local_time.min,local_time.sec);
		if (ntyGpsSyncDatetimeCb != NULL) {
			(*ntyGpsSyncDatetimeCb)(&local_time);
			g_gpsSyncStatus = GSS_SUSPEND;
			ntyGPSClose();
			g_gpsSyncCount = 0;
			g_gpsSuspendCount = 0;
		}
        nty_printf("latitude=[%f]",gprmc->latitude);
        nty_printf("longitude=[%f]",gprmc->longitude);
        nty_printf("ground_speed=[%f]",gprmc->ground_speed);
        nty_printf("trace_degree=[%f]",gprmc->trace_degree);
        nty_printf("magnetic=[%f]",gprmc->magnetic);
        nty_printf("status=[%d]",gprmc->status);
        nty_printf("north_south=[%d]",gprmc->north_south);
        nty_printf("east_west=[%d]",gprmc->east_west);
        nty_printf("magnetic_e_w=[%f]",gprmc->magnetic_e_w);
        nty_printf("cmode=[%d]",gprmc->cmode);
        nty_printf("nav_status=[%d]",gprmc->nav_status);
        nty_printf("year=[%d],month=[%d],day=[%d]",gprmc->utc_date.year,gprmc->utc_date.month,gprmc->utc_date.day);
        nty_printf("hour=[%d],min=[%d],sec=[%d]",gprmc->utc_time.hour,gprmc->utc_time.minute,gprmc->utc_time.second);

	}
#endif
	
	if (ntySetNMEARMCCb != NULL) {
		(*ntySetNMEARMCCb)(gprmc);
	}
}



static void ntyMMIFmGPSNMEACallback(mdi_gps_parser_info_enum type, void *buffer, U32 length) 
{
	//nty_printf("ntyMMIFmGPSNMEACallback");
	if(buffer)
	{
        //nty_printf("mmi_fm_gps_nmea_callback 22 type=[%d]",type);
		switch(type) {
			case MDI_GPS_PARSER_NMEA_GGA:
				ntyGPSParseGPGGA((mdi_gps_nmea_gga_struct*)buffer);
				break;
			case MDI_GPS_PARSER_NMEA_RMC:
				if (NULL != ntyGpsSyncDatetimeCb)
					ntyGPSParseGPRMC((mdi_gps_nmea_rmc_struct *)buffer);
				break;
			case MDI_GPS_PARSER_NMEA_GSA:
				//vm_sal_gps_nmea_gsa_callback((mdi_gps_nmea_gsa_struct *)buffer);
				break;
			case MDI_GPS_PARSER_NMEA_GSV:
				//fm_gps_prase_gpgsv((mdi_gps_nmea_gsv_struct *)buffer);
				break;    
			case MDI_GPS_PARSER_NMEA_VTG:
				//vm_sal_gps_nmea_vtg_callback((mdi_gps_nmea_vtg_struct *)buffer);
				break;        
			case MDI_GPS_PARSER_NMEA_GLL:
				//vm_sal_gps_nmea_gll_callback((mdi_gps_nmea_gll_struct *)buffer);
				break;  
			case MDI_GPS_PARSER_RAW_DATA: {
				nty_printf("raw:%s", buffer);
				if(strncmp((char*)buffer, "$PMTK010,002*2D", 15)==0) {
               // wk_gps_send_assistance_data();
	              	ntyLocatSendGPSParam();					
	            }
				
				break;
			}
		}
	}
}

void ntyGpsSyncDatetime(ntyTime *time) 
{
    ntyTime now;
    nty_printf("ntyGpsSyncDatetime");
    ntySetRtcTime(time);
    //update_mainlcd_dt_display();

    ntyGetRtcTime(&now);
    nty_printf("get year=%d month=%d day=%d hour=%d minute=%d second=%d", 
              now.nYear, now.nMonth, now.nDay, now.nHour, now.nMin, now.nSec);
    //kwp_UnregisterGpsSyncDatetime();
}


void ntyUnregisterGpsSyncDatetime() {
	ntyGpsSyncDatetimeCb = NULL;
}
void ntyRegisterGpsSyncDatetime(ntyGpsSyncDatetimeCallBack cb) {
	//kwp_debug_print("kwp_RegisterGpsSyncDatetime");
	ntyGpsSyncDatetimeCb = cb;
	//kwp_gps_open();
}

void ntyGpsSyncProc(void) {
	//kwp_debug_print("current status is [%d]",g_gpsSyncStatus);
	if (1 == g_gpsSyncEnableFlag) {
		switch(g_gpsSyncStatus) {
			case GSS_IDLE:
				ntyGPSOpen();
				g_gpsSyncStatus = GSS_LOCATIONING;
			break;
			case GSS_LOCATIONING:
				if (g_gpsSyncCount == (6*60)) {
					g_gpsSyncCount = 0;
					ntyGPSClose();
					g_gpsSuspendCount = 0;
					g_gpsSyncStatus = GSS_SUSPEND;
				} else {
					g_gpsSyncCount ++;
				}
			break;
			case GSS_SUSPEND:
				if (g_gpsSuspendCount == (60*60)) {
					ntyGPSOpen();
					g_gpsSyncStatus = GSS_LOCATIONING;
					g_gpsSuspendCount = 0;
				} else {
					g_gpsSuspendCount ++;
				}
			break;
		}
		StartTimer(NATTY_GPSSYNC_TIMER, 1000, ntyGpsSyncCallback);
	}
}


void ntyGpsSyncCallback(void) {
	ntyGpsSyncProc();
}


void ntyGpsSyncEnable(void) {
	nty_printf("kwp_GpsSyncEnable");
	if (g_gpsSyncEnableFlag == 0) {
		g_gpsSyncEnableFlag = 1;
		ntyRegisterGpsSyncDatetime(ntyGpsSyncDatetime);
		g_gpsSyncStatus = GSS_IDLE;
		StartTimer(NATTY_GPSSYNC_TIMER, 1000, ntyGpsSyncCallback);
		//StartTimer(NATTY_GPSSCAN_TIMER, 120*1000, ntyGPSStartScan);
	}
}

void ntyGpsScanTimeout(void) {
	nty_printf("ntyGpsScanTimeout");
	ntyGPSClose();
	StopTimer(NATTY_GPSTIMEOUT_TIMER);
}

void ntyGpsScanStop(void) {
	nty_printf("ntyGpsScanStop");
	ntyGpsScanTimeout();
}

U8 ntyGPSOpen(void) {
	nty_printf("kwp_gps_open %d", mmi_fm_gps_open_flag);

	if (mmi_fm_gps_open_flag == 0) {
		int gps_handle;

		g_uiGpsScanStartTime = ntyGetUtcSec();
		g_bFinishedGpsLoc      = FALSE;
		local_gps_port = mdi_get_gps_port();

		g_uiGetGPSCount = 0;
		g_gps_fixed_flag = 0;

		GPS_Init(NULL); 
		gps_handle = mdi_gps_uart_open(local_gps_port, MDI_GPS_UART_MODE_LOCATION, ntyMMIFmGPSNMEACallback);
		nty_printf("gps uart open ret %d", gps_handle);
		//kal_prompt_trace(MOD_IDLE,"gps_test_open_gps 1111 ret=%d\n",ret);
		gps_handle = mdi_gps_uart_open(local_gps_port, MDI_GPS_UART_MODE_RAW_DATA, ntyMMIFmGPSNMEACallback);
		nty_printf("gps_test_open_gps 2222 ret=%d\n",gps_handle);
			
		StartTimer(NATTY_GPSTIMEOUT_TIMER, 95*1000, ntyGpsScanTimeout);
		//if(g_uiGpsValidTime == 0)
		//mdi_gps_uart_cmd(local_gps_port,MDI_GPS_UART_GPS_COLD_START,NULL);
		//else
		//mdi_gps_uart_cmd(local_gps_port,MDI_GPS_UART_GPS_WARM_START,NULL);

		mmi_fm_gps_open_flag = 1;
	}
	return 0;
}

U8 ntyGPSClose(void) {
    nty_printf("kwp_gps_close:%d ",mmi_fm_gps_open_flag);

    if( mmi_fm_gps_open_flag == 1) {
		g_uiGetGPSCount = 0;
		g_gpsSyncEnableFlag = 0;
		GPS_Shutdown();	
		mdi_gps_uart_close(local_gps_port, MDI_GPS_UART_MODE_RAW_DATA, ntyMMIFmGPSNMEACallback);
		mdi_gps_uart_close(local_gps_port, MDI_GPS_UART_MODE_LOCATION, ntyMMIFmGPSNMEACallback);
		mmi_fm_gps_open_flag = 0;
		g_bFinishedGpsLoc        = TRUE;
		g_gps_fixed_flag = 0;	
    }
    return 0;
}


void ntyGPSStartScan(void) {
	StopTimer(NATTY_GPSSCAN_TIMER);
	ntyGPSOpen();
	StartTimer(NATTY_GPSSCAN_TIMER, 120*1000, ntyGPSStartScan);
}

