/*******************************************************************************
################################################################################
#   Copyright (c) [2017-2019] [Radisys]                                        #
#                                                                              #
#   Licensed under the Apache License, Version 2.0 (the "License");            #
#   you may not use this file except in compliance with the License.           #
#   You may obtain a copy of the License at                                    #
#                                                                              #
#       http://www.apache.org/licenses/LICENSE-2.0                             #
#                                                                              #
#   Unless required by applicable law or agreed to in writing, software        #
#   distributed under the License is distributed on an "AS IS" BASIS,          #
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
#   See the License for the specific language governing permissions and        #
#   limitations under the License.                                             #
################################################################################
*******************************************************************************/

/********************************************************************20**
  
     Name:    LTE-RLC Layer - Lower Interface Functions
    
     Type:    C file
  
     Desc:     Source code for RLC Lower Interface Module.
               This file contains following functions
      
        --KwLiRguBndCfm
        --KwLiRguCDatInd
        --KwLiRguDDatInd
        --KwLiRguCStaInd
        --KwLiRguDStaInd
        --KwLiRguHqStaInd

     File:     kw_lim.c

**********************************************************************/
static const char* RLOG_MODULE_NAME="LIM";
static int RLOG_MODULE_ID=2048;
static int RLOG_FILE_ID=196;

/** 
 * @file kw_lim.c
 * @brief RLC Lower Interface module
*/

#define KW_MODULE KW_DBGMASK_INF


/* header (.h) include files */
#include "envopt.h"        /* environment options */
#include "envdep.h"        /* environment dependent */
#include "envind.h"        /* environment independent */

#include "gen.h"           /* general */
#include "ssi.h"           /* system services */
#include "cm5.h"           /* common timer defines */
#include "cm_tkns.h"       /* common tokens defines */
#include "cm_mblk.h"       /* common memory allocation library defines */
#include "cm_llist.h"      /* common link list  defines  */
#include "cm_hash.h"       /* common hash list  defines */
#include "cm_lte.h"        /* common LTE defines */
#include "lkw.h"           /* LKW defines */
#include "ckw.h"           /* CKW defines */
#include "kwu.h"           /* KWU defines */
#include "rgu.h"           /* RGU defines */
#include "kw_err.h"
#include "kw_env.h"        /* RLC environment options */

#include "kw.h"            /* RLC defines */
#include "kw_udx.h"
#include "kw_dl.h"
#include "kw_ul.h"

/* extern (.x) include files */
#include "gen.x"           /* general */
#include "ssi.x"           /* system services */

#include "cm5.x"           /* common timer library */
#include "cm_tkns.x"       /* common tokens */
#include "cm_mblk.x"       /* common memory allocation */
#include "cm_llist.x"      /* common link list */
#include "cm_hash.x"       /* common hash list */
#include "cm_lte.x"        /* common LTE includes */
#include "cm_lib.x"        /* common memory allocation library */
#include "lkw.x"           /* LKW */
#include "ckw.x"           /* CKW */
#include "kwu.x"           /* KWU */
#include "rgu.x"           /* RGU */

#include "kw.x"
#include "kw_udx.x"
#include "kw_dl.x"
#include "kw_ul.x"

