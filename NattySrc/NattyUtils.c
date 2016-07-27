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

#include "NattyUtils.h"
#include "NattyProtocol.h"
#include "UCMGprot.h"
#include "mmi_frm_mem_gprot.h"
#include "custom_mmi_default_value.h"
#include "mmi_msg_struct.h"
#include "EventsGprot.h"
#include "l4c_common_enum.h"
#include "mmi_frm_queue_gprot.h"
#include "NwInfoSrvGprot.h"
//#include "UcmProt.h"
#include "TimerEvents.h"
#include "ProtocolEvents.h"
#include "UcmSrvGprot.h"
//#include "UcmGProt.h"
#include "Device.h"


U8 ntyGetBatteryLevel(void) {
    U8 level = mmi_gpio_get_current_battery_level();
    if (level < 2)
    {
        level = 2;
        
    }
    if (level > 6)
    {
        level = 6;
    }
    return level;
}

U8 ntyGetSignalLevel(void) {
	return srv_nw_info_get_signal_strength_in_percentage(MMI_SIM1);
}

extern void sn3730_led_on(void);

void ntySetSystemTime(U8 *buf) {
	MYTIME myTime;
	
	myTime.nYear = buf[NTY_PROTO_TIMECHECK_YEAR_IDX] % 100 + 2000;
	myTime.nMonth = buf[NTY_PROTO_TIMECHECK_MONTH_IDX];
	myTime.nDay = buf[NTY_PROTO_TIMECHECK_DAY_IDX];
	myTime.DayIndex = buf[NTY_PROTO_TIMECHECK_WDAY_IDX];
	myTime.nHour = buf[NTY_PROTO_TIMECHECK_HOUR_IDX];
	myTime.nMin = buf[NTY_PROTO_TIMECHECK_MINUTE_IDX];
	myTime.nSec = buf[NTY_PROTO_TIMECHECK_SECOND_IDX];
	mmi_dt_set_rtc_dt(&myTime);
	
	sn3730_led_on();

	//kal_prompt_trace(MOD_BT,"hour:%d, min:%d, sec:%d\n", myTime.nHour, myTime.nMin, myTime.nSec);
}

void ntySetRtcTime(ntyTime *time) {
	#if 0
	MYTIME myTime;
	
	myTime.nYear = time->nYear;
	myTime.nMonth = time->nMonth;
	myTime.nDay = time->nDay;
	myTime.DayIndex = time->DayIndex;
	myTime.nHour = buf[NTY_PROTO_TIMECHECK_HOUR_IDX];
	myTime.nMin = buf[NTY_PROTO_TIMECHECK_MINUTE_IDX];
	myTime.nSec = buf[NTY_PROTO_TIMECHECK_SECOND_IDX];
	#endif
	
	mmi_dt_set_rtc_dt((MYTIME*)time);
	
	//sn3730_led_on();

	//kal_prompt_trace(MOD_BT,"hour:%d, min:%d, sec:%d\n", myTime.nHour, myTime.nMin, myTime.nSec);
}

void ntyGetRtcTime(ntyTime *time) {
	applib_dt_get_rtc_time(time);
}

U32 ntyGetUtcTime(void) {
	U32 sec = app_getcurrtime();
	return sec;
}

void ntySetDefaultSystemTime(void) {
	MYTIME myTime;
	
	myTime.nYear = 2016;
	myTime.nMonth = 6;
	myTime.nDay = 30;
	myTime.DayIndex = 4;
	myTime.nHour = 10;
	myTime.nMin = 30;
	myTime.nSec = 30;
	mmi_dt_set_rtc_dt(&myTime);
}

#define SECS_PER_HOUR   (60 * 60)
#define SECS_PER_DAY    (SECS_PER_HOUR * 24)

static S32 isleap(int year) { 
    if(year%4==0 && year%100!=0 || year%400==0) 
        return 1; 
    else 
        return 0; 
}

static const U16 mon_yday[2][13] = {
	/* Normal years.  */
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
	/* Leap years.  */
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};


