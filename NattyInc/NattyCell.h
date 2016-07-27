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
//this code from haichao
#ifndef __NATTY_CELL_H__
#define __NATTY_CELL_H__

#include "NattyAbstractClass.h"

#include "mmi_frm_events_gprot.h"
#include "nbr_public_struct.h"

/*cell info structure*/
typedef struct{
    unsigned short arfcn;           /*ARFCN*/
    unsigned char bsic;              /*BSIC*/
    unsigned char rxlev;            /*Received signal level*/
    unsigned short mcc;            /*MCC*/
    unsigned short mnc;            /*MNC*/
    unsigned short lac;              /*LAC*/
    unsigned short ci;                /*CI*/
} CellInfo;


#define CELL_ITEM_MAX_NUM        6

extern CellInfo nty_cur_cell_info;
extern CellInfo nty_nbr_cell_info[CELL_ITEM_MAX_NUM];
extern U8 nty_cell_nbr_num;


void ntySetCellEndCallback(HANDLE_LOCATION cb);
void ntyLBSStartScan(void);


#endif




