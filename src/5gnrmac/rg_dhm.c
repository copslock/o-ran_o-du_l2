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

/************************************************************************
 
     Name:     LTE-MAC layer
  
     Type:     C source file
  
     Desc:     C source code for Entry point fucntions
  
     File:     rg_dhm.c
  
**********************************************************************/

/** @file rg_dhm.c
@brief APIs related to Downlink HARQ.
*/

static const char* RLOG_MODULE_NAME="MAC";
static int RLOG_FILE_ID=279;
static int RLOG_MODULE_ID=4096;
/* header include files -- defines (.h) */
#include "envopt.h"        /* environment options */
#include "envdep.h"        /* environment dependent */
#include "envind.h"        /* environment independent */
#include "gen.h"           /* general layer */
#include "ssi.h"           /* system service interface */
#ifdef L2_OPTMZ
#include "ss_strm.h"
#endif
#include "cm5.h"           /* common timers */
#include "cm_hash.h"       /* common hash list */
#include "cm_mblk.h"       /* common memory link list library */
#include "cm_llist.h"      /* common linked list library */
#include "cm_err.h"        /* common error */
#include "cm_lte.h"        /* common LTE */
#include "lrg.h"
#include "crg.h"
#include "rgu.h"
#include "tfu.h"
#include "rg_sch_inf.h"
#include "rg_env.h"
#include "rg_err.h"
#include "rg.h"

/* header/extern include files (.x) */
#include "gen.x"           /* general layer */
#include "ssi.x"           /* system service interface */
#ifdef L2_OPTMZ
#include "ss_strm.x"
#endif
#include "cm5.x"           /* common timers */
#include "cm_lib.x"        /* common library */
#include "cm_hash.x"       /* common hash list */
#include "cm_llist.x"      /* common linked list library */
#include "cm_mblk.x"       /* memory management */
#include "cm_tkns.x"       /* common tokens */
#include "cm_lte.x"        /* common LTE */
#include "lrg.x"
#include "crg.x"
#include "rgu.x"
#include "tfu.x"
#include "rg_sch_inf.x"
#include "rg_prg.x"        /* PRG interface typedefs */
#include "rg.x"

#ifdef L2_OPTMZ
#include "ss_queue.h"
#include "ss_queue.x"
#include "ss_task.x"
#include "ss_msg.x"            /* MAC includes */
/* local defines */
//EXTERN  S16 ssGetDBufOfSize(Region region,Size size,Buffer **dBuf);
//void prc_trace_format_string(UINT32 group_mask, UINT16 level, const char *format, ...);
#endif

/* local typedefs */
 
/* local externs */
EXTERN S16 SIncMsgRef(Buffer *srcBuf,Region dstRegion, Pool dstPool,Buffer **dstBuf);

PRIVATE Void rgDHMBldTfuDatReq ARGS((RgCellCb *cellCb, RgDlSf *dlSf, RgDlHqProcCb *hqP,
                           RgTfuDatReqPduInfo *datReq));

#ifdef L2_OPTMZ
PUBLIC S16 rgDHMFreeHqProcTB
(
RgDlHqProcCb         *hqP,
U8                   tbIndex
);

#endif

/* forward references */

/**
 * @brief This function initializes the DL HARQ Entity of UE
 *
 * @details
 *
 *     Function: rgDHMHqEntInit
 *     Purpose:  This function initializes the DL HARQ entity of 
 *               UE control block. This is performed at the time
 *               of creating UE control block.
 *     
 *     Invoked by: configuration module
 *     
 *  @param[in]  Inst        inst
 *  @param[in]  RgCellCb*  cell
 *  @param[in]  RgUeCb*    ue
 *  @return  S16
 *           -# ROK
 *           -# RFAILED
 *
 **/
#ifdef ANSI
PUBLIC S16 rgDHMHqEntInit
(
Inst               inst,
RgDlHqEnt          *hqE,
U8                 maxHqProcs
)
#else
PUBLIC S16 rgDHMHqEntInit(inst,hqE, maxHqProcs)
Inst               inst,
RgDlHqEnt          *hqE;
U8                 maxHqProcs;
#endif
{
   U8 idx1,idx2;
#ifdef L2_OPTMZ
   Buffer  *hdrDBuf = NULLP;
   Buffer  *ceDBuf = NULLP;
#endif

   TRC2(rgDHMHqEntInit)

   hqE->numHqProcs = maxHqProcs;
   /* for each harq process */
   for (idx1 = 0; idx1 < hqE->numHqProcs; idx1++)
   {
      if (rgAllocSBuf(inst,(Data **)&(hqE->procs[idx1]),sizeof(RgDlHqProcCb)) != ROK) 
      {
         while(idx1--)
         {
            rgFreeSBuf(inst,(Data **)&(hqE->procs[idx1]), sizeof(RgDlHqProcCb));
         }
         RLOG0(L_ERROR, "Memory Alloc Failure for RgDlHqProcCb");        
         RETVALUE(RFAILED);
      }

      hqE->procs[idx1]->procId      = idx1;
      for(idx2 = 0; idx2 < RG_MAX_TB_PER_UE; idx2++)
      {
#ifndef L2_OPTMZ
         hqE->procs[idx1]->tbInfo[idx2].tb = NULLP;
#else
         Buffer *tmpMBuf;
         /* L2 optimization for mUe/Tti: Allocating buffers for macHdr, macCes
          * and macPadding. These buffers shall not be released by MAC/CL.
          * However, Only rPtr and wPtr will be reset while release of hq proc
          */
         tmpMBuf = hqE->procs[idx1]->tbInfo[idx2].tb.macHdr;
         rgGetMsg(inst, &tmpMBuf);
         RG_ADD_DBuf(hdrDBuf, RG_MAC_HDR_SIZE, tmpMBuf);
         hqE->procs[idx1]->tbInfo[idx2].tb.macHdr = tmpMBuf;
         macHeader[idx2] = MacPtrAddress;

         tmpMBuf = hqE->procs[idx1]->tbInfo[idx2].tb.macCes;
         rgGetMsg(inst, &tmpMBuf);
         RG_ADD_DBuf(ceDBuf, RG_MAC_CE_SIZE, tmpMBuf);
         hqE->procs[idx1]->tbInfo[idx2].tb.macCes = tmpMBuf;

         hqE->procs[idx1]->tbInfo[idx2].tb.padSize = 0;
#endif
#ifdef LTE_L2_MEAS
         hqE->procs[idx1]->tbId[idx2] = RGU_INVALID_TBID;
#endif
      }

      cmLListInit(&hqE->savedProcLst[idx1]);
   }


   RETVALUE(ROK);
} /* rgDHMHqEntInit */

/**
 * @brief This function releases a HARQ process
 *
 * @details
 *
 *     Function: rgDHMUeReset
 *     Purpose:  This function resets TB in each HarqProc.
 *     
 *     Invoked by: CFG UE Reset 
 *     
 *  @param[in]  RgDlHqProc    *hqP
 *  @return  Void      
 *
 **/
#ifdef ANSI
PUBLIC Void rgDHMUeReset
(
RgCellCb *cell,
RgDlHqEnt          *hqE
)
#else
PUBLIC Void rgDHMUeReset(cell, hqE)
RgCellCb *cell;
RgDlHqEnt          *hqE;
#endif
{
   U8       i = 0;

   TRC2(rgDHMUeReset)

   if(hqE->procs[0])
   {
      /* Free all the TB memory associated with HARQ */
      for (i=0; i < hqE->numHqProcs; i++)
      {
         rgDHMRlsHqProcTB(cell, hqE->procs[i], 1);
         rgDHMRlsHqProcTB(cell, hqE->procs[i], 2);

#ifdef LTE_ADV
         rgDHMFreeSavedHqP((cell->macInst - RG_INST_START), hqE, i);
#endif
      }
   }
   RETVOID;
} /* rgDHMUeReset*/

/**
 * @brief This function defers shared memory buffer
 *        freeing out of the critical RT path.
 *
 * @details
 *
 *     Function: rgDHMHdlBufFree
 *     Purpose: To defer shared memory freeing post 
 *              critical path. Defer as many if defer queue 
 *              is full then release instantly.
 *     
 *     Invoked by: HARQ TB Release. 
 *     
 *  @return  Void      
 *
 **/