#ifdef __cplusplus
EXTERN "C" {
#endif /* __cplusplus */


/*****************************************************************************
 *                          RGU INTERFACE
 ****************************************************************************/
/**
 * @brief Handler for bind confirmation from MAC.
 *
 * @details
 *    This function handles the bind confirmation received from MAC. If the 
 *    bind was successful changes the state of the SAP to KW_SAP_BND 
 *    else KW_SAP_CFG. Sends an alarm to LM in any case
 *
 * @param[in] pst     Post structure
 * @param[in] suId    Service User ID
 * @param[in] status  Status whether the bind was successful or not
 *
 * @return  S16
 *    -# ROK 
 *    -# RFAILED 
 *
*/
#ifdef ANSI
PUBLIC S16 KwLiRguBndCfm
(
Pst    *pst,   
SuId   suId, 
U8     status 
)
#else
PUBLIC S16 KwLiRguBndCfm (pst, suId, status)
Pst    *pst; 
SuId   suId;
U8     status;
#endif
{
   U16          event;     /* Event */
   U16          cause;     /* Cause */
   KwRguSapCb   *rguSap;   /* RGU SAP Control Block */
   KwCb         *tKwCb;

   TRC3(KwLiRguBndCfm)

#if (ERRCLASS & ERRCLS_INT_PAR)
   if (pst->dstInst >= KW_MAX_RLC_INSTANCES)
   {
      RETVALUE (RFAILED);
   }
#endif
   tKwCb = KW_GET_KWCB(pst->dstInst);

   RLOG2(L_DEBUG,"KwLiRguBndCfm(suId(%d), status(%d)", suId, status);

#if (ERRCLASS & ERRCLS_INT_PAR)
   if (tKwCb->init.cfgDone != TRUE)
   {
      RLOG0(L_FATAL,"General configuration not done");
      
      KW_SEND_SAPID_ALARM(tKwCb,suId,LKW_EVENT_LI_BND_CFM, LCM_CAUSE_INV_STATE);

      RETVALUE(RFAILED);
   }

   if ((suId >= tKwCb->genCfg.maxRguSaps) || (suId < 0))
   {
      RLOG0(L_ERROR, "Invalid suId");
      
      KW_SEND_SAPID_ALARM(tKwCb,suId, LKW_EVENT_LI_BND_CFM, LCM_CAUSE_INV_SUID);

      RETVALUE(RFAILED);
   }
#endif /* ERRCLASS & ERRCLS_INT_PAR */

   rguSap = (tKwCb->genCfg.rlcMode == LKW_RLC_MODE_DL) ?
            &(tKwCb->u.dlCb->rguDlSap[suId]) : &(tKwCb->u.ulCb->rguUlSap[suId]);

   RLOG1(L_DEBUG, "KwLiRguBndCfm: For RGU SAP state=%d", rguSap->state)

   switch (rguSap->state)
   {
      case KW_SAP_BINDING:
      {
         kwStopTmr (tKwCb,(PTR)rguSap, KW_EVT_WAIT_BNDCFM);

         rguSap->retryCnt = 0;
          
         if (status == CM_BND_OK)
         {
            rguSap->state = KW_SAP_BND;
            event = LCM_EVENT_BND_OK;
            cause = LKW_CAUSE_SAP_BNDENB;
         }
         else
         {
            rguSap->state = KW_SAP_CFG;
            event = LCM_EVENT_BND_FAIL;
            cause = LKW_CAUSE_UNKNOWN;
         }

         break;
      }
      default:
         event = LKW_EVENT_RGU_BND_CFM;
         cause = LCM_CAUSE_INV_STATE;
         break;
   }

   /* Send an alarm with proper event and cause */
   KW_SEND_SAPID_ALARM(tKwCb, suId, event, cause);

   RETVALUE(ROK);
} /* KwLiRguBndCfm */

PUBLIC int   rlcDDatIndRcvd;
PUBLIC int   rlcCDatIndRcvd;
/**
 * @brief Handler to process PDU received from MAC for common logical channels. 
 *
 * @details
 *    This function receives the PDU from MAC for common logical channels
 *    does checks before handing over the PDU to the TM module
 *
 * @param[in] pst     Post structure
 * @param[in] suId    Service User ID
 * @param[in] datInd  Data Indication Information 
 *
 * @return  S16
 *    -# ROK 
 *    -# RFAILED 
 *
*/
#ifdef ANSI
PUBLIC S16 KwLiRguCDatInd
(
Pst              *pst,   
SuId             suId,  
RguCDatIndInfo   *datInd
)
#else
PUBLIC S16 KwLiRguCDatInd(pst,suId,datInd)
Pst              *pst; 
SuId             suId;
RguCDatIndInfo   *datInd;
#endif
{
   KwUlRbCb   *rbCb; 
   KwCb       *tKwCb; 

   rlcCDatIndRcvd++;
   TRC3(KwLiRguCDatInd)

#if (ERRCLASS & ERRCLS_INT_PAR)
   if (pst->dstInst >= KW_MAX_RLC_INSTANCES)
   {
      KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, datInd, sizeof(RguCDatIndInfo));
      RETVALUE (RFAILED);
   }
#endif

   tKwCb = KW_GET_KWCB(pst->dstInst);


#if (ERRCLASS & ERRCLS_DEBUG)
   if (tKwCb->genCfg.rlcMode == LKW_RLC_MODE_DL)
   {
      KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, datInd, sizeof(RguCDatIndInfo));
      RETVALUE(RFAILED);
   }