/* Compute the `struct tm' representation of *T,
   offset OFFSET seconds east of UTC,
   and store year, yday, mon, mday, wday, hour, min, sec into *TP.
   Return nonzero if successful.  */
U8 ntyLocalTime (const U32 t, S32 offset, struct tm *tp)
{
  S32 days, rem, y;
  const unsigned short *ip;

  days = t / SECS_PER_DAY;
  rem = t % SECS_PER_DAY;
  rem += offset;
  while (rem < 0)
    {
      rem += SECS_PER_DAY;
      --days;
    }
  while (rem >= SECS_PER_DAY)
    {
      rem -= SECS_PER_DAY;
      ++days;
    }
  tp->tm_hour = rem / SECS_PER_HOUR;
  rem %= SECS_PER_HOUR;
  tp->tm_min = rem / 60;
  tp->tm_sec = rem % 60;
  /* January 1, 1970 was a Thursday.  */
  tp->tm_wday = (4 + days) % 7;
  if (tp->tm_wday < 0)
    tp->tm_wday += 7;
  y = 1970;

#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

  while (days < 0 || days >= (isleap (y) ? 366 : 365))
    {
      /* Guess a corrected year, assuming 365 days per year.  */
      U32 yg = y + days / 365 - (days % 365 < 0);

      /* Adjust DAYS and Y to match the guessed year.  */
      days -= ((yg - y) * 365
               + LEAPS_THRU_END_OF (yg - 1)
               - LEAPS_THRU_END_OF (y - 1));
      y = yg;
    }
  tp->tm_year = y - 1900;
  if (tp->tm_year != y - 1900)
    {
      /* The year cannot be represented due to overflow.  */
      //__set_errno (EOVERFLOW);
      return 0;
    }
  tp->tm_yday = days;
  ip = mon_yday[isleap(y)];
  for (y = 11; days < (U32) ip[y]; --y)
    continue;
  days -= ip[y];
  tp->tm_mon = y;
  tp->tm_mday = days + 1;
  return 1;
}



U32 ntyGetUtcSec(void)
{
	return app_getcurrtime();
}


void ntyColdRestart(void) {
 	/*----------------------------------------------------------------*/
	/* Local Variables                                                */
	/*----------------------------------------------------------------*/
	//S16 error;
	//kal_uint16 i;
	//U8 play_welcome=2;
	
	/*----------------------------------------------------------------*/
	/* Code Body													   */
	/*----------------------------------------------------------------*/
	kal_prompt_trace(MOD_BT,"Restore system OK\n\r");
	srv_shutdown_normal_start(0); 
	
}



#if 1 //Code From 52rd.com
void ntyMMILmcGetICCIDRSP(void *info) {
	mmi_smu_read_sim_rsp_struct *iccid_data;
	kal_uint8 iccid[21];
	U8 i = 0;
	
	iccid_data = (mmi_smu_read_sim_rsp_struct*)info;
	ClearProtocolEventHandler(MSG_ID_MMI_SMU_READ_SIM_RSP);

	if (iccid_data->result.flag == L4C_SUCCESS) {
		kal_prompt_trace(MOD_BT, "Read iccid success!\n");
		for (i = 0;i < iccid_data->length;i ++) {
			iccid[2*i] = 0x30 + (*(iccid_data->data + i) & 0x0F);
			iccid[2*i+1] = 0x30 + ((*(iccid_data->data + i) & 0xF0) >> 4);
		}
		iccid[20] = 0;
		kal_prompt_trace(MOD_BT, "ICCID:%s\n", iccid);
	} else {
		kal_prompt_trace(MOD_BT, " read ICCID failed");
	}
}


void ntyMMILmcGetICCIDReq(void) {
	MYQUEUE Message;
	mmi_smu_read_sim_req_struct *dataPtr;

	SetProtocolEventHandler(ntyMMILmcGetICCIDRSP, MSG_ID_MMI_SMU_READ_SIM_RSP);

	Message.oslSrcId = MOD_MMI;
	Message.oslDestId = MOD_L4C;
	Message.oslMsgId = MSG_ID_MMI_SMU_READ_SIM_REQ;
	dataPtr = (mmi_smu_read_sim_req_struct*)OslConstructDataPtr(sizeof(mmi_smu_read_sim_req_struct));
	dataPtr->file_idx = (U8)FILE_ICCID_IDX;
	dataPtr->para = 0;
	dataPtr->length = 0x0A;

	Message.oslDataPtr = (oslParaType*)dataPtr;
	Message.oslPeerBuffPtr = NULL;

	OslMsgSendExtQueue(&Message);
}