#ifdef ANSI
PUBLIC Void rgDHMHdlBufFree
(
Inst inst,
Buffer **mBuf
)
#else
PUBLIC Void rgDHMHdlBufFree(Inst inst, Buffer **mBuf)
Inst inst;
#endif
{
   RgCb *rgCbP = &rgCb[inst];
   TRC2(rgDHMHdlBufFree)

   if (rgCbP->bufCnt < RG_MAX_DFRD_FREE_BUFS)
   {
      if (*mBuf)
      {
         rgCbP->bufToFree[rgCbP->bufCnt] = *mBuf;
         rgCbP->bufCnt++;
         *mBuf = NULLP;
      }
   }
   else
   {
      RG_FREE_MSG(*mBuf);
   }
   RETVOID;
}
/**
 * @brief This function is called to release the 
 *        shared memory of the HARQ TBs outside 
 *        the critical RT path.
 *
 * @details
 *
 *     Function: rgDHMFreeTbBufs
 *     Purpose: This function is called to release the 
 *        shared memory of the HARQ TBs outside 
 *        the critical RT path.
 *     
 *     1. Job of releasing TBs is shared across TTIs
 *     Invoked by: MAC every TTI 
 *     
 *  @return  Void      
 *
 **/
#ifdef ANSI
PUBLIC Void rgDHMFreeTbBufs
(
Inst inst
)
#else
PUBLIC Void rgDHMFreeTbBufs(inst)
Inst inst;
#endif
{
   RgCb *rgCbP = &rgCb[inst];
   U8 start = rgCbP->bufCnt;
   U8 end = 0;

   TRC2(rgDHMFreeTbBufs)

   if (rgCbP->bufCnt < RG_MAX_FREE_BUFS_PERTTI)
   {
      end = 0;
   }
   else
   {
      end = rgCbP->bufCnt - RG_MAX_FREE_BUFS_PERTTI;
   }
   while (start != end)
   {
      start--;
      SPutMsg(rgCbP->bufToFree[start]);
   }
   rgCbP->bufCnt = end;
   RETVOID;
} /* rgDHMFreeTbBufs */

#ifdef ANSI
PUBLIC Void rgDHMFreeAllTbBufs
(
Inst inst
)
#else
PUBLIC Void rgDHMFreeAllTbBufs(inst)
Inst inst;
#endif
{
   RgCb *rgCbP = &rgCb[inst];
   U8 start = rgCbP->bufCnt;
   U8 end = 0;

   TRC2(rgDHMFreeAllTbBufs)

   while (start != end)
   {
      start--;
      SPutMsg(rgCbP->bufToFree[start]);
   }
   rgCbP->bufCnt = end;
   RETVOID;
} /* rgDHMFreeTbBufs */


/**
 * @brief This function releases a HARQ process
 *
 * @details
 *
 *     Function: rgDHMRlsHqProcTB
 *     Purpose:  This function returns a HARQ process to HARQ Entity 
 *               in the DL direction.
 *     
 *               1. Add the HARQ process to the free queue.
 *     Invoked by: scheduler and HARQ processing
 *     
 *  @param[in]  RgDlHqProc    *hqP
 *  @return  Void      
 *
 **/
#ifdef ANSI
PUBLIC S16 rgDHMRlsHqProcTB
(
RgCellCb             *cell,
RgDlHqProcCb         *hqP,
U8                   tbIndex
)
#else
PUBLIC S16 rgDHMRlsHqProcTB(cell, hqP, tbIndex)
RgCellCb             *cell;
RgDlHqProcCb         *hqP;
U8                   tbIndex;
#endif
{
    U8                    idx;
#ifdef L2_OPTMZ
    RgTfuDatReqTbInfo     *tb;   /* TB to be sent to CL/PHY*/
   // U32 lchIdx, pduIdx;
#endif

   TRC2(rgDHMRlsHqProcTB)

   if((tbIndex > RG_MAX_TB_PER_UE) ||
      (tbIndex == 0))
   {
      RETVALUE(RFAILED);
   }

   hqP->tbInfo[tbIndex-1].numSchLch = 0;
#ifndef L2_OPTMZ
   if (hqP->tbInfo[tbIndex-1].tb)
   {
      rgDHMHdlBufFree(cell->macInst - RG_INST_START, &hqP->tbInfo[tbIndex-1].tb);
   }
#else
   /* L2 Optimization for mUe/Tti:  macHdr, macCes and macPad mBuf pointers
    * shall not be released. However, Inorder to release harq info/TB info,
    * just Resetting rPtr and wPtr of these mbufs to db_base
    */
   tb = &(hqP->tbInfo[tbIndex-1].tb);
   if (tb->tbPres == TRUE)
   {
      RG_FREE_TB(tb);
   }
#endif
   hqP->tbInfo[tbIndex-1].schdTa.pres = FALSE;
#ifdef LTE_ADV
   hqP->tbInfo[tbIndex -1].sCellActCe.pres = FALSE;
#endif

   /* Decrementing might lead to roundoff error in case of say UE reset
    * where all the HqProcs irrespective whether in use are called for rls.
    * Hence to avoid the same shift operator is being used. */
   hqP->numOfTBs = hqP->numOfTBs >> 1;
   for(idx = 0; idx < 2; idx++)
   {
      if (hqP->tbInfo[tbIndex-1].sfLnkInfo[idx].sfLnk.node != NULLP)
   {
         cmLListDelFrm(&hqP->tbInfo[tbIndex-1].sfLnkInfo[idx].sf->tbs,
               &(hqP->tbInfo[tbIndex-1].sfLnkInfo[idx].sfLnk));
         hqP->tbInfo[tbIndex-1].sfLnkInfo[idx].sfLnk.node  = (PTR)NULLP;
      printf("\nrgDHMRlsHqProcTB:: hqP %p \n", (Void *)hqP);
   }
      hqP->tbInfo[tbIndex-1].sfLnkInfo[idx].sf = NULLP;
   }
   /* Fix : syed It is better to reset these feilds
    * corruption avoidance */
   hqP->tbInfo[tbIndex-1].tbSz = 0;
   hqP->tbInfo[tbIndex-1].contResCe = NOTPRSNT;
   hqP->tbInfo[tbIndex-1].contResId = NULLP;

   RETVALUE(ROK);
} /* rgDHMRlsHqProc */

/**
 * @brief This function gets HARQ process with the given Id
 *
 * @details
 *
 *     Function: rgDHMGetHqProcFrmId
 *     Purpose:  This function returns the HARQ process with the given ID.
 *     Invoked by: ROM
 *     
 *  @param[in]  RgUeCb        *ue
 *  @param[in]  U8            idx
 *  @param[in]  RgDlHqProc    **hqP
 *  @return  S16       
 *         -#   ROK     if successful
 *         -#   RFAILED otherwise
 *
 **/
#ifdef ANSI
PUBLIC S16 rgDHMGetHqProcFrmId
(
RgUeCb               *ue,
U8                   idx,
RgDlHqProcCb         **hqP
)
#else
PUBLIC S16 rgDHMGetHqProcFrmId(ue, idx, hqP)
RgUeCb               *ue;
U8                   idx;
RgDlHqProcCb         **hqP;
#endif
{
   TRC2(rgDHMGetHqProcFrmId)

   /* Pick the proc based on the index provided */
   *hqP = (ue->dl.hqEnt.procs[idx]);

   RETVALUE(ROK);
} /* rgDHMGetHqProcFrmId */

/*PRIVATE U32 dataAvl; */
/**
 * @brief Handler for sending data to PHY
 *
 * @details
 *
 *     Function : rgDHMSndDatReq
 *     
 *     This function shall send the MAC PDU built for the UE to TOM
 *     when invoked as part of TTI processing and keep track of the number of
 *     transmissions for this TB.
 *     
 *           
 *  @param[in]  RgCellCb      *cell
 *  @param[in]  RgDlHqProcCb  *hqE 
 *  @param[out] RgErrInfo     *err 
 *  @return     S16
 *      -#ROK 
 *      -#RFAILED 
 **/