#endif

   /* kw006.201 ccpu00120058, Added array boundary condition check */
#if (ERRCLASS & ERRCLS_DEBUG)
   if(KW_MAX_LCH_PER_CELL <= datInd->lcId)
   {
      RLOG_ARG1(L_ERROR,DBG_LCID,datInd->lcId, "Invalid LcId, Max is [%d]",
         KW_MAX_LCH_PER_CELL);
      KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, datInd, sizeof(RguCDatIndInfo));
      RETVALUE(RFAILED);
   }
#endif /* (ERRCLASS & ERRCLS_DEBUG) */

   /* Fetch RbCb from lcId */
   kwDbmFetchUlRbCbFromLchId(tKwCb, 0, datInd->cellId, datInd->lcId, &rbCb);
   if (!rbCb)
   {
      RLOG_ARG1(L_ERROR, DBG_CELLID,datInd->cellId, "LcId [%d] not found",
         datInd->lcId);
      KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, datInd, sizeof(RguCDatIndInfo));
      RETVALUE(RFAILED);
   }

   /* Dispatch to TM Module */
#ifdef CCPU_OPT
   kwTmmRcvFrmLi(tKwCb, rbCb, datInd->rnti, datInd->pdu);
#else /* CCPU_OPT */
   kwTmmRcvFrmLi(tKwCb, rbCb, datInd->pdu);
#endif /* CCPU_OPT */

   KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, datInd, sizeof(RguCDatIndInfo));

   RETVALUE(ROK);
} /* KwLiRguCDatInd */

/**
 * @brief Handler to process PDU received from MAC for 
 *        dedicated logical channels. 
 *
 * @details
 *    This function receives the PDU from MAC for one or more dedicated 
 *    logical channels and passes it to the UTL module for further processing
 *
 * @param[in] pst     Post structure
 * @param[in] suId    Service User ID
 * @param[in] datInd  Data Indication Information 
 *
 * @return  S16
 *    -# ROK 
 *    -# RFAILED 
 *
*/
 
#ifdef ANSI
PUBLIC S16 KwLiRguDDatInd
(
Pst              *pst,   
SuId             suId,  
RguDDatIndInfo   *datInd
)
#else
PUBLIC S16 KwLiRguDDatInd(pst, suId, datInd)
Pst              *pst; 
SuId             suId;
RguDDatIndInfo   *datInd;
#endif
{
   TRC3(KwLiRguDDatInd)

   rlcDDatIndRcvd++;
#if (ERRCLASS & ERRCLS_INT_PAR)
   if (pst->dstInst >= KW_MAX_RLC_INSTANCES)
   {
       KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, datInd, sizeof(RguDDatIndInfo));
       RETVALUE (RFAILED);
   }
#endif

#if (ERRCLASS & ERRCLS_DEBUG)
   if (((KwCb*)KW_GET_KWCB(pst->dstInst))->genCfg.rlcMode == LKW_RLC_MODE_DL)
   {
       RLOG1(L_DEBUG,"KwLiRguDDatInd(pst, suId(%d))recieved in DL Inst",suId);
       KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, datInd, sizeof(RguDDatIndInfo));
       RETVALUE(RFAILED);
   }