void ntyMMILmcGetIMSIRSP(void *info) {
	mmi_smu_get_imsi_rsp_struct *imsi_data;
	kal_uint8 imsi[17];
	U8 i = 0;

	ClearProtocolEventHandler(PRT_GET_IMSI_RSP);
	imsi_data = (mmi_smu_get_imsi_rsp_struct*)info;
	//if (imsi_data->result.flag == L4C_SUCCESS) {
	//kal_prompt_trace(MOD_BT, "Read imsi success!\n");
	#if 0
	for (i = 0;i < imsi_data->length;i ++) {
			imsi[2*i] = 0x30 + (*(imsi_data->imsi+ i) & 0x0F);
			imsi[2*i+1] = 0x30 + ((*(imsi_data->data + i) & 0xF0) >> 4);
	}
	imsi[16] = 0;
	#else
	strcpy(imsi, imsi_data->imsi);
	#endif
	kal_prompt_trace(MOD_BT, "IMSI:%s\n", imsi);
	//} else {
	//	kal_prompt_trace(MOD_BT, " read IMSI failed");
	//}

}

void ntyMMILmcGetIMSIReq(void) {
	MYQUEUE Message;
	//mmi_smu_get_imsi_rsp_struct *dataPtr;

	SetProtocolEventHandler(ntyMMILmcGetIMSIRSP, PRT_GET_IMSI_RSP);

	Message.oslSrcId = MOD_MMI;
	Message.oslDestId = MOD_L4C;
	Message.oslMsgId = PRT_GET_IMSI_REQ;
	

	Message.oslDataPtr = NULL;
	Message.oslPeerBuffPtr = NULL;

	OslMsgSendExtQueue(&Message);
	kal_prompt_trace(MOD_BT, "ntyMMILmcGetIMSIReq\n");
}


void ntySetVolumeLevelReq(void) {
	MYQUEUE Message;  
    mmi_eq_set_volume_req_struct *setVolumeLevelReq;  
    Message.oslMsgId = MSG_ID_MMI_EQ_SET_VOLUME_REQ;//设置消息类型为设置音量  
    //在l4a.h中定义  
    setVolumeLevelReq = OslConstructDataPtr(sizeof(mmi_eq_set_volume_req_struct));  
    //创建 local parameter buffer  
    setVolumeLevelReq->volume_type = VOL_TYPE_CTN;  
    setVolumeLevelReq->volume_level = 100;  
    Message.oslDataPtr = (oslParaType *)setVolumeLevelReq; //Local parameter buffer  
    Message.oslPeerBuffPtr= NULL; //Peer parameter buffer  
    Message.oslSrcId=MOD_MMI; //设置源模块  
    Message.oslDestId=MOD_L4C; //设置目标模块  
    OslMsgSendExtQueue(&Message); //发送到L4  
}

#endif

#if 1 //Call phone
extern kal_uint8 u8LedDisplayMode;
extern void mmiKeypadInit(void);
extern void mmi_ucm_incoming_call_sendkey(void);

kal_bool ntyCallNumberIsAllowed(char callType,U8 *number) {
	u8LedDisplayMode = 2;
	StartTimer(REGISTER_KEYPAD_TIMER, 500, mmiKeypadInit);
	StartTimer(CALL_TIMER, 10000, mmi_ucm_incoming_call_sendkey);

	return KAL_TRUE;
}

void ntyEndCallCb(void)//when call is released,it calls this cb
{
	kal_prompt_trace(MOD_BT, "ntyEndCallCb\n");
	//YxAppCallEndProc();
	u8LedDisplayMode = 1; //return to time MainPage
}