#ifdef ANSI
PUBLIC S16 rgDHMSndDatReq
(
RgCellCb        *cellCb,
RgDlSf          *dlSf,
RgTfuDatReqInfo *datInfo,
RgDlHqProcCb   *hqP,
RgErrInfo      *err 
)
#else
PUBLIC S16 rgDHMSndDatReq(cellCb, dlSf, datInfo, hqP, err)
RgCellCb        *cellCb;
RgDlSf          *dlSf;
RgTfuDatReqInfo *datInfo;
RgDlHqProcCb    *hqP;
RgErrInfo       *err;
#endif
{
   U8 i;
   Inst               inst = cellCb->macInst - RG_INST_START;
   RgTfuDatReqPduInfo   *datReq;
   RgBldPduInfo      bldPdu;
   /*Added this variable to figure out that whether this UE data
     has to be inclueded in the TFU Data request.*/
   Bool  dataAvlblUe;

   TRC2(rgDHMSndDatReq)
  
   dataAvlblUe = TRUE;
   for(i=0;i< RG_MAX_TB_PER_UE;i++)
   {
         /* printf("\nDHMSndDatReq1: Rnti %d dlSfSchdTime(sfn sf) : (%d %d)\n"
                "macCell(sfn sf): (%d %d) tbTimingInfo(sfn sf): (%d %d)\n"
                "dlSf %p dlSf->tbs.count %d hqp %p tb %p\n",
                             hqP->tbInfo[i].pdcch.rnti,
                             dlSf->schdTime.sfn, dlSf->schdTime.subframe,
                             cellCb->crntTime.sfn, cellCb->crntTime.subframe,
                             hqP->tbInfo[i].timingInfo.sfn, 
                             hqP->tbInfo[i].timingInfo.subframe,
                             (Void *)dlSf, dlSf->tbs.count,
                             (Void *)hqP,
                             (Void *)hqP->tbInfo[i].tb);*/
      /* Mukesh :: in case of rpepetiton this is not rerd*/
      if (hqP->tbInfo[i].sfLnkInfo[dlSf->schdTime.subframe % 2].sf == dlSf)
      {
         /* Check if data from RLC had been received and got muxed. */
#ifndef L2_OPTMZ
         if (hqP->tbInfo[i].tb == NULLP) 
#else
         if (!(hqP->tbInfo[i].tb.tbPres)) 
#endif
         {
#ifndef LTE_ADV
            if (hqP->tbInfo[i].schdTa.pres == TRUE ||
                  hqP->tbInfo[i].contResCe == PRSNT_NODEF)
#else
            if ((hqP->tbInfo[i].schdTa.pres == TRUE) ||
                 (hqP->tbInfo[i].contResCe == PRSNT_NODEF) ||
                 (hqP->tbInfo[i].sCellActCe.pres == TRUE))
#endif
            {
               /* Data not received but ta needs to be sent. */
               /* MUX TA and send it */
               bldPdu.datReq    =  NULLP;
               bldPdu.reqType   =  EVTTFUTTIIND;
               bldPdu.schdTbSz  =  hqP->tbInfo[i].tbSz;
               bldPdu.ta        =  hqP->tbInfo[i].schdTa;
#ifdef LTE_ADV
               bldPdu.sCellActCe= hqP->tbInfo[i].sCellActCe;
#endif
               /* changes for CR timer implementation*/
               bldPdu.contResId =  hqP->tbInfo[i].contResId;
               if (ROK != rgMUXBldPdu(inst,&bldPdu, &(hqP->tbInfo[i].tb), err))  
               {
                  RLOG1(L_ERROR, "MUXing failed for:  MacInst %d", inst);
                  RLOG4(L_ERROR, "MUXing failed for:  time: %d/%d\
                  procId %d ueId %d", hqP->tbInfo[i].timingInfo.sfn,
                  hqP->tbInfo[i].timingInfo.subframe, hqP->procId, 
                  hqP->tbInfo[i].pdcch.rnti);

                  RETVALUE(RFAILED);
               }
            }
            else   
            {
#ifdef LTEMAC_RGU_PAD
               /* Data not received from RLC. Padding at MAC */
               bldPdu.datReq    =  NULLP;
               bldPdu.reqType   =  EVTTFUTTIIND;
               bldPdu.schdTbSz  =  hqP->tbInfo[i].tbSz;
               bldPdu.ta        =  hqP->tbInfo[i].schdTa;
#ifdef LTE_ADV
               bldPdu.sCellActCe= hqP->tbInfo[i].sCellActCe;
#endif
               bldPdu.ta.val    =  0;
               bldPdu.contResId =  NULLP;

               if (ROK != rgMUXBldPdu(inst,&bldPdu, &(hqP->tbInfo[i].tb), err))  
               {
                  RLOG1(L_ERROR, "MUXing failed for:  MacInst %d", inst);
                  RLOG4(L_ERROR, "MUXing failed for:  time: %d/%d\
                  procId %d ueId %d", hqP->tbInfo[i].timingInfo.sfn,
                  hqP->tbInfo[i].timingInfo.subframe, hqP->procId, 
                  hqP->tbInfo[i].pdcch.rnti);
                  
                  RETVALUE(RFAILED);
               }
#else
               /*Padding is not done so data for this UE will not be
                 included.*/
               dataAvlblUe = FALSE;
#endif
            }
         }
         else
         {
         }
      }
      //else
      {
      }
   }

   /*If Data/Padding is not available for UE, then we can not include
     any Data for this UE in TFU Data Request.*/
   if(!dataAvlblUe)
   {
      /*Free up the HARQ process for this allocation.*/
      /* Release First TB, as this would be anyway there*/
      rgDHMRlsHqProcTB(cellCb, hqP, 1);
      if(2 == hqP->numOfTBs)
      {
         rgDHMRlsHqProcTB(cellCb, hqP, 2);
      }
      
      RETVALUE(ROK);
   }

   if (rgGetEventMem(inst,(Ptr *)&datReq, sizeof(TfuDatReqPduInfo),
            &(datInfo->memCp)) != ROK)
   {
      RETVALUE(RFAILED);
   }
   /* Fill the TFU Dat Req with information from Harq Proc */
  
   rgDHMBldTfuDatReq(cellCb, dlSf, hqP, datReq);

   /* MS_WORKAROUND for ccpu00122894 */
   for(i=0;i< RG_MAX_TB_PER_UE;i++)
   {
      if (hqP->tbInfo[i].sfLnkInfo[dlSf->schdTime.subframe % 2].sf == dlSf)
      {
         cmLListDelFrm(&dlSf->tbs, &(hqP->tbInfo[i].sfLnkInfo[dlSf->schdTime.subframe % 2].sfLnk));
         hqP->tbInfo[i].sfLnkInfo[dlSf->schdTime.subframe % 2].sfLnk.node = NULLP;
         
        
        hqP->tbInfo[i].sfLnkInfo[dlSf->schdTime.subframe % 2].sf = NULLP;
      }
   }
   cmLListAdd2Tail(&datInfo->pdus, &(datReq->lnk));
   datReq->lnk.node = (PTR)datReq;

   RETVALUE(ROK);
}  /* rgDHMSndDatReq */

/**
 * @brief Function to handle RGU datReq received from ROM
 *
 * @details
 *
 *     Function : rgDHMHndlDedDatReq
 *     
 *     This function shall act on the datReq received on RGU. It shall 
 *     store the data IDs for all the logical channels and get the MAC 
 *     PDU built.
 *     
 *           
 *  @param[in]  Inst        inst
 *  @param[in]  RgDlHqProcCb   *hqProc 
 *  @param[in]  RgRguDedDatReq *datReq
 *  @param[out] RgErrInfo      *err
 *  @return     S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PUBLIC S16 rgDHMHndlDedDatReq
(
Inst           inst,
RgDlHqProcCb   *hqProc,
RgRguDDatReqPerUe *datReq,
RgDlSf            *dlSf,
RgErrInfo      *err
)
#else
PUBLIC S16 rgDHMHndlDedDatReq(inst,hqProc, datReq, dlSf, err)
Inst           inst;
RgDlHqProcCb   *hqProc;
RgRguDDatReqPerUe *datReq;
RgDlSf            *dlSf;
RgErrInfo      *err;
#endif
{
//   U32            len;
   U8             i;
   U8             j;
   RgBldPduInfo   bldPdu;
   U8             tbIndex;
#ifdef L2_OPTMZ
   RgTfuDatReqTbInfo     *tb;
#endif

   TRC2(rgDHMHndlDedDatReq);

   tbIndex = (U8)(datReq->transId & 0x03);
   /* Accept all the data requests even if delayed in case nothing
    * has been sent earlier on the harq proc.
    */
   if((datReq->nmbOfTbs > RG_MAX_TB_PER_UE) ||
         (tbIndex == 0))
   {
      /* release corresponding TBs from SF tbs List */
      for(j=0;j<datReq->nmbOfTbs;j++)
      {
         if (!(tbIndex & (j+1)))
         {
            j++;
         } 
         rgDHMRlsHqProcTB(rgCb[inst].cell, hqProc, (U8)(j+1));
      }
      RETVALUE(RFAILED);
   }

   for(i=0;i<datReq->nmbOfTbs;i++)
   {
      /* tbIndex 01 corresponds to presence of 1st TB
       * 10 corresponds 2nd TB
       * 11 corresponds two TBs of UE */
      if (!(tbIndex & (i+1)))
      {
          continue;
      }
      if (hqProc->tbInfo[i].sfLnkInfo[dlSf->schdTime.subframe % 2].sfLnk.node == NULLP)
      {
         /* release corresponding TBs from SF tbs List */
         for(j=0;j<datReq->nmbOfTbs;j++)
         {
            if (!(tbIndex & (j+1)))
            {
               j++;
            }
            rgDHMRlsHqProcTB(rgCb[inst].cell, hqProc, (U8)(j+1));
            printf("\nrgDHMHndlDedDatReq:: hqP %p \n", (Void *)hqProc);
         }
         RETVALUE(RFAILED);

      }
#ifndef L2_OPTMZ
      RG_FREE_MSG(hqProc->tbInfo[i].tb);
      /* L2 optimization for mUe/Tti: Pre-allocated mBuf pointers(macHdr, 
       * macCes and MacPad) of harq TB need to be reset to db_base
       */
#else
      tb = &hqProc->tbInfo[i].tb;
      if (tb->tbPres == TRUE)
      {
         RG_FREE_TB(tb);
      }
#endif
      bldPdu.datReq    =  datReq;
      bldPdu.reqType   =  EVTRGUDDATREQ;
      bldPdu.schdTbSz  =  hqProc->tbInfo[i].tbSz;
      bldPdu.tbIndex   =  i+1;
      bldPdu.ta        =  hqProc->tbInfo[i].schdTa;
#ifdef LTE_ADV
      bldPdu.sCellActCe= hqProc->tbInfo[i].sCellActCe;
#endif
      bldPdu.contResId =  NULLP;
#ifdef LTE_L2_MEAS
      /* Store tbId from RLC in DDatRequest */
      hqProc->tbId[i] = datReq->datReqTb[i].tbId;


      hqProc->status[i] =  FALSE;
#endif 
      if(rgMUXBldPdu(inst,&bldPdu, &(hqProc->tbInfo[i].tb), err) != ROK)
      {
         RLOG1(L_ERROR, "MUXing failed for:  MacInst %d", inst);
         RLOG4(L_ERROR, "MUXing failed for:  time: %d/%d\
               procId %d ueId %d", hqProc->tbInfo[i].timingInfo.sfn,
               hqProc->tbInfo[i].timingInfo.subframe, hqProc->procId, 
               hqProc->tbInfo[i].pdcch.rnti);

         /* release corresponding TBs from SF tbs List */
         for(j=0;j<datReq->nmbOfTbs;j++)
         {
            if (!(tbIndex & (j+1)))
            {
               j++;
            }
            rgDHMRlsHqProcTB(rgCb[inst].cell, hqProc, (U8)(j+1));
         }
         RETVALUE(RFAILED);
      }
      /*
      SFndLenMsg(hqProc->tbInfo[i].tb, &len);
      */
   }
   RETVALUE(ROK);
}  /* rgDHMHndlDedDatReq */