#endif
   kwUtlRcvFrmLi(KW_GET_KWCB(pst->dstInst),datInd);
#ifndef SS_RBUF 
#ifdef SS_LOCKLESS_MEMORY
   KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, datInd, sizeof(RguDDatIndInfo));
#else
   KW_PST_FREE(pst->region, pst->pool, datInd, sizeof(RguDDatIndInfo));
#endif
#endif

   RETVALUE(ROK);
} /* KwLiRguDDatInd */

/**
 * @brief Handler for trigerring the data transfer from RLC to MAC
 *        for common logical channels.
 *
 * @details
 *    This function receives the size of the PDU to be transmitted
 *    and acts as a trigger for forming PDU and sending it to MAC. 
 *
 * @param[in] pst       Post structure
 * @param[in] suId      Service User ID
 * @param[in] staInd    Status Indication Information for Common Logical 
 *                      Channels
 *
 * @return  S16
 *    -# ROK 
 *    -# RFAILED 
 *
*/ 
#ifdef ANSI
PUBLIC S16 KwLiRguCStaInd
(
Pst              *pst,  
SuId             suId,
RguCStaIndInfo   *staInd  
)
#else
PUBLIC S16 KwLiRguCStaInd(pst,suId,staInd)
Pst              *pst;   
SuId             suId; 
RguCStaIndInfo   *staInd; 
#endif
{
   KwDlRbCb   *rbCb;  
   KwCb       *tKwCb;

   TRC3(KwLiRguCStaInd)

#if (ERRCLASS & ERRCLS_INT_PAR)
   if (pst->dstInst >= KW_MAX_RLC_INSTANCES)
   {
      KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, staInd, sizeof(RguCStaIndInfo));
      RETVALUE (RFAILED);
   }
#endif

   tKwCb = KW_GET_KWCB(pst->dstInst);


#if (ERRCLASS & ERRCLS_INT_PAR)
   if ((suId >= tKwCb->genCfg.maxRguSaps) || (suId < 0))
   {
      KWLOGERROR(tKwCb,
            ERRCLS_INT_PAR, 
            EKW040, 
            (ErrVal) suId,
            "KwLiRguCStaInd: Invalid RGU suId\n");
      RETVALUE(RFAILED); 
   }
   if (tKwCb->genCfg.rlcMode == LKW_RLC_MODE_UL)
   {
       RLOG_ARG1(L_ERROR,DBG_LCID,staInd->lcId,
             "Received in RLC UL CELLID:%d",
             staInd->cellId);
       KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, staInd, sizeof(RguCStaIndInfo));
       RETVALUE(RFAILED);
   }
#endif

   rbCb = NULLP;

   /* kw006.201 ccpu00120058, added boundary condition check */
#if (ERRCLASS & ERRCLS_DEBUG)
   if(KW_MAX_LCH_PER_CELL < staInd->lcId)
   {
      RLOG_ARG2(L_ERROR,DBG_LCID,staInd->lcId, 
            "Invalid LcId, Max is [%d] CELLID:%d",
            KW_MAX_LCH_PER_CELL,
            staInd->cellId);
      KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, staInd, sizeof(RguCStaIndInfo));
      RETVALUE(RFAILED);
   }
#endif /* (ERRCLASS & ERRCLS_DEBUG) */
   /* Fertch RbCb from lcId */
   kwDbmFetchDlRbCbFromLchId(tKwCb,0, staInd->cellId, staInd->lcId, &rbCb);
   if(!rbCb)                                               
   {
      RLOG_ARG1(L_ERROR, DBG_CELLID,staInd->cellId, 
            "LcId [%d] not found CELLID:%d",
            staInd->lcId);
      KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, staInd, sizeof(RguCStaIndInfo));
      RETVALUE(RFAILED);
   }

   /* Dispatch to TM Module */
   rbCb->transId = staInd->transId;
   /* ccpu00136940 */
  /* If trace flag is enabled send the trace indication */
   if(tKwCb->init.trc == TRUE)
   {
     /* Populate the trace params */
      kwLmmSendTrc(tKwCb,EVTRGUCSTAIND, NULLP);
   }                            
   kwTmmSndToLi(tKwCb, suId, rbCb, staInd);
