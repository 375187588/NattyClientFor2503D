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

#ifndef __NATTY_NODE_AGENT_H__
#define __NATTY_NODE_AGENT_H__

#include "string.h"

typedef struct NTYNODE {
	const char *name;
	int length;
} ntyNode;
#if 0
const char *ntyNode_Power = "Power";
const char *ntyNode_Signal = "Signal";
const char *ntyNode_PhoneBook = "PhoneBook";
const char *ntyNode_FamilyNumber = "FamilyNumber";
const char *ntyNode_Fallen = "Fallen";
const char *ntyNode_GPS = "GPS";
const char *ntyNode_Wifi = "Wifi";
const char *ntyNode_Lab = "Lab";
#endif

const ntyNode ntyNodeTable[] = {
	{"Power", 5},
	{"Signal", 6},
	{"PhoneBook", 9},
	{"FamilyNumber", 12},
	{"Fallen", 6},
	{"GPS", 3},
	{"Wifi", 4},
	{"Lab", 3},
};


enum ntyNode_Database {
	DB_NODE_START = 0,
	DB_NODE_POWER = DB_NODE_START,
	DB_NODE_SIGNAL,
	DB_NODE_PHONEBOOK,
	DB_NODE_FAMILYNUMBER,
	DB_NODE_FALLEN,
	DB_NODE_GPS,
	DB_NODE_WIFI,
	DB_NODE_LAB,
	DB_NODE_END = DB_NODE_LAB,
};


int ntyNodeCompare(const unsigned char *taken) {
	int node = 0;

	for (node = DB_NODE_START;node < DB_NODE_END;node ++) {
		if (!strcmp((const char*)taken, (const char*)ntyNodeTable[node].name)) {
			return node;
		}
	}
	return -1;
}


#endif