/**
 * @brief Function to handle RGU datReq received from ROM
 *
 * @details
 *
 *     Function : rgDHMHndlCmnDatReq
 *     
 *     This function shall act on the datReq received on RGU. It shall 
 *     store the data IDs for all the logical channels and get the MAC 
 *     PDU built.
 *     
 *           
 *  @param[in]  Inst        inst
 *  @param[in]  RgDlHqProcCb   *hqProc 
 *  @param[in]  RgRguCmnDatReq *datReq
 *  @param[out] RgErrInfo      *err
 *  @return     S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PUBLIC S16 rgDHMHndlCmnDatReq
(
Inst           inst,
RgDlHqProcCb   *hqProc,
RgRguCmnDatReq *datReq,
RgErrInfo      *err
)
#else
PUBLIC S16 rgDHMHndlCmnDatReq(inst,hqProc, datReq, err)
Inst           inst;
RgDlHqProcCb   *hqProc;
RgRguCmnDatReq *datReq;
RgErrInfo      *err;
#endif
{
   RgUstaDgn      dgn;
   RgBldPduInfo   bldPdu;

   TRC2(rgDHMHndlCmnDatReq)

#ifndef L2_OPTMZ
      if (hqProc->tbInfo[0].tb != NULLP)
#else
      /* If numLch is non zero means HQ Proc is busy*/
      if (hqProc->tbInfo[0].tb.tbPres)
#endif
      {
         /* datReq discarded. Generate an alarm */
         rgFillDgnParams(inst,&dgn, LRG_USTA_DGNVAL_HARQ); 
         rgLMMStaInd(inst,LCM_CATEGORY_PROTOCOL, LCM_EVENT_UI_INV_EVT,
               LRG_CAUSE_HQ_PROC_BUSY, &dgn);
         RETVALUE(RFAILED);
      }

   bldPdu.datReq    =  datReq;
   bldPdu.reqType   =  EVTRGUCDATREQ;
   bldPdu.schdTbSz  =  hqProc->tbInfo[0].tbSz;
   bldPdu.ta        =  hqProc->tbInfo[0].schdTa;
#ifdef LTE_ADV
   bldPdu.sCellActCe= hqProc->tbInfo[0].sCellActCe;
#endif

   bldPdu.contResId  =  hqProc->tbInfo[0].contResId;

   if(rgMUXBldPdu(inst,&bldPdu, &(hqProc->tbInfo[0].tb), err) != ROK)
   {
      RLOG1(L_ERROR, "MUXing failed for:  MacInst %d", inst); 
      RLOG4(L_ERROR, "MUXing failed for: time: %d/%d\
               procId %d ueId %d", hqProc->tbInfo[0].timingInfo.sfn,
            hqProc->tbInfo[0].timingInfo.subframe, hqProc->procId, 
            hqProc->tbInfo[0].pdcch.rnti);

      RG_FREE_MSG(datReq->pdu);
      RETVALUE(RFAILED);
   }

   RETVALUE(ROK);
}  /* rgDHMHndlCmnDatReq */

/**
 * @brief Function to get consolidate grants and send consolidated grant to RLC
 *
 * @details
 *
 *     Function : rgDHMSndConsolidatedStaInd
 *     
 *     This function shall be invoked by Scheduler to trigger DHM to send a
 *     consolidated status indication of all UE scheduled in a TTI as well as
 *     send consolidated CStaInd for MSG4 and for all common channels(PCCH,
 *     if RGR_SI_SCH is not defined then it includes BCH and BCCH also)
 *     
 *           
 *  @param[in]  RgCellCb       *cell
 *  @param[in]  RgInfUeInfo   *ueInfo,
 *  @param[in]  CmLteTimingInfo timingInfo,
 *  @param[out] RgErrInfo      err
 *  @param[in]  RguCStaIndInfo   *cStaInd
 *  @return     S16
 *      -# ROK 
 *      -# RFAILED 
 **/
 RgUeCb  *gUe =NULLP;