#define NTYISINCALL(a)   srv_ucm_query_call_count(a, SRV_UCM_VOICE_CALL_TYPE_ALL, NULL)
char ntyMakeCall(char check,char *number)//ascii code
{
	if(NTYISINCALL(SRV_UCM_CALL_STATE_ALL) > 0)
		return 0;
	else
	{
		if((strlen(number) <= 0) || (strlen(number) >= NTY_CALL_NUMBER_MAX_LENGTH))
			return 0;
		if(ntyIsAvailablePhnum(number)==0)
			return 0;
		else
		{
			U8 phoneNum[NTY_CALL_NUMBER_MAX_LENGTH+1] = {0};
			//phoneNum[0] = NTY_MSG_CALL_CTRL;
			strcpy(phoneNum, number);
			if(check)
				return 1;
			//memset((void*)yxDataPool,0,YX_APP_CALL_NUMBER_MAX_LENGTH+1);
			//strcpy(yxDataPool,number);
			//YxAppSendMsgToMMIMod(YX_MSG_CALL_CTRL,YX_DATA_MAKE_CALL,0);
			kal_prompt_trace(MOD_BT, "number:%s\n", number);
			
			ntySendMsgToMMIMod(NTY_MSG_CALL_CTRL, phoneNum, 12);
		}
		return 1;
	}
}

//struct mmi_ucm_make_call_para_struct;
void display_callout(void);

void ntyLaunchCall(char *number) {
#if 1
	U16   phnum[NTY_CALL_NUMBER_MAX_LENGTH+1],i = 0;
	mmi_ucm_make_call_para_struct  make_call_para;


	kal_prompt_trace(MOD_BT, "ntyLaunchCall:%s\n", number);
	mmi_ucm_init_call_para(&make_call_para);
	while(number[i])
	{
		phnum[i] = number[i];
		i++;
		if(i>=NTY_CALL_NUMBER_MAX_LENGTH)
			break;
	}
	phnum[i] = 0;
	make_call_para.ucs2_num_uri = (U16*)phnum;
	mmi_ucm_call_launch(0, &make_call_para);
	//u8LedDisplayMode = 3;
	display_callout();
#endif
}


#endif

#if 1
void *ntyRealloc(void* pVar, int Size) {
	void *pNewVar = OslMalloc((Size+1)*sizeof(void*));
	if (pNewVar) {
		memcpy(pNewVar, pVar, Size);
		OslMfree(pVar);
	}
	return pNewVar;
}

#define ITEM_SIZE		64

int ntySeparation(char ch, char *sequence, char ***pChTable, int *Count) {
	int i = 0, j = 0;
	int len = strlen(sequence);
	char ChArray[ITEM_SIZE] = {0};
	char **pTable = *pChTable;
	
	*Count = 0;

	for (i = 0;i < len;i ++) {
		if (sequence[i] == ch) {
			pTable[*Count] = (char*)OslMalloc((j+1) * sizeof(char));
			memcpy(pTable[*Count], ChArray, j+1);
			(*Count) ++;

			//OslMfree(pTable);
			//pTable = (char**)OslMalloc((*Count+1) * sizeof(char**));
			pTable = (char**)ntyRealloc(pTable, (*Count+1) * sizeof(char**));
			j = 0;
			memset(ChArray, 0, ITEM_SIZE);

			continue;
		} 
		ChArray[j++] = sequence[i];
	}
	
	pTable[*Count] = (char*)OslMalloc((j+1) * sizeof(char));
	memcpy(pTable[*Count], ChArray, j+1);
	(*Count) ++;
	
	memset(ChArray, 0, ITEM_SIZE);

	*pChTable = pTable;

	return 0;
}

void ntyFreeTable(unsigned char ***table, int count) {
	int i = 0;
	unsigned char **pTable = *table;

	for (i = 0;i < count;i ++) {
		OslMfree(pTable[i]);
	}
	
}

#endif

#if 1 //String Operator
char ntyCompareGetOpera(const U8 *taken) {
	if (!strcmp(taken, "GET") || !strcmp(taken, "Get") || !strcmp(taken, "get")) {
		return 1;
	}
	return 0;
}