#ifndef SS_RBUF
#ifdef SS_LOCKLESS_MEMORY
   KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, staInd, sizeof(RguCStaIndInfo));
#else
   KW_PST_FREE(pst->region, pst->pool, staInd, sizeof(RguCStaIndInfo));
#endif
#else
   KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, staInd, sizeof(RguCStaIndInfo));
#endif
   RETVALUE(ROK);
} /* KwLiRguCStaInd */

/**
 * @brief Handler for trigerring the data transfer from RLC to MAC
 *        for dedicated logical channels.
 *
 * @details
 *    This function receives the size of the PDUs to be transmitted to
 *    MAC via one or more dedicated logical channels and acts as a trigger
 *    for forming PDUs and sending them to MAC. 
 *
 * @param[in] pst       Post structure
 * @param[in] suId      Service User ID
 * @param[in] staInd    Status Indication Information for Dedicated Logical 
 *                      Channels
 *
 * @return  S16
 *    -# ROK 
 *    -# RFAILED 
 *
*/ 
#ifdef ANSI
PUBLIC S16 KwLiRguDStaInd
(
Pst              *pst,  
SuId             suId,
RguDStaIndInfo   *staInd 
)
#else
PUBLIC S16 KwLiRguDStaInd(pst, suId, staInd)
Pst              *pst; 
SuId             suId; 
RguDStaIndInfo   *staInd; 
#endif
{
   KwCb        *gCb;
   TRC3(KwLiRguDStaInd)

#if (ERRCLASS & ERRCLS_INT_PAR)
   if (pst->dstInst >= KW_MAX_RLC_INSTANCES)
   {
      KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, staInd, sizeof(RguDStaIndInfo));
      RETVALUE (RFAILED);
   }
#endif

   gCb = KW_GET_KWCB(pst->dstInst);

#if (ERRCLASS & ERRCLS_INT_PAR)
   if (((KwCb*)KW_GET_KWCB(pst->dstInst))->genCfg.rlcMode == LKW_RLC_MODE_UL)
   {
       RLOG_ARG0(L_ERROR,DBG_CELLID,staInd->cellId,"Received in RLC UL ");
       KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, staInd, sizeof(RguDStaIndInfo));
       RETVALUE(RFAILED);
   }
   if ((suId >= gCb->genCfg.maxRguSaps) || (suId < 0))
   {
      KWLOGERROR(gCb,
            ERRCLS_INT_PAR, 
            EKW040, 
            (ErrVal) suId,
            "KwLiRguDStaInd: Invalid RGU suId\n");
      RETVALUE(RFAILED); 
   }
#endif
   kwUtlSndToLi(gCb, suId, staInd);

   /* kw002.201 :Freeing from proper region */
   KW_SHRABL_STATIC_BUF_FREE(pst->region, pst->pool, staInd, sizeof(RguDStaIndInfo));
   RETVALUE(ROK);
} /* KwLiRguDStaInd */