#ifdef ANSI
PUBLIC S16 rgDHMSndConsolidatedStaInd
(
RgCellCb        *cell,
RgInfUeInfo     *ueInfo,
CmLteTimingInfo timingInfo,
RgErrInfo       *err
)
#else
PUBLIC S16 rgDHMSndConsolidatedStaInd(cell, ueInfo, timingInfo, err)
RgCellCb        *cell;
RgInfUeInfo     *ueInfo;
CmLteTimingInfo timingInfo;
RgErrInfo       *err;
#endif
{
   SuId            rguDlSpId;/*need to use spID instead of suID*/
   U8              idx;
   U8              ueIdx;
   U8              lcIdx;
   U8              tbIndex=0,idx1;
   RgDlSf          *dlSf = &cell->subFrms[(timingInfo.subframe % RG_NUM_SUB_FRAMES)];
   Inst            inst = cell->macInst - RG_INST_START;
//   Bool            isDStaReqrd = FALSE;
   RgRguDedStaInd  *dStaInd[rgCb[inst].numRguSaps] ;
   RgUpSapCb      *rguDlSap[rgCb[inst].numRguSaps];

   int lchBufSize =0;
   RgUeCb         *ue;
   RgDlHqProcCb   *hqP;
   RgInfUeAlloc   *allocInfo;
   U8             activeSapCnt = 0;
   U8             staIndCnt    = 0;
#ifdef LTE_ADV
   Bool           hqPAdded     = FALSE;
#endif
#ifdef L2_OPTMZ
   RgTfuDatReqTbInfo     *tb;   /* TB to be sent to CL/PHY*/
#endif

   TRC2(rgDHMSndConsolidatedStaInd)
   cmMemset ((U8 *)dStaInd, 0, (sizeof(RgRguDedStaInd *) * rgCb[inst].numRguSaps));
   cmMemset ((U8 *)rguDlSap, 0, (sizeof(RgUpSapCb  *) * rgCb[inst].numRguSaps));

   /* Send StaInd for the scheduled UEs */
   for(ueIdx = 0; ueIdx < ueInfo->numUes; ueIdx++)
   {
#ifdef LTE_ADV
      hqPAdded = FALSE;
#endif
      if((ue=rgDBMGetUeCb (cell, ueInfo->allocInfo[ueIdx].rnti)) == NULLP)
      {
         /* Check in RachLst */
         if((ue=rgDBMGetUeCbFromRachLst (cell, 
                     ueInfo->allocInfo[ueIdx].rnti)) == NULLP)
         {
            RLOG_ARG1(L_ERROR,DBG_CELLID,cell->cellId,"CRNTI:%d No ueCb found", 
                      ueInfo->allocInfo[ueIdx].rnti);
            /*Fix: If one UE is not present dont return, look for the next.*/
            continue;
         }
      }


      rgDHMGetHqProcFrmId(ue,ueInfo->allocInfo[ueIdx].hqProcId,&hqP);
      allocInfo = &ueInfo->allocInfo[ueIdx];
      gUe = ue;

      /* Fix : syed Avoid sending data for a RETX
       * if initial TX data processing was unsuccessful */
      if((allocInfo->tbInfo[0].isReTx == TRUE) &&
            (hqP->tbInfo[0].tbSz == 0)) 
      {
         RLOG_ARG2(L_ERROR,DBG_CELLID,cell->cellId,
               "CRNTI:%d RETX hqP(%d) tb(0) for a failed New Tx", 
                  allocInfo->rnti, hqP->procId);        
         continue;
      }
      if((allocInfo->tbInfo[1].isReTx == TRUE) &&
            (hqP->tbInfo[1].tbSz == 0)) 
      {
         RLOG_ARG2(L_ERROR,DBG_CELLID,cell->cellId,
                   "CRNTI:%d RETX hqP(%d) tb(1) for a failed New Tx", 
                  allocInfo->rnti, hqP->procId);        
         continue;
      }

      if(ue->rguDlSap != NULLP)
      {
          rguDlSpId = ue->rguDlSap->sapCfg.spId;
      }else
      {/* UeCb is from rachList */
          rguDlSpId = cell->rguDlSap->sapCfg.spId;
      }


      for(idx=allocInfo->tbStrtIdx;((idx-allocInfo->tbStrtIdx) <\
               allocInfo->nmbOfTBs); idx++)
      {
         RguCStaIndInfo  *cStaInd;
#ifdef TFU_UPGRADE
         /* LTE_ADV_FLAG_REMOVED_START */
         hqP->tbInfo[idx].isEnbSFR = allocInfo->isEnbSFR;
         /* update pA value */
         hqP->tbInfo[idx].pa = allocInfo->pa;
         /* LTE_ADV_FLAG_REMOVED_END */
#endif

         hqP->numOfTBs =  allocInfo->nmbOfTBs;
         hqP->tbInfo[idx].sfLnkInfo[dlSf->schdTime.subframe % 2].sfLnk.node = (PTR)hqP;
         hqP->tbInfo[idx].sfLnkInfo[dlSf->schdTime.subframe % 2].sf = dlSf;
         cmLListAdd2Tail(&dlSf->tbs,&(hqP->tbInfo[idx].sfLnkInfo[dlSf->schdTime.subframe % 2].sfLnk));
         /* Changes as part of performance testing*/
         /*   hqP->numOfTBs++;*/
         hqP->tbInfo[idx].doa = allocInfo->doa;
         hqP->tbInfo[idx].txMode = allocInfo->txMode;
         hqP->tbInfo[idx].puschRptUsd = allocInfo->puschRptUsd;
         hqP->tbInfo[idx].puschPmiInfo = allocInfo->puschPmiInfo;
#ifdef LTEMAC_SPS
         hqP->tbInfo[idx].pdcch.rnti = allocInfo->pdcchRnti;
#else
         hqP->tbInfo[idx].pdcch.rnti = allocInfo->rnti;
#endif
         if(allocInfo->tbInfo[idx].isReTx == TRUE)
         {
            hqP->tbInfo[idx].pdcch.dci = allocInfo->dciInfo;
            continue;
         }

         hqP->tbInfo[idx].timingInfo = timingInfo;
         hqP->tbInfo[idx].pdcch.dci = allocInfo->dciInfo;
#ifndef L2_OPTMZ
         RG_FREE_MSG(hqP->tbInfo[idx].tb);
#else
         /* L2 optimization for mUe/Tti: Pre-allocated mBuf pointers(macHdr, 
          * macCes and MacPad) of harq TB need to be reset to db_base
          */
         tb = &(hqP->tbInfo[idx].tb);
         if (tb->tbPres == TRUE)
         {
            RG_FREE_TB(tb);
         }
#endif
         hqP->tbInfo[idx].tbSz = allocInfo->tbInfo[idx].schdTbSz; 

         hqP->tbInfo[idx].schdTa.pres = allocInfo->tbInfo[idx].ta.pres;
         hqP->tbInfo[idx].schdTa.val  = allocInfo->tbInfo[idx].ta.val;

#ifdef LTE_ADV
         hqP->tbInfo[idx].sCellActCe.pres = allocInfo->tbInfo[idx].sCellActCe.pres;
         hqP->tbInfo[idx].sCellActCe.val  = allocInfo->tbInfo[idx].sCellActCe.val;
#endif

#ifdef LTE_ADV 
         if(( hqPAdded == TRUE) || (ROK == rgLaaPushHqPToScellLst(allocInfo,cell,timingInfo)))
         {
            hqPAdded = TRUE;
            continue;
         }
#endif
         if (allocInfo->tbInfo[idx].schdDat[0].lcId == RG_CCCH_LCID)        
         {
#ifndef L2_OPTMZ
            RG_FREE_MSG(hqP->tbInfo[idx].tb);
#else
           /* L2 optimization for mUe/Tti: Pre-allocated mBuf pointers(macHdr, 
            * macCes and MacPad) of harq TB need to be reset to db_base
            */
           tb = &(hqP->tbInfo[idx].tb);

           if (tb->tbPres == TRUE)
           {
              RG_FREE_TB(tb);
           }
#endif
            hqP->tbInfo[0].contResCe  = allocInfo->tbInfo[0].contResCe;
            if(allocInfo->tbInfo[0].contResCe)
            {
               hqP->tbInfo[0].contResId = &ue->contResId;
            }


            if(allocInfo->tbInfo[idx].numSchLch == 0)
            {
               RLOG_ARG2(L_DEBUG,DBG_CELLID,cell->cellId,
                        "UEID:%d MSG4 with only contResId hqP(%d)",
               			allocInfo->rnti,
                     	hqP->procId);
               hqP->tbInfo[idx].numSchLch = 0;
               continue;
            }

            /* Increamenting the tbIndex instead of
               assigning it to constant */
            tbIndex++;


            hqP->tbInfo[idx].numSchLch = 1;
            hqP->tbInfo[idx].schdData[0].lcId = 
               allocInfo->tbInfo[idx].schdDat[0].lcId;
            hqP->tbInfo[idx].schdData[0].schdSz = 
               allocInfo->tbInfo[idx].schdDat[0].numBytes;

           // if(cStaInd == NULLP)
            {
               if ((rgAllocShrablSBuf(inst,(Data**)&cStaInd, sizeof(RguCStaIndInfo))) != ROK)
               {
                  err->errType  = RGERR_DHM_SND_STA_IND;
                  err->errCause = RG_DHM_MEM_ALLOC_FAIL;
                  RETVALUE(RFAILED); 
               }
            }

            idx1 = (hqP->procId << 2) | tbIndex;
            
            cStaInd->cellId    = cell->cellId;
            cStaInd->rnti      = allocInfo->rnti;
            cStaInd->lcId      = cell->dlCcchId;
            cStaInd->transId   = (timingInfo.sfn << 16) | 
                                 (timingInfo.subframe << 8) | idx1;
               /* ADD Changes for Downlink UE Timing Optimization */
#ifdef LTEMAC_DLUE_TMGOPTMZ
            dlSf->remDatReqCnt++;
#endif
            RLOG_ARG3(L_DEBUG,DBG_CELLID,cell->cellId,
		               "RNTI:%d UE:MSG4 grant for CCCH hqP(%d) LCID:%d",
                     allocInfo->rnti, 
                     hqP->procId,
		               cStaInd->lcId);       
            /* Fix : syed Avoid return param for interface prim and
             * proceed for other UEs. For the failed UE, MAC shall
             * do padding. */
            rgUIMSndCmnStaInd(cell->macInst,cell->rguDlSap,cStaInd);
            break;
         }
         else
         {
            tbIndex+=idx+1;
#ifndef L2_OPTMZ
            RG_FREE_MSG(hqP->tbInfo[idx].tb);
#else
            /* L2 optimization for mUe/Tti: Pre-allocated mBuf pointers(macHdr, 
             * macCes and MacPad) of harq TB need to be reset to db_base
             */
            tb = &(hqP->tbInfo[idx].tb);
            if (tb->tbPres == TRUE)
            {  
               RG_FREE_TB(tb);
            }
#endif

            if((NULLP == dStaInd[rguDlSpId]) && (allocInfo->tbInfo[idx].numSchLch))
            {
               if ((rgAllocShrablSBuf (inst,(Data**)&dStaInd[rguDlSpId], sizeof(RguDStaIndInfo))) != ROK)
               {
                  err->errType  = RGERR_DHM_SND_STA_IND;
                  err->errCause = RG_DHM_MEM_ALLOC_FAIL;
                  /* Need to return as memory allocation will fail for other UEs also*/
                  RETVALUE(RFAILED);
               }
               dStaInd[rguDlSpId]->nmbOfUeGrantPerTti = 0;
               rguDlSap[rguDlSpId] = ue->rguDlSap;
               activeSapCnt++;
            }

            for (lcIdx = 0; 
                  lcIdx < allocInfo->tbInfo[idx].numSchLch; lcIdx++)
            {
               hqP->tbInfo[idx].schdData[lcIdx].lcId = 
                  allocInfo->tbInfo[idx].schdDat[lcIdx].lcId;
               if (hqP->tbInfo[idx].schdData[lcIdx].lcId == 0)
               {
                  RLOG_ARG2(L_ERROR,DBG_CELLID,cell->cellId, 
                        "CCCH grant in DStaInd for LCID:%d CRNTI:%d",
                        hqP->tbInfo[idx].schdData[lcIdx].lcId,allocInfo->rnti);
               }
               hqP->tbInfo[idx].schdData[lcIdx].schdSz = 
                  allocInfo->tbInfo[idx].schdDat[lcIdx].numBytes;
               if(dStaInd[rguDlSpId])
               {
                  dStaInd[rguDlSpId]->staInd[dStaInd[rguDlSpId]->nmbOfUeGrantPerTti].staIndTb[idx].\
                  lchStaInd[lcIdx].lcId = allocInfo->tbInfo[idx].\
                  schdDat[lcIdx].lcId;
                  dStaInd[rguDlSpId]->staInd[dStaInd[rguDlSpId]->nmbOfUeGrantPerTti].staIndTb[idx].\
                  lchStaInd[lcIdx].totBufSize = allocInfo->tbInfo[idx].\
                  schdDat[lcIdx].numBytes;
               }

               lchBufSize+=allocInfo->tbInfo[idx].schdDat[lcIdx].numBytes;
            }
            hqP->tbInfo[idx].numSchLch = 
               allocInfo->tbInfo[idx].numSchLch;
            if(dStaInd[rguDlSpId])
            {
               dStaInd[rguDlSpId]->staInd[dStaInd[rguDlSpId]->nmbOfUeGrantPerTti].staIndTb[idx].nmbLch =
                  allocInfo->tbInfo[idx].numSchLch;
#ifdef LTE_L2_MEAS
               dStaInd[rguDlSpId]->staInd[dStaInd[rguDlSpId]->nmbOfUeGrantPerTti].staIndTb[idx].tbId =
                  hqP->tbId[idx]; 
#endif
            }
            lchBufSize=0;
         }
      }
      //if((dStaInd) && (tbIndex) && (isDStaReqrd == TRUE))
      if((dStaInd[rguDlSpId]) && (tbIndex))
      {
         idx1 = (hqP->procId << 2) | tbIndex;
         /* Create RguDStaInd struct and send to UIM */
         dStaInd[rguDlSpId]->staInd[dStaInd[rguDlSpId]->nmbOfUeGrantPerTti].rnti    = allocInfo->rnti;
         /*
            dStaInd->transId = (hqP->timingInfo.sfn << 16) | 
            (hqP->timingInfo.subframe << 8) | hqP->procId;
          */
         dStaInd[rguDlSpId]->staInd[dStaInd[rguDlSpId]->nmbOfUeGrantPerTti].transId = (timingInfo.sfn << 16) | 
            (timingInfo.subframe << 8) | idx1;
         dStaInd[rguDlSpId]->staInd[dStaInd[rguDlSpId]->nmbOfUeGrantPerTti].nmbOfTbs = hqP->numOfTBs;
#ifdef LTE_ADV
         dStaInd[rguDlSpId]->staInd[dStaInd[rguDlSpId]->nmbOfUeGrantPerTti].fillCtrlPdu = allocInfo->fillCtrlPdu;
#endif        
         /*increment num of UE as staInd is prepared for it */
         dStaInd[rguDlSpId]->nmbOfUeGrantPerTti++;
         /* ADD Changes for Downlink UE Timing Optimization */
#ifdef LTEMAC_DLUE_TMGOPTMZ
         dlSf->remDatReqCnt++;
#endif
      }
      //isDStaReqrd = FALSE;
      tbIndex = 0;
   }

   for(idx = 0; idx < rgCb[inst].numRguSaps ; idx++)
   {
      if(dStaInd[idx] != NULLP)
      {
         dStaInd[idx]->cellId  = cell->cellId;
         /* Fix : syed Avoid return param for interface prim and
          * proceed for other UEs. For the failed UE, MAC shall
          * do padding. */
         rgUIMSndDedStaInd(inst,rguDlSap[idx],dStaInd[idx]);
   
         staIndCnt++;
         if(staIndCnt == activeSapCnt)
            break;/* all valid staind are considered */
      }

   }
   RETVALUE(ROK);
}  /* rgDHMSndConsolidatedStaInd */