char ntyCompareSetOpera(const U8 *taken) {
	if (!strcmp(taken, "SET") || !strcmp(taken, "Set") || !strcmp(taken, "set")) {
		return 1;
	}
	return 0;
}

void ntyU16Datacpy(U8 *send, U16 num) {
	U8* pNum = (U8*)(&num);
	int i = 0;

	for (i = 0;i < sizeof(U16);i ++) {
		send[i] = (U8)*(pNum+i);
	}
}

void ntyU32Datacpy(U8 *send, U32 num) {
	U8* pNum = (U8*)(&num);
	int i = 0;

	for (i = 0;i < sizeof(U32);i ++) {
		send[i] = (U8)*(pNum+i);
	}
}

void ntyU64Datacpy(U8 *send, C_DEVID num) {
	U8* pNum = (U8*)(&num);
	int i = 0;

	for (i = 0;i < sizeof(C_DEVID);i ++) {
		send[i] = (U8)*(pNum+i);
	}
}

char ntyHexToString(U8 hex, U8 *string) {
	U8 high, low;

	//if (strlen(string) < 2) return -1;

	high = (hex & 0xF0) >> 4;
	low = hex & 0x0F;

	if (high >= 0 && high <= 9) {
		string[0] = 0x30 + high;
	} else if (high >= 10 && high <= 16) {
		string[0] = 0x41 + high - 10;
	}

	if (low >= 0 && low <= 9) {
		string[1] = 0x30 + low;
	} else if (low >= 10 && low <= 16) {
		string[1] = 0x41 + low - 10;
	} 

	return 0;
}

U8 ntyArrayIsEmpty(U8 *array) {
	if (array[0] == 0) {
		return 0;
	}
	return 1;
}

#if 1
void ntyBuildNext(const char *pattern, U32 length, U32 *next) {
	U32 i = 1 ,t = 0;

	next[1] = 0;
	while (i < length + 1) {
		while (t > 0 && pattern[i-1] != pattern[t-1]) {
			t = next[t];
		}

		++ t;
		++ i;
		if (pattern[i-1] == pattern[t-1]) {
			next[i] = next[t];
		} else {
			next[i] = t;
		}
	}

	while (t > 0 && pattern[i-1] != pattern[t-1]) {
		t = next[t];
	}

	++ t;
	++ i;

	next[i] = t;
}

#define PATTERN_LENGTH_COUNT 32

U32 ntyKMP(const char *text,const U32 text_length, const char *pattern,const U32 pattern_length, U32 *matches) {
	U32 i,j,n;
	U32 next[PATTERN_LENGTH_COUNT];

	ntyBuildNext(pattern, pattern_length, next);

	i = 0;
	j = 1;
	n = 0;
	
	while (pattern_length + 1 - j <= text_length - i) {
		if (text[i] == pattern[j-1]) {
			++ i;
			++ j;
			if (j == pattern_length + 1) {
				matches[n++] = i - pattern_length;
				j = next[j];
			}
		} else {
			j = next[j];
			if (j == 0) {
				++i;
				++j;
			}
		}
	}

	return n;
}

#endif

char ntyIsAvailablePhnum(char *phnum)
{
	while(*phnum != 0)
	{
		if (   *phnum != '0'
			&& *phnum != '1'
			&& *phnum != '2'
			&& *phnum != '3'
			&& *phnum != '4'
			&& *phnum != '5'
			&& *phnum != '6'
			&& *phnum != '7'
			&& *phnum != '8'
			&& *phnum != '9'
			&& *phnum != 'P'
			&& *phnum != '*'
			&& *phnum != '+'
			&& *phnum != '#')
		{
			return 0;
		}
		phnum++;
	}
	return 1;
}


#if 0
void ntyU8ArrayToU16(U16 *num, U8 *buf) {
	U16 dat = 0;
	int i = 0;

	for (i = 0;i < sizeof(U16);i ++) {
		dat |= (buf[i] << 8*i);
		kal_prompt_trace(MOD_BT,"buf[%d]:%x, dat:%x", i, buf[i], dat);
	}
	*num = dat;
	kal_prompt_trace(MOD_BT,"num:%d", *num);
}


