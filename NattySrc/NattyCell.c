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

#include "NattyCell.h"
#include "NattyUtils.h"
#include "NattyClientDev.h"
#include "TimerEvents.h"


CellInfo nty_cur_cell_info={0x00};
CellInfo nty_nbr_cell_info[CELL_ITEM_MAX_NUM]={0x00};
U8 nty_cell_nbr_num=0;

//This data array is used for saving sending data.
CellInfo nty_snd_cur_cell_info={0x00};
CellInfo nty_snd_nbr_cell_info[CELL_ITEM_MAX_NUM]={0x00};
U8 nty_snd_cell_nbr_num=0;

U8 flg_lbs_init = 0;

BOOL g_bChangeCell = 0;
U8   g_ucSameLBSItemNum = 0;
U16  g_ucBeyondLitimtDB = 0;//Rf singal change delt.

HANDLE_LOCATION ntyCellEndCb = NULL;

void ntySetCellEndCallback(HANDLE_LOCATION cb) {
	ntyCellEndCb = cb;
}


extern void heartbeat(void);
extern int ntyGetNetworkLevel(void);
static void ntyCellRegRsp(l4c_nbr_cell_info_ind_struct *msg_ptr)
{
	gas_nbr_cell_info_struct cell_info;
	int i;
	//BOOL kwp_udp_send = FALSE;
	static U32 uiSampleTime = 0;
	U8   bNeedCheckWifi = FALSE;
	//U32  uiDeltStepCount = step_count - g_uiSndStepCount;

	if(msg_ptr)
	{
		if (KAL_TRUE == msg_ptr->is_nbr_info_valid)
		{
			memcpy((void *)&cell_info, (void *)(&(msg_ptr->ps_nbr_cell_info_union.gas_nbr_cell_info)), sizeof(gas_nbr_cell_info_struct));
		}
		else
		{
			memset((void *)&cell_info, 0, sizeof(gas_nbr_cell_info_struct));    
		}
#if 0
		if(nty_cur_cell_info.arfcn != cell_info.nbr_meas_rslt.nbr_cells[cell_info.serv_info.nbr_meas_rslt_index].arfcn ||
			nty_cur_cell_info.bsic != cell_info.nbr_meas_rslt.nbr_cells[cell_info.serv_info.nbr_meas_rslt_index].bsic)
		{		
			kwp_udp_send = TRUE;
		}
#endif
		//Only lac or ci has changed, to means that the connecting base station has been changed.
		if (cell_info.serv_info.gci.lac != nty_cur_cell_info.lac
			||  nty_cur_cell_info.ci != cell_info.serv_info.gci.ci)
		{
			int level = ntyGetNetworkLevel();
			nty_printf("change cell");
#if 1
			if (level > LEVEL_TIMECHECK && nty_cur_cell_info.lac != 0) {				
				heartbeat();
			}
#endif
		}

		nty_cur_cell_info.arfcn = cell_info.nbr_meas_rslt.nbr_cells[cell_info.serv_info.nbr_meas_rslt_index].arfcn;
		nty_cur_cell_info.bsic = cell_info.nbr_meas_rslt.nbr_cells[cell_info.serv_info.nbr_meas_rslt_index].bsic;
		nty_cur_cell_info.rxlev = cell_info.nbr_meas_rslt.nbr_cells[cell_info.serv_info.nbr_meas_rslt_index].rxlev;
		nty_cur_cell_info.mcc = cell_info.serv_info.gci.mcc;
		nty_cur_cell_info.mnc = cell_info.serv_info.gci.mnc;
		nty_cur_cell_info.lac = cell_info.serv_info.gci.lac;
		nty_cur_cell_info.ci = cell_info.serv_info.gci.ci;
		nty_cell_nbr_num = cell_info.nbr_cell_num;

		nty_printf("Cur LBS: lac = %d,ci:%d,Signal:%d",nty_cur_cell_info.lac,
					nty_cur_cell_info.ci ,nty_cur_cell_info.rxlev);	
#if 1
		nty_printf("vm_sal_cell_reg_rsp kwp_cell_nbr_num = %d",nty_cell_nbr_num);
		nty_printf("vm_sal_cell_reg_rsp arfcn = %d", nty_cur_cell_info.arfcn);
		nty_printf("vm_sal_cell_reg_rsp bsic = %d", nty_cur_cell_info.bsic);
		nty_printf("vm_sal_cell_reg_rsp rxlev = %d", nty_cur_cell_info.rxlev);
		nty_printf("vm_sal_cell_reg_rsp mcc = %d", nty_cur_cell_info.mcc);
		nty_printf("vm_sal_cell_reg_rsp mnc = %d", nty_cur_cell_info.mnc);
		nty_printf("vm_sal_cell_reg_rsp lac = %d", nty_cur_cell_info.lac);
		nty_printf("vm_sal_cell_reg_rsp ci = %d", nty_cur_cell_info.ci);
#endif
		//kwp_debug_print("LBS:current lac = %d", kwp_cur_cell_info.lac);
		//kwp_debug_print("LBS:current ci = %d", kwp_cur_cell_info.ci);
		//kwp_debug_print("LBS:nbr number %d", cell_info.nbr_cell_num);

		//kwp_debug_print("kwp_cell_nbr_num = %d", kwp_cell_nbr_num);	
		if (cell_info.nbr_cell_num > CELL_ITEM_MAX_NUM) {
			nty_cell_nbr_num = CELL_ITEM_MAX_NUM;
		}
		for (i = 0; i < nty_cell_nbr_num; i++)
		{
			nty_nbr_cell_info[i].arfcn = cell_info.nbr_meas_rslt.nbr_cells[cell_info.nbr_cell_info[i].nbr_meas_rslt_index].arfcn;
			nty_nbr_cell_info[i].bsic = cell_info.nbr_meas_rslt.nbr_cells[cell_info.nbr_cell_info[i].nbr_meas_rslt_index].bsic;
			nty_nbr_cell_info[i].rxlev = cell_info.nbr_meas_rslt.nbr_cells[cell_info.nbr_cell_info[i].nbr_meas_rslt_index].rxlev;
			nty_nbr_cell_info[i].mcc = cell_info.nbr_cell_info[i].gci.mcc;
			nty_nbr_cell_info[i].mnc = cell_info.nbr_cell_info[i].gci.mnc;

			nty_nbr_cell_info[i].lac = cell_info.nbr_cell_info[i].gci.lac;
			nty_nbr_cell_info[i].ci = cell_info.nbr_cell_info[i].gci.ci;

			nty_printf("LBS:nbr[%d]lac = %d,ci:%d,Signal:%d", i,cell_info.nbr_cell_info[i].gci.lac,
					cell_info.nbr_cell_info[i].gci.ci,nty_nbr_cell_info[i].rxlev);
			nty_printf("LBS:nbr[%d] ci = %d", i, cell_info.nbr_cell_info[i].gci.ci);
		}

		if (ntyCellEndCb != NULL) {
			(*ntyCellEndCb)(NULL);
		}
#if 0
		//Power consumption Algorith
		if(ntyGetUtcTime() - uiSampleTime >= kw_config.set_info.uiLocateFreq -5)
		{
			uiSampleTime = ntyGetUtcTime() ;
			bNeedCheckWifi = TRUE;
		}

		if(uiDeltStepCount >= STEP_LIMIT_UPLOAD_THRESHOLD)
		{
			if(kw_config.set_info.uiLocateMode ==  LOCATION_MODE_ACCURATE)
			{
				uiSampleTime = kwp_get_utc_sec() ;
				bNeedCheckWifi = TRUE;
			}							

		}

		//kwp_debug_print("LBS:iSometime:%d,LocMode:%d ,%d", iSometime,kw_config.set_info.uiLocateMode,g_uiUploadLocInfoTime);

		if((kw_config.set_info.uiLocateMode !=  UPLOAD_LOCATION_LBS_DATA) && bNeedCheckWifi)
		{
			U8   j;
			U8   ucSameLBSItemNum = 0;
			U16  ucBeyondLitimtDB = 0;		

			if(kwp_snd_cur_cell_info.lac == kwp_cur_cell_info.lac &&  kwp_cur_cell_info.ci == kwp_snd_cur_cell_info.ci)
			{
				ucSameLBSItemNum++;
				if( kwp_cur_cell_info.rxlev >  kwp_snd_cur_cell_info.rxlev)
					ucBeyondLitimtDB +=(kwp_cur_cell_info.rxlev - kwp_snd_cur_cell_info.rxlev);
				else
					ucBeyondLitimtDB +=(kwp_snd_cur_cell_info.rxlev - kwp_cur_cell_info.rxlev);			
			}

			for( i = 0 ; i < kwp_cell_nbr_num; i++)
			{
				for( j = 0; j < kwp_snd_cell_nbr_num;j++)
				{
					if(kwp_nbr_cell_info[i].mcc == kwp_snd_nbr_cell_info[j].mcc 
						&& kwp_nbr_cell_info[i].mnc == kwp_snd_nbr_cell_info[j].mnc 
						&& kwp_nbr_cell_info[i].lac == kwp_snd_nbr_cell_info[j].lac 
						&& kwp_nbr_cell_info[i].ci == kwp_snd_nbr_cell_info[j].ci)
					{
						ucSameLBSItemNum++;
						if( kwp_nbr_cell_info[i].rxlev >  kwp_snd_nbr_cell_info[j].rxlev)
							ucBeyondLitimtDB +=(kwp_nbr_cell_info[i].rxlev - kwp_snd_nbr_cell_info[j].rxlev);
						else
							ucBeyondLitimtDB +=(kwp_snd_nbr_cell_info[j].rxlev - kwp_nbr_cell_info[i].rxlev);

					}
				}
			}

			// If match this condition ,we may be think user didn't move.
			kwp_debug_print("Cell Scan SameLBS:%d,db:%d,iLocMode:%d,DeltStep:%d,LocFre:%d",
			ucSameLBSItemNum,ucBeyondLitimtDB,kw_config.set_info.uiLocateMode, uiDeltStepCount,kw_config.set_info.uiLocateFreq);		

			if(uiDeltStepCount >= STEP_LIMIT_UPLOAD_THRESHOLD*2)
			{
				if(kw_config.set_info.uiLocateMode >=  LOCATION_MODE_NORMAL)
				{
					kwp_timer_start_at_once(KWP_TIMER_SCAN_WIFI);
					return;
				}							
			}
			else if(uiDeltStepCount >= STEP_LIMIT_UPLOAD_THRESHOLD)
			{
				if(kw_config.set_info.uiLocateMode ==  LOCATION_MODE_ACCURATE)
				{
					kwp_timer_start_at_once(KWP_TIMER_SCAN_WIFI);
					return;
				}							

			}
			else
			{
				//To do , we think that this position is as same as the previous position.
			}

			if(ucSameLBSItemNum >= 5)
			{
				//To do , we think that this position is as same as the previous position.
			}
			else if(ucSameLBSItemNum >= 3 && ucBeyondLitimtDB <= ucSameLBSItemNum*5 )
			{
				//To do 	,we think that this position is as same as the previous position.
			}
			else if(ucSameLBSItemNum >=3)//Move a larger distance
			{
				if(kw_config.set_info.uiLocateMode ==  LOCATION_MODE_ACCURATE)
				{
					kwp_timer_start_at_once(KWP_TIMER_SCAN_WIFI);
					return;
				}	
			}
			else if(ucSameLBSItemNum >= 1)
			{
				if(kw_config.set_info.uiLocateMode >=  LOCATION_MODE_NORMAL)
				{
					kwp_timer_start_at_once(KWP_TIMER_SCAN_WIFI);
					return;
				}	
			}
			else //Move a much larger distance.
			{
				if(kw_config.set_info.uiLocateMode >=  LOCATION_MODE_NORMAL)
				{
					kwp_timer_start_at_once(KWP_TIMER_SCAN_WIFI);
					return;
				}			
			}

		}
		
		if(kwp_udp_send)
		{		
			//vm_cell_info_change();
		}
#endif
	}
}