/**
 * @brief Function to handle building the TFU Data Request
 *
 * @details
 *
 *     Function : rgDHMBldTfuDatReq
 *     
 *     This function builds the TFU Data Request with the details 
 *     present in HARQ Process.
 *           
 *  @param[in]  RgDlHqProcCb     *hqP 
 *  @param[out] TfuDatReqPduInfo  *datReq 
 *  @return     Void
 *              None 
 **/
//U8 crashFlag = 0;
#ifdef ANSI
PRIVATE Void rgDHMBldTfuDatReq
(
RgCellCb           *cellCb,
RgDlSf             *dlSf,
RgDlHqProcCb       *hqP,
RgTfuDatReqPduInfo *datReq
)
#else
PRIVATE Void rgDHMBldTfuDatReq(cellCb, dlSf, hqP, datReq)
RgCellCb           *cellCb;
RgDlSf             *dlSf;
RgDlHqProcCb       *hqP;
RgTfuDatReqPduInfo *datReq;
#endif
{

#ifndef L2_OPTMZ
#if !(!(defined TENB_ACC) && !(defined LTE_PAL_ENB))
   Inst inst;
#elif defined(TENB_T2K3K_SPECIFIC_CHANGES) && defined(LTE_TDD)
   Inst inst;
#endif
#endif

   U8 i;

#ifdef L2_OPTMZ
   U32 lchIdx, pduIdx;
#endif
   TRC2(rgDHMBldTfuDatReq)
 
   datReq->nmbOfTBs = 0;
#ifndef L2_OPTMZ
#if !(!(defined TENB_ACC) && !(defined LTE_PAL_ENB))
   inst = cellCb->macInst - RG_INST_START;
#elif defined(TENB_T2K3K_SPECIFIC_CHANGES) && defined(LTE_TDD)
   inst = cellCb->macInst - RG_INST_START;
#endif
#endif
   /*MS_WORKAROUND  for ccpu00123904*/
   datReq->isTApres = FALSE;
#ifdef TFU_ALLOC_EVENT_NO_INIT
#ifndef L2_OPTMZ   
   datReq->mBuf[0] = 0;
   datReq->mBuf[1] = 0;
#endif    
#endif
 
   for(i=0;i<RG_MAX_TB_PER_UE;i++)
   {
#ifndef L2_OPTMZ
      if ((hqP->tbInfo[i].sfLnkInfo[dlSf->schdTime.subframe % 2].sf == dlSf) &&
          (hqP->tbInfo[i].tb != NULLP))
#else
      if ((hqP->tbInfo[i].sfLnkInfo[dlSf->schdTime.subframe % 2].sf == dlSf) &&
           RgUtlIsTbMuxed(&(hqP->tbInfo[i].tb)))
#endif
      {

         datReq->rnti           =  hqP->tbInfo[i].pdcch.rnti;
         datReq->dciInfo        =  hqP->tbInfo[i].pdcch.dci;
         datReq->doa            =  hqP->tbInfo[i].doa;
         datReq->transMode      =  hqP->tbInfo[i].txMode;
         datReq->puschRptUsd    =  hqP->tbInfo[i].puschRptUsd;
         datReq->puschPmiInfo   =  hqP->tbInfo[i].puschPmiInfo;
         /*MS_WORKAROUND  for ccpu00123904*/
         if (hqP->tbInfo[i].schdTa.pres)
         {
            datReq->isTApres       =  TRUE; 
         }
#ifdef   TFU_UPGRADE
         /* update pA value */
         datReq->pa             =  hqP->tbInfo[i].pa;
#endif
         /* LTE_ADV_FLAG_REMOVED_START */
         datReq->isEnbSFR       =  hqP->tbInfo[i].isEnbSFR;
         /* LTE_ADV_FLAG_REMOVED_END */
#ifndef L2_OPTMZ
#if (!(defined TENB_ACC) && !(defined LTE_PAL_ENB))  /* ABHI */ /* This is only temp fix. It needs to be removed
                                after rebasing to MAC CCB */
#ifdef BRDCM
         datReq->mBuf[i] = hqP->tbInfo[i].tb;
#else
         /* Intel Tdd- Commenting out the Optimization for direct Access of 
          * mBuf Index */
         /*Proper clean-up needed as this needs long stability tests
          * in all SoCs*/
#if defined(TENB_T2K3K_SPECIFIC_CHANGES) && defined(LTE_TDD)
         SIncMsgRef(hqP->tbInfo[i].tb, RG_GET_MEM_REGION(rgCb[inst]),
               RG_GET_MEM_POOL(rgCb[inst]), &datReq->mBuf[i]); 
#else
         datReq->mBuf[i] = hqP->tbInfo[i].tb;
#endif
#endif/*BRDCM*/
#else
         SIncMsgRef(hqP->tbInfo[i].tb, RG_GET_MEM_REGION(rgCb[inst]),
               RG_GET_MEM_POOL(rgCb[inst]), &datReq->mBuf[i]);
#endif
         {
            MsgLen   dbgBufLen;
            if(SFndLenMsg(datReq->mBuf[i], &dbgBufLen))
            {
               if(dbgBufLen == 0)
               {              
                  RLOG_ARG4(L_ERROR,DBG_CELLID,cellCb->cellId,
			                  "RNTI:%d SFN:%d subframe:%d tbIdx:%d Sdu Length 0 ",
                    	      datReq->rnti,
            			      hqP->tbInfo[i].timingInfo.sfn,
			                  hqP->tbInfo[i].timingInfo.subframe,i);
                  RLOG_ARG3(L_ERROR,DBG_CELLID,cellCb->cellId,
                   	      "taPres [%d] numOfTbs [%d] format[%d]",
                   	      datReq->isTApres, 
                           hqP->numOfTBs, 
                           datReq->dciInfo.format);  
               }              
            }  
         }
#else
         /* L2 optimization for mUe/Tti: Removing SIncMsgRef to avoid additional
          * mBuf allocation. MAC header, MAC Ces, MAC PDU per LCH per TB Per UE
          * and MAC padding Mbufs are being sent to CL. Populating these Ptrs
          * From TB Info to TfuDatReq
          */
         datReq->tbInfo[i].tbPres =  TRUE;
         datReq->tbInfo[i].tbSize =  hqP->tbInfo[i].tbSz;
         datReq->tbInfo[i].macHdr =  hqP->tbInfo[i].tb.macHdr;
         datReq->tbInfo[i].macCes =  hqP->tbInfo[i].tb.macCes;
         datReq->tbInfo[i].numLch =  hqP->tbInfo[i].tb.numLch;
         for(lchIdx = 0; lchIdx < hqP->tbInfo[i].tb.numLch; lchIdx++)
         {
            datReq->tbInfo[i].lchInfo[lchIdx].numPdu = hqP->tbInfo[i].tb.\
                                                       lchInfo[lchIdx].numPdu;
            for(pduIdx = 0; pduIdx < hqP->tbInfo[i].tb.lchInfo[lchIdx].numPdu;\
                  pduIdx++)
            {
               datReq->tbInfo[i].lchInfo[lchIdx].mBuf[pduIdx] =
                  hqP->tbInfo[i].tb.lchInfo[lchIdx].mBuf[pduIdx];
            }
         }
        // datReq->tbInfo[i].macPad  =  hqP->tbInfo[i].tb.macPad;
         datReq->tbInfo[i].padSize =  hqP->tbInfo[i].tb.padSize;
        // prc_trace_format_string(0x40,3,"TfuDatReq:RNTI=%d TbIdx=%d TbSize=%d PdSz=(%d) macHdraddr: (%p) macCEAddr: (%p) noLch=(%d)",datReq->rnti, i,
          //     hqP->tbInfo[i].tbSz, datReq->tbInfo[i].padSize, datReq->tbInfo[i].macHdr, datReq->tbInfo[i].macCes, datReq->tbInfo[i].numLch);

#endif
         datReq->nmbOfTBs++;
      }
   }
   RETVOID;
}  /* rgDHMBldTfuDatReq */