void ntyU8ArrayToU32(U32 *num, U8 *buf) {
	U32 dat = 0;
	int i = 0;

	for (i = 0;i < sizeof(U32);i ++) {
		dat |= (buf[i] << 8*i);
		kal_prompt_trace(MOD_BT,"buf[%d]:%x, dat:%x", i, buf[i], dat);
	}
	*num = dat;
	kal_prompt_trace(MOD_BT,"num:%d", *num);
}

void ntyU8ArrayToU64(C_DEVID *num, U8 *buf) {
	C_DEVID id = 0;
	int i = 0;

	for (i = 0;i < sizeof(C_DEVID);i ++) {
		id |= (buf[i] << 8*i);
		kal_prompt_trace(MOD_BT,"buf[%d]:%x, dat:%x", i, buf[i], id);
	}
	*num = id;
}
#else
U16 ntyU8ArrayToU16(U8 *buf) {
	U16 dat1 = 0;
	int i = 0;

	for (i = 0;i < sizeof(U16);i ++) {
		dat1 |= (buf[i] << 8*i);
		//kal_prompt_trace(MOD_BT,"buf[%d]:%x, dat:%x", i, buf[i], dat1);
	}

	//kal_prompt_trace(MOD_BT,"U16 dat:%x", dat1);
	return dat1;
}


U32 ntyU8ArrayToU32(U8 *buf) {
	U32 dat2 = 0;
	int i = 0;

	for (i = 0;i < sizeof(U32);i ++) {
		dat2 |= (buf[i] << 8*i);
	}
	//kal_prompt_trace(MOD_BT,"U32 dat:%x", dat2);
	return dat2;
}

C_DEVID ntyU8ArrayToU64(U8 *buf) {
	C_DEVID id = 0;
	int i = 0;

	for (i = 0;i < sizeof(C_DEVID);i ++) {
		id |= (buf[i] << 8*i);
		//kal_prompt_trace(MOD_BT,"buf[%d]:%x, dat:%x", i, buf[i], id);
	}
	//kal_prompt_trace(MOD_BT,"u64 dat:%x", id);
	return id;
}

#endif

void ntyDevIdDatacpy(U8 *send, U8 *u8DevId) {
	int i = 0;
	
	for (i = 0;i < 8;i ++) {
		send[i] = *(u8DevId+i);
	}
}


void ntyDebugPrint(const char *fmt, ...) {
    char buf[0x100];
    va_list args;//define va_list

    memset(buf, 0x00, sizeof(buf));
    va_start(args, fmt);//init
    vsnprintf((char *) buf, sizeof(buf), (char *) fmt, args);//get all arguments
    va_end(args);//the end of get string

    dbg_print("%s\r\n",(const char *)buf);//real output string
    
}

char ntyInt2Char(char num) {
    char tmp;
    tmp = num;
    if (tmp < 0xa) 
    {
        tmp = '0'+tmp;
    }
    else
    {
        tmp = 'A'+tmp-0xA;
    }
    return tmp;
}

int ntyWstrlen(U16 *s) {
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    int i = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    if (s == NULL)
        return 0;

    for (i = 0; s[i] != 0; i++);

    return i;
}


#endif



#if 1 //fs 

U32 ntyReadFile(CHAR *fileName , U8 *data) {
	char configFile[128];
	U32 bytes_read = 0;
	FS_HANDLE fh;

	memset((void*)configFile, 0x00, 128);
	mmi_ucs2cpy((CHAR*)configFile, (const CHAR*)fileName);
	fh = FS_Open((const U16*)configFile,FS_READ_ONLY);
	if(fh >= FS_NO_ERROR) {
		U32   new_file_len = 0;
		FS_GetFileSize(fh,&new_file_len);
		if(new_file_len <= 128){
			FS_Read(fh,(void*)data,new_file_len,&bytes_read);
		} else {			
			new_file_len = 128;
			FS_Read(fh,(void*)data,new_file_len,&bytes_read);
		}
		FS_Close(fh);
	} else {
		return -1;
	}
	return bytes_read;
}