/**
 * @brief Handler for handling the flow cntrl Ind from MAC
 *  to RLC      
 *
 * @details
 *   This function receives the flow control indication from
 *   MAC and calls kwUtlTrigPdbFlowCntrl
 *
 * @param[in] pst       Post structure
 * @param[in] suId      Service User ID
 * @param[in] flowCntrlInd flow control Indication Information 
 *           from MAC  
 *
 * @return  S16
 *    -# ROK 
 *    -# RFAILED 
 *
*/ 
#ifdef ANSI
PUBLIC S16 KwLiRguFlowCntrlInd
(
Pst              *pst,  
SuId             suId,
RguFlowCntrlInd *flowCntrlInd
)
#else
PUBLIC S16 KwLiRguFlowCntrlInd(pst, suId, flowCntrlInd)
Pst              *pst; 
SuId             suId; 
RguFlowCntrlInd  *flowCntrlInd; 
#endif
{
   KwCb       *tKwCb; 
   KwDlRbCb   *rbCb = NULLP;
   U32        idx;
   U32        lcIdx;

   tKwCb = KW_GET_KWCB(pst->dstInst);
   for (idx = 0; idx < flowCntrlInd->numUes; idx++)
   {
      for (lcIdx = 0; lcIdx < flowCntrlInd->ueFlowCntrlInfo[idx].numLcs; lcIdx++)  
      {
         RguLcFlowCntrlInfo *lcInfo = &(flowCntrlInd->ueFlowCntrlInfo[idx].lcInfo[lcIdx]); 
         kwDbmFetchDlRbCbFromLchId(tKwCb, flowCntrlInd->ueFlowCntrlInfo[idx].ueId, flowCntrlInd->cellId, lcInfo->lcId, &rbCb);
         if (rbCb)
         {
           
            if (lcInfo->pktAdmitCnt == 0) /* Special case */
            {
               kwUtlTrigPdbFlowCntrl(tKwCb, rbCb, lcInfo->pktAdmitCnt);
               continue;
            }
            if (rbCb->mode == CM_LTE_MODE_AM)
            {
               if ((rbCb->m.amDl.retxLst.count != 0) || 
                   ((rbCb->m.amDl.bo == 0) || 
                    (rbCb->m.amDl.bo < lcInfo->maxBo4FlowCtrl)))
               {
                  continue;
               }
            }
            else /* UM */
            {
               if ((rbCb->m.umDl.bo == 0) ||
                   (rbCb->m.umDl.bo < lcInfo->maxBo4FlowCtrl))
               {
                  continue;
               }
            }
            kwUtlTrigPdbFlowCntrl(tKwCb, rbCb, lcInfo->pktAdmitCnt);
         }
      }
   }
   RETVALUE(ROK);
}
/* kw005.201 added support for L2 Measurement */
#ifdef LTE_L2_MEAS

/**
 *
 * @brief  
 *
 *        Handler for indicating the Harq Status of the data sent.
 *
 * @b Description:
 *
 *        This function receives the harq status of the data sent to MAC.
 *        This information is used for two things.
 *        1. Computing the UuLoss of UM
 *        2. Computing the DL Delay for UM and AM.
 *
 *  @param[in] pst     -   Post structure  
 *  @param[in] suId    -   Service User ID
 *  @param[in] staInd  -   Harq Status Indication Information. 
 *
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED
 *
 */
#ifdef ANSI
PUBLIC S16 KwLiRguHqStaInd
(
Pst                *pst, 
SuId               suId,
RguHarqStatusInd   *staInd 
)
#else
PUBLIC S16 KwLiRguHqStaInd(pst,suId,staInd)
Pst                *pst;  
SuId               suId; 
RguHarqStatusInd   *staInd;
#endif
{

   KwUeKey   ueKey;
   S16       ret; 
   KwDlUeCb    *ueCb;
   U8        tbIdx;
   KwCb       *tKwCb; 

   TRC3(KwLiRguHqStaInd)

   tKwCb = KW_GET_KWCB(pst->dstInst);
   ueKey.cellId = staInd->cellId;
   ueKey.ueId   = staInd->ueId;

   ret = kwDbmFetchDlUeCb(tKwCb, ueKey.ueId, ueKey.cellId, &ueCb);
   if (ret != ROK )
   {
      RETVALUE(RFAILED);
   }

   /*Call kwUtlProcHarqInd as many times as number of Tbs present*/
   for ( tbIdx = 0; tbIdx < staInd->numTbs; tbIdx++)
   {
      kwUtlProcHarqInd(tKwCb, staInd, ueCb, tbIdx);
   }

   RETVALUE(ROK);
} /* KwLiRguHqStaInd */
#endif /* LTE_L2_MEAS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

/********************************************************************30**
         End of file
**********************************************************************/