#ifdef L2_OPTMZ
/**
 * @brief This function releases a HARQ process
 *
 * @details
 *
 *     Function: rgDHMFreeHqProcTB
 *     Purpose:  This function returns a HARQ process to HARQ Entity 
 *               in the DL direction.
 *     
 *               1. Add the HARQ process to the free queue.
 *     Invoked by: scheduler and HARQ processing
 *     
 *  @param[in]  RgDlHqProc    *hqP
 *  @return  Void      
 *
 **/
#ifdef ANSI
PUBLIC S16 rgDHMFreeHqProcTB
(
RgDlHqProcCb         *hqP,
U8                   tbIndex
)
#else
PUBLIC S16 rgDHMFreeHqProcTB(hqP, tbIndex)
RgDlHqProcCb         *hqP;
U8                   tbIndex;
#endif
{
   RgTfuDatReqTbInfo     *tb;   /* TB to be sent to CL/PHY*/
   U8                    idx;

   TRC2(rgDHMFreeHqProcTB)

   if((tbIndex > RG_MAX_TB_PER_UE) ||
      (tbIndex == 0))
   {
      RETVALUE(RFAILED);
   }

   tb = &(hqP->tbInfo[tbIndex-1].tb);
   RG_FREE_MSG(tb->macHdr);
   RG_FREE_MSG(tb->macCes);

   for(idx = 0; idx < 2; idx++)
   {
      if (hqP->tbInfo[tbIndex-1].sfLnkInfo[idx].sfLnk.node != NULLP)
   {
         cmLListDelFrm(&hqP->tbInfo[tbIndex-1].sfLnkInfo[idx].sf->tbs,
               &(hqP->tbInfo[tbIndex-1].sfLnkInfo[idx].sfLnk));
         hqP->tbInfo[tbIndex-1].sfLnkInfo[idx].sfLnk.node  = (PTR)NULLP;
      printf("\nrgDHMFreeHqProcTB:: hqP %p \n", (Void *)hqP);
   }
      hqP->tbInfo[tbIndex-1].sfLnkInfo[idx].sf = NULLP;
   }
   RETVALUE(ROK);
}
#endif



/**
 * @brief Handler for freeing up the harq related information from ueCb
 *
 * @details
 *
 *     Function : rgDHMFreeUe
 *     
 *     This function shall free up the HARQ specific information from ueCb.
 *           
 *  @param[in]  Inst        inst
 *  @param[in]  RgDlHqEnt     *hqE 
 *
 *  @return     None.
 *
 **/
#ifdef ANSI
PUBLIC Void rgDHMFreeUe
(
Inst               inst,
RgDlHqEnt          *hqE
)
#else
PUBLIC Void rgDHMFreeUe(inst,hqE)
Inst               inst;
RgDlHqEnt          *hqE;
#endif
{
   U8             i;
   TRC2(rgDHMFreeUe)

   if(hqE->procs)
   {
      /* Free all the memory associated with HARQ */
      for (i=0; i < hqE->numHqProcs; i++)
      {
#ifndef L2_OPTMZ
         rgDHMRlsHqProcTB(rgCb[inst].cell, hqE->procs[i], 1);
         rgDHMRlsHqProcTB(rgCb[inst].cell, hqE->procs[i], 2);
#else
         rgDHMFreeHqProcTB(hqE->procs[i], 1);
         rgDHMFreeHqProcTB(hqE->procs[i], 2);
#endif
         
         rgFreeSBuf(inst,(Data **)&(hqE->procs[i]), sizeof(RgDlHqProcCb));
#ifdef LTE_ADV
         rgDHMFreeSavedHqP(inst,hqE,i);
#endif
      }

      /*ccpu00117052 - MOD - Passing double pointer for proper NULLP
                            assignment */
   }

   RETVOID;

}  /* rgDHMFreeUe */
/**
 * @brief Function for handling RaResp request received from scheduler to MAC
 *
 * @details
 *
 *     Function : RgSchMacRstHqEntReq
 *     
 *     This function shall be invoked whenever a sec cell of an ue
 *     is deactivated. MAC needs to reset the harqentity associated 
 *     with the deactivated scell of the ue
 *     
 *           
 *  @param[in] Pst             *pst
 *  @param[in] RgInfResetHqEnt *hqEntInfo
 *  @return  S16
 *      -# ROK 
 **/