int ntyWriteFile(CHAR *fileName,U8 *data, int len) {
	char configFile[128];
	UINT   wrsize = 0;
	FS_HANDLE fh;
	

	memset((void*)configFile, 0x00, 128);
	mmi_ucs2cpy((CHAR*)configFile, (const CHAR*)fileName);

	fh = FS_Open((const U16*)configFile,FS_CREATE_ALWAYS|FS_READ_WRITE|FS_CACHE_DATA);
	if(fh>=FS_NO_ERROR) {
		
		FS_Write(fh,(void*)data,len,(UINT*)&wrsize);

		FS_Commit(fh);
		FS_Close(fh);
	}
	return wrsize;
}



#endif

#if 1 //ASCII, UNICODE, UTF-8
void ASCII2UCS2(U8* src, WCHAR* dest) {
	U8 index = 0;

	while(src[index] != '\0') {
		dest[index] = src[index];
		index++;
	}
	dest[index]=0 ;

	return;
}

#endif


#if 1 //State Run

ntyWatchConfig ntyWatchIntance;
U16 u16WatchStatusFlag = 0x0000;


int ntyConfigParser(char *data, int len) {
	int Count = 0, i, j;
	char **pTable = (char **)OslMalloc(sizeof(char**));
	
	//printf("%s\n", data);
	ntySeparation('\n', data, &pTable, &Count);

	for (i = 0;i < Count;i ++) {
		int OrderCount = 0;
		char **pItem = (char **)OslMalloc(sizeof(char**));
		printf("%s\n", pTable[i]);
		
		ntySeparation(':', pTable[i], &pItem, &OrderCount);
		if (!strcmp(pItem[0], "Step") && (OrderCount == 2)) {
			ntyWatchIntance.u8StepFlag = pItem[1][0]-0x30;
		} else if (!strcmp(pItem[0], "Sleep") && (OrderCount == 2)) {
			ntyWatchIntance.u8SleepFlag = pItem[1][0]-0x30;
		} else if (!strcmp(pItem[0], "SOSMsg") && (OrderCount == 2)) {
			ntyWatchIntance.u8SOSMsgFlag = pItem[1][0]-0x30;
		} else if (!strcmp(pItem[0], "PowerMsg") && (OrderCount == 2)) {
			ntyWatchIntance.u8PowerMsgFlag = pItem[1][0]-0x30;
		} else if (!strcmp(pItem[0], "AlarmMusic") && (OrderCount == 2)) {
			ntyWatchIntance.u8AlarmMusicFlag = pItem[1][0]-0x30;
		} else if (!strcmp(pItem[0], "SafeZoneMusic") && (OrderCount == 2)) {
			ntyWatchIntance.u8SafeZoneMusic = pItem[1][0]-0x30;
		} else if (!strcmp(pItem[0], "Freq") && (OrderCount == 2)) {
			ntyWatchIntance.u8Freq = pItem[1][0]-0x30;
		} else if ((!strcmp(pItem[0], "FamilyNumber")) && (OrderCount == 3)) {
			int idx = atoi(pItem[1]);
			if (idx <= 2) {
				strcpy(ntyWatchIntance.u8FamilyNumber[idx-1], pItem[2]);
			} 
		} else if ((!strcmp(pItem[0], "Contacts")) && (OrderCount == 3)) {
			int idx = atoi(pItem[1]);
			if (idx <= 10) {
				strcpy(ntyWatchIntance.u8ContactNumber[idx-1], pItem[2]);
			} 
		} else if ((!strcmp(pItem[0], "PhoneBook")) && (OrderCount == 3)) {
			int idx = atoi(pItem[1]);
			if (idx <= 10) {
				strcpy(ntyWatchIntance.u8PhoneBook[idx-1], pItem[2]);
			} 
		} else {
			printf("%s\n", pItem[0]);
		}

		for (j = 0;j < OrderCount;j ++) {
			OslMfree(pItem[j]);
		}
		OslMfree(pItem);
		OslMfree(pTable[i]);
	}
	OslMfree(pTable);

	return 0;
}

