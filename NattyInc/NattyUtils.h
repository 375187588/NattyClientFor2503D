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


#ifndef __NATTY_UTILS_H__
#define __NATTY_UTILS_H__

#include "NattyAbstractClass.h"
#include "app_datetime.h"


#define ITEM_SIZE		64

#define FAMILY_NUMBER_SIZE		2
#define CONTACT_NUMBER_SIZE		10
#define PHONE_BOOK_SIZE			10
#define PHONE_NUMBER_LENGTH		20

#define  NATTY_WATCH_CONF_SIZE		2*1024
#define  NATTY_WATCH_CONF_FILE		L"\\watch.conf"
#define  NATTY_SERVER_IP_FILE		L"\\server.conf"

#define NTY_DEBUG 	1
#if (NTY_DEBUG == 1) //catcher 
#define nty_printf(format, ...) 		kal_prompt_trace(MOD_BT, format, ##__VA_ARGS__)
#elif (NTY_DEBUG == 2) // Serial
#define nty_printf(format, ...)
#elif (NTY_DEBUG == 3) //Log file
#define nty_printf(format, ...)
#else
#define nty_printf(format, ...)
#endif


#define FLAG_COUNTSTEP_MODE		0x0001
#define FLAG_SLEEPQUALITY_MODE	0x0002

#define FLAG_SOSMSG_MODE		0x0004
#define FLAG_POWERMSG_MODE		0x0008
#define FLAG_ALARMMUSIC_MODE	0x0010
#define FLAG_SAFEZONEALARM_MODE	0x0020

#define  NTY_CALL_NUMBER_MAX_LENGTH   20

#define NTY_MSG_CALL_CTRL		0xF0

#define NTY_MALLOC_SIZE_MAX		(1024+24)

typedef struct _WATCHCONF {
	char u8StepFlag;
	char u8SleepFlag;
	char u8SOSMsgFlag;
	char u8PowerMsgFlag;
	char u8AlarmMusicFlag;
	char u8SafeZoneMusic;
	char u8Freq;
	char u8FamilyNumber[FAMILY_NUMBER_SIZE][PHONE_NUMBER_LENGTH];
	char u8ContactNumber[CONTACT_NUMBER_SIZE][PHONE_NUMBER_LENGTH];
	char u8PhoneBook[PHONE_BOOK_SIZE][PHONE_NUMBER_LENGTH];
} ntyWatchConfig;


void ntyU16Datacpy(U8 *send, U16 num);
void ntyU32Datacpy(U8 *send, U32 num);
void ntyU64Datacpy(U8 *send, C_DEVID num);
U32 ntyKMP(const char *text,const U32 text_length, const char *pattern,const U32 pattern_length, U32 *matches);

char ntyIsAvailablePhnum(char *phnum);

typedef applib_time_struct ntyTime;

#if 0
void ntyU8ArrayToU16(U16 *num, U8 *buf);
void ntyU8ArrayToU32(U32 *num, U8 *buf);
void ntyU8ArrayToU64(C_DEVID *num, U8 *buf);
#else
U16 ntyU8ArrayToU16(U8 *buf);
U32 ntyU8ArrayToU32(U8 *buf);
C_DEVID ntyU8ArrayToU64(U8 *buf);
char ntyHexToString(U8 hex, U8 *string);
U8 ntyArrayIsEmpty(U8 *array);

void ASCII2UCS2(U8* src, WCHAR* dest);


#endif


void ntyDevIdDatacpy(U8 *send, U8 *u8DevId);
void ntyDebugPrint(const char *fmt, ...);
char ntyInt2Char(char num);
void *ntyMalloc(U32 size);
void ntyFree(void *ptr);
void *ntyRealloc(void* pVar, int Size);
int ntySeparation(char ch, char *sequence, char ***pChTable, int *Count);
void ntyFreeTable(unsigned char ***table, int count);


int ntyWstrlen(U16 *s);
U8 ntyGetBatteryLevel(void);
U8 ntyGetSignalLevel(void);

void ntySetSystemTime(U8 *buf);
void ntySetDefaultSystemTime(void);
U32 ntyGetUtcSec(void);
U8 ntyLocalTime (const U32 t, S32 offset, struct tm *tp);
void ntySetRtcTime(ntyTime *time);
void ntyGetRtcTime(ntyTime *time);
U32 ntyGetUtcTime(void);


void ntyMMILmcGetICCIDReq(void);
void ntyColdRestart(void);


char ntyCompareGetOpera(const U8 *taken);
char ntyCompareSetOpera(const U8 *taken);


int ntyWatchSave(void);
int ntyWatchConfigInit(void);


char ntyMakeCall(char check,char *number);
void ntyLaunchCall(char *number);
void ntySetVolumeLevelReq(void) ;


#endif