U8 ntyCellOpen(void) {
    SetProtocolEventHandler(ntyCellRegRsp, MSG_ID_L4C_NBR_CELL_INFO_REG_CNF);
    SetProtocolEventHandler(ntyCellRegRsp, MSG_ID_L4C_NBR_CELL_INFO_IND);
    mmi_frm_send_ilm(MOD_L4C, MSG_ID_L4C_NBR_CELL_INFO_REG_REQ, NULL, NULL);
    flg_lbs_init = 1;
    return 0;
}

U8 ntyCellClose(void) {
    ClearProtocolEventHandler(MSG_ID_L4C_NBR_CELL_INFO_IND);
    ClearProtocolEventHandler(MSG_ID_L4C_NBR_CELL_INFO_REG_CNF);
    mmi_frm_send_ilm(MOD_L4C, MSG_ID_L4C_NBR_CELL_INFO_DEREG_REQ, NULL, NULL);
    flg_lbs_init = 0;
    return 0;
}


U8 ntyGetCurCellInfo(void) {
    mmi_frm_send_ilm(MOD_L4C, MSG_ID_L4C_NBR_CELL_INFO_REG_REQ, NULL, NULL);
    return 0;
}


void ntyLBSStartScan(void) {
	//StopTimer(NATTY_LBSSCAN_TIMER);
	ntyCellOpen();
	//StartTimer(NATTY_LBSSCAN_TIMER, 60*1000, ntyLBSStartScan);
}