void ntyWatchStatusConfig(void) {
	if (ntyWatchIntance.u8StepFlag == 1) {
		u16WatchStatusFlag |= FLAG_COUNTSTEP_MODE;
	} 
	if (ntyWatchIntance.u8SleepFlag == 1) {
		u16WatchStatusFlag |= FLAG_SLEEPQUALITY_MODE;
	}
	if (ntyWatchIntance.u8SOSMsgFlag == 1) {
		u16WatchStatusFlag |= FLAG_SOSMSG_MODE;
	}
	if (ntyWatchIntance.u8PowerMsgFlag == 1) {
		u16WatchStatusFlag |= FLAG_POWERMSG_MODE;
	}
	if (ntyWatchIntance.u8AlarmMusicFlag == 1) {
		u16WatchStatusFlag |= FLAG_ALARMMUSIC_MODE;
	}
	if (ntyWatchIntance.u8SafeZoneMusic == 1) {
		u16WatchStatusFlag |= FLAG_SAFEZONEALARM_MODE;
	}
}


int ntyWatchConfigOrganization(char *data) {
	char buffer[ITEM_SIZE*4] = {0};
	int index = 0, i;

	sprintf(buffer, "Step:%d\nSleep:%d\nSOSMsg:%d\nPowerMsg:%d\nAlarmMusic:%d\nSafeZoneMusic:%d\nFreq:%d\n", 
		ntyWatchIntance.u8StepFlag, ntyWatchIntance.u8SleepFlag, ntyWatchIntance.u8SOSMsgFlag, ntyWatchIntance.u8PowerMsgFlag,
		ntyWatchIntance.u8AlarmMusicFlag, ntyWatchIntance.u8SafeZoneMusic, ntyWatchIntance.u8Freq );

	index = strlen(buffer);
	strcpy(data, buffer);

	for (i = 0;i < FAMILY_NUMBER_SIZE;i ++) {
		if (strlen(ntyWatchIntance.u8FamilyNumber[i]) > 6) {
			memset(buffer, 0, ITEM_SIZE*4);
			sprintf(buffer, "FamilyNumber:%d:%s\n", i+1, ntyWatchIntance.u8FamilyNumber[i]);
			strcpy(data+index, buffer);
			index += strlen(buffer);			
		}
	}

	for (i = 0;i < CONTACT_NUMBER_SIZE;i ++) {
		if (strlen(ntyWatchIntance.u8ContactNumber[i]) > 6) {
			memset(buffer, 0, ITEM_SIZE*4);
			sprintf(buffer, "Contacts:%d:%s\n", i+1, ntyWatchIntance.u8ContactNumber[i]);
			strcpy(data+index, buffer);
			index += strlen(buffer);			
		}
	}
	for (i = 0;i < PHONE_BOOK_SIZE;i ++) {
		if (strlen(ntyWatchIntance.u8PhoneBook[i]) > 6) {
			memset(buffer, 0, ITEM_SIZE*4);
			sprintf(buffer, "PhoneBook:%d:%s\n", i+1, ntyWatchIntance.u8PhoneBook[i]);
			strcpy(data+index, buffer);
			index += strlen(buffer);			
		}
	}
	return index;
}


int ntyWatchConfigInit(void) {
	U8 data[NATTY_WATCH_CONF_SIZE] = {0};
	U32 len = 0;
	
	len = ntyReadFile((CHAR*)NATTY_WATCH_CONF_FILE, data);
	if (len == -1) {
		ntyConfigParser(data, len);
		ntyWatchStatusConfig();
	} else {
		memset(&ntyWatchIntance, 0, sizeof(ntyWatchConfig));

		strcpy(ntyWatchIntance.u8FamilyNumber[0], "18874187429");
		strcpy(ntyWatchIntance.u8FamilyNumber[1], "15889650380");

	}
}

int ntyWatchSave(void) {
	char data[NATTY_WATCH_CONF_SIZE] = {0};
	int len = ntyWatchConfigOrganization(data);

	return ntyWriteFile((CHAR*)NATTY_WATCH_CONF_FILE, data, len);
}

#endif