#ifdef ANSI
PUBLIC S16 RgSchMacRstHqEntReq
(
Pst*                 pst,    
RgInfResetHqEnt*     hqEntInfo
)
#else
PUBLIC S16 RgSchMacRstHqEntReq(pst, hqEntInfo)
Pst*                 pst;
RgInfResetHqEnt*     hqEntInfo;
#endif
{
   Inst      inst;
   RgCellCb  *cell;
   RgUeCb    *ue;

   inst = pst->dstInst - RG_INST_START;

   if (((cell = rgCb[inst].cell) == NULLP) ||
       (rgCb[inst].cell->cellId != hqEntInfo->cellId))
   {
      RGDBGERRNEW(inst,(rgPBuf(inst), "For user [%d]Cell does not exist %d\n",
                hqEntInfo->crnti,hqEntInfo->cellId));
      RETVALUE(RFAILED);
   }

   if ((ue = rgDBMGetUeCb(cell, hqEntInfo->crnti)) == NULLP)
   {
      RGDBGERRNEW(inst,(rgPBuf(inst), "[%d]UE does not exist for this hqEntInfo\n",
                       hqEntInfo->crnti));
      RETVALUE(RFAILED);
   }

   rgDHMUeReset(cell, &ue->dl.hqEnt);

   RETVALUE(ROK);
}
U32 gSaveVal;

/**
 * @brief Function for handling RaResp request received from scheduler to MAC
 *
 * @details
 *
 *     Function : RgSchMacRlsHqReq
 *     
 *     This function shall be invoked whenever scheduler is done with the
 *     allocations of random access responses for a subframe.
 *     This shall invoke RAM to create ueCbs for all the rapIds allocated and 
 *     shall invoke MUX to create RAR PDUs for raRntis allocated.
 *     
 *           
 *  @param[in] CmLteCellId         cellId,
 *  @param[in] CmLteTimingInfo     timingInfo,
 *  @param[in] RaRespInfo          *rarInfo
 *  @return  S16
 *      -# ROK 
 **/
#ifdef ANSI
PUBLIC S16 RgSchMacRlsHqReq
(
Pst                 *pst,
RgInfRlsHqInfo      *rlshqUeInfo
)
#else
PUBLIC S16 RgSchMacRlsHqReq(pst, rlshqUeInfo)
Pst                 *pst;
RgInfRlsHqInfo      *rlshqUeInfo;
#endif
{
   Inst           inst;
   RgCellCb       *cell = NULLP;
   RgUeCb         *ue;
   RgDlHqProcCb   *hqP;
   U8             idx1,idx2;
#ifdef LTE_L2_MEAS
   U8                tbId;
   RguHarqStatusInd  hqStaInd;
   Bool              isValidTbId = FALSE;
#endif
   U32        startTime=0;
   
   TRC2(RgSchMacRlsHqReq)

   RG_IS_INST_VALID(pst->dstInst);
   inst = pst->dstInst - RG_INST_START;
   cell  = rgCb[inst].cell;
   /*starting Task*/
   SStartTask(&startTime, PID_MAC_AM_HARQ_RLS);

   if(NULLP == rlshqUeInfo)
   {
      RETVALUE(RFAILED);
   }

   if((cell  == NULLP)
      ||( cell->cellId != rlshqUeInfo->cellId))
   {
       
      RLOG_ARG0(L_ERROR,DBG_CELLID,rlshqUeInfo->cellId,
                "No cellCb found with cellId");
      RETVALUE(RFAILED);
   }

   if(NULLP == rlshqUeInfo->ueHqInfo)
   {
      RETVALUE(RFAILED);
   }

   for(idx1 = 0; idx1 < rlshqUeInfo->numUes; idx1++)
   {
      if((ue=rgDBMGetUeCb (cell, rlshqUeInfo->ueHqInfo[idx1].rnti)) == NULLP)
      {
         /* Check in RachLst */
         if((ue=rgDBMGetUeCbFromRachLst (cell, 
                     rlshqUeInfo->ueHqInfo[idx1].rnti)) == NULLP)
         {
            RLOG_ARG1(L_ERROR,DBG_CELLID,rlshqUeInfo->cellId, "CRNTI:%d No ueCb found",
                     rlshqUeInfo->ueHqInfo[idx1].rnti);
            continue;
         }
      }
#ifdef LTE_ADV
#ifdef LAA_DBG
     if ((rlshqUeInfo->ueHqInfo[idx1].rlsOperationType && !gSaveVal) || (rlshqUeInfo->ueHqInfo[idx1].hqProcId > 8))
      {
         int *p = NULL;
         RLOG_ARG1(L_INFO," SPURIOUS CALLL !!!! procId %d \n", rlshqUeInfo->ueHqInfo[idx1].hqProcId);


       printf ("RgSchMacRlsHqReq cell %d : numUes %d idx %d rnti %d hqProc %d numTbs %d tbid[0] %d tbid[1] %d rlsopr %d \n",
      cell->cellId,
       rlshqUeInfo->numUes,
       idx1,
       rlshqUeInfo->ueHqInfo[idx1].rnti,
       rlshqUeInfo->ueHqInfo[idx1].hqProcId,
       rlshqUeInfo->ueHqInfo[idx1].numOfTBs,
       rlshqUeInfo->ueHqInfo[idx1].tbId[0],
       rlshqUeInfo->ueHqInfo[idx1].tbId[1],
       rlshqUeInfo->ueHqInfo[idx1].rlsOperationType);
      
         *p = 10; 
      }
#endif
      gSaveVal = 0;


      RgSchMacHndlRelReq(cell, ue, &rlshqUeInfo->ueHqInfo[idx1]);

      if (RGINF_RLS_HQ_DEL_TB == rlshqUeInfo->ueHqInfo[idx1].rlsOperationType)
      {
         /* If REQ is to DEL the saved TBs no need to free the HqP as it's already
            freed up earlier */
         continue;
      }
#endif /* LTE_ADV */
      rgDHMGetHqProcFrmId(ue,rlshqUeInfo->ueHqInfo[idx1].hqProcId,&hqP);
      if(rlshqUeInfo->ueHqInfo[idx1].status[0] != TRUE)
      {
         rgCb[inst].genSts.numHarqFail++;
      }
     
#ifdef LTE_L2_MEAS
      hqStaInd.cellId = cell->cellId;
      hqStaInd.ueId = rlshqUeInfo->ueHqInfo[idx1].rnti;
      hqStaInd.numTbs = rlshqUeInfo->ueHqInfo[idx1].numOfTBs;
#endif

      for(idx2=0; idx2 < rlshqUeInfo->ueHqInfo[idx1].numOfTBs; idx2++)
      {
#ifdef LTE_L2_MEAS
         /* Fill the hq sta Ind stucture. Need to send the Status Ind for only
          those TBID's reported by Scheduler*/
            tbId = rlshqUeInfo->ueHqInfo[idx1].tbId[idx2];
            if (hqP->tbId[tbId-1] != RGU_INVALID_TBID)
            {
            /* Fill the correct Sn Map corresponding to the TBID */
            hqStaInd.tbId[idx2] = hqP->tbId[tbId-1];
            hqStaInd.status[idx2] = rlshqUeInfo->ueHqInfo[idx1].status[idx2];
               isValidTbId = TRUE;
            }
#endif
         if(rgDHMRlsHqProcTB(cell, hqP, 
               rlshqUeInfo->ueHqInfo[idx1].tbId[idx2]) != ROK)
         {
            RLOG_ARG1(L_ERROR,DBG_CELLID,rlshqUeInfo->cellId,
                  "CRNTI:%d Failure in releasing hq TB",
                  rlshqUeInfo->ueHqInfo[idx1].rnti);
            continue;
         }
      }

#ifdef LTE_L2_MEAS

         if (isValidTbId)
         {
            if(ue->rguDlSap)
            {
               RgUiRguHqStaInd(&(ue->rguDlSap->sapCfg.sapPst),
                     ue->rguDlSap->sapCfg.suId,
                     &hqStaInd);
            }
            else
            {/* Ue is from rach list*/
               RgUiRguHqStaInd(&(cell->rguDlSap->sapCfg.sapPst),
                     cell->rguDlSap->sapCfg.suId,
                     &hqStaInd);
            }
         }
#endif
   } /* end of ues loop */

   /*starting Task*/
   SStopTask(startTime,PID_MAC_AM_HARQ_RLS);

   RETVALUE(ROK);
} /* end of RgSchMacRlsHqReq */


/**********************************************************************
 
         End of file
**********************************************************************/
