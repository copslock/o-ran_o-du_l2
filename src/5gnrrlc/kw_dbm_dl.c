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
  
        Name:    RLC - Database module file
    
        Type:    C source file
  
        Desc:    Source code for Database Module functions such as, 

                 - kwDbmInit
                 - kwDbmDeInit
                 - kwDbmCreateRbCb
                 - kwDbmFetchRbCb 
                 - kwDbmUpdateRbCb
                 - kwDbmDelRbCb
                 - kwDbmDelAllRb
                 - kwDbmCreateUeCb
                 - kwDbmFetchUeCb
                 - kwDbmUpdateUeCb
                 - kwDbmDelUeCb
                 - kwDbmDelAllUe
                 - kwDbmDelMeasEvtCb
                 - kwDbmCreateCellCb
                 - kwDbmFetchCellCb
                 - kwDbmUpdateCellCb
                 - kwDbmDelCellCb
                 - kwDbmDelAllCell
                 - kwDbmShutdown

        File:    kw_dbm_dl.c
  
*********************************************************************21*/
static const char* RLOG_MODULE_NAME="DBM";
static int RLOG_MODULE_ID=2048;
static int RLOG_FILE_ID=193;


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
#include "kw_err.h"        /* Err defines */
#include "kw_env.h"        /* RLC environment options */

#include "kw.h"            /* RLC defines */
#include "kw_udx.h"
#include "kw_dl.h"

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


#ifdef TENB_STATS
EXTERN TSL2UeStatsCb* TSL2MapUeStatsBlk (U16 rnti);
#endif
/** 
 * @file gp_dbm_dl.c
 * @brief RLC Downlink database module
*/
#define KW_MODULE KW_DBGMASK_DUT


/**
 * @brief Handler to initialize hash list
 *       
 * @details
 *    This function initializes the UeCb, CellCb hash lists
 *
 *  @return  S16
 *     -# ROK 
 *     -# RFAILED 
*/
#ifdef ANSI
PUBLIC S16 kwDbmDlInit
(
KwCb *gCb
)
#else
PUBLIC S16 kwDbmDlInit(gCb)
KwCb *gCb;
#endif
{
   TRC3(kwDbmDlInit)

   /* Initialize ueCb Hash List */
   if(ROK != cmHashListInit(&(gCb->u.dlCb->ueLstCp), 
                            (U16) KW_UE_LIST_BUCKET_SIZE,
                            (U16) 0, 
                            (Bool) FALSE, 
                            (U16) CM_HASH_KEYTYPE_CONID,
                            KW_GET_MEM_REGION(gCb), 
                            KW_GET_MEM_POOL(gCb)))
   {   
      RLOG0(L_ERROR, "UeLstCp Initialization Failed");
      RETVALUE(RFAILED);
   }

   /* Initialize cellCb Hash List */
   if(ROK != cmHashListInit(&(gCb->u.dlCb->cellLstCp), 
                            (U16) KW_CELL_LIST_BUCKET_SIZE,
                            (U16) 0, 
                            (Bool) FALSE, 
                            (U16) CM_HASH_KEYTYPE_CONID,
                            KW_GET_MEM_REGION(gCb), 
                            KW_GET_MEM_POOL(gCb)))
   {
      cmHashListDeinit(&gCb->u.dlCb->ueLstCp);
      RLOG0(L_ERROR, "CellLstCp Initialization Failed");
      RETVALUE(RFAILED);
   }

/* kw005.201 added support for L2 Measurement */         
#ifdef LTE_L2_MEAS_RLC
   /* Initialize qcI Hash List */
   if(ROK != cmHashListInit(&(kwCb.kwL2Cb.qciHlCp), 
                            (U16) KW_QCI_LIST_BUCKET_SIZE,
                            (U16) 0, 
                            (Bool) TRUE, 
                            (U16) CM_HASH_KEYTYPE_DEF,
                            kwCb.init.region, 
                            kwCb.init.pool))
   {
      cmHashListDeinit(&gCb->u.dlCb->cellLstCp);
      cmHashListDeinit(&gCb->u.dlCb->ueLstCp);
      RLOG0(L_ERROR, "kwDbmInit: cmHashListInit Failed for kwCb.qciHlCp");
      RETVALUE(RFAILED);
   }
   
   /* Initialize tbHlCp Hash List */
   if(ROK != cmHashListInit(&(kwCb.kwL2Cb.tbHlCp), 
                            (U16) KW_TB_LIST_BUCKET_SIZE,
                            (U16) 0, 
                            (Bool) FALSE, 
                            (U16) CM_HASH_KEYTYPE_DEF,
                            kwCb.init.region, 
                            kwCb.init.pool))
   {
      cmHashListDeinit(&kwCb.kwL2Cb.qciHlCp);
      cmHashListDeinit(&gCb->u.dlCb->cellLstCp);
      cmHashListDeinit(&gCb->u.dlCb->ueLstCp);
      RLOG0(L_ERROR, "kwDbmInit: cmHashListInit Failed for kwCb.tbHlCp");
      RETVALUE(RFAILED);
   }
#endif /* LTE_L2_MEAS */

   RETVALUE(ROK);
} /* kwDbmDlInit */


/**
 * @brief Handler to De initialize hash list
 *       
 * @param[in]    gCb      RLC Instance Control Block
 *
 * @return  Void
*/
#ifdef ANSI
PUBLIC Void kwDbmDlDeInit
(
KwCb *gCb
)
#else
PUBLIC Void kwDbmDlDeInit(gCb)
KwCb *gCb;
#endif
{
   TRC3(kwDbmDlDeInit);


   /* De Initialize ueCb Hash List */
   cmHashListDeinit(&(gCb->u.dlCb->ueLstCp));

   /* De Initialize cellCb Hash List */
   cmHashListDeinit(&(gCb->u.dlCb->cellLstCp));

/* kw005.201 added support for L2 Measurement */         
#ifdef LTE_L2_MEAS_RLC
   /* De Initialize qciCb Hash List */
   cmHashListDeinit(&(kwCb.kwL2Cb.qciHlCp));

   /* De Initialize tbHlCp Hash List */
   cmHashListDeinit(&(kwCb.kwL2Cb.tbHlCp));
   
#endif /* LTE_L2_MEAS */

   RETVOID;
} /* kwDbmDlDeInit */


/**
 * @brief Handler to fetch rbCb by the rlcId
 *       
 * @details
 *    This function is invoked by CFG to fetch rbCb from the cellCb/ueCb in
 *    the upper interface (CKW/KWU).
 *
 *  @param[in]    gCb     RLC Instance Control Block
 *  @param[in]    rlcId   RLC Identifier 
 *  @param[out]   rbCb    RB Control Block
 *
 *  @return  Void
*/
#ifdef ANSI
PUBLIC Void kwDbmFetchDlRbCbByRbId
(
KwCb         *gCb,
CmLteRlcId   *rlcId,
KwDlRbCb     **rbCb
)
#else
PUBLIC Void kwDbmFetchDlRbCbByRbId(gCb, rlcId, rbCb)
KwCb         *gCb;
CmLteRlcId   *rlcId;
KwDlRbCb     **rbCb;
#endif
{
   TRC3(kwDbmFetchDlRbCbByRbId)

   *rbCb= NULLP;
   
   /* Check for UE CB or CELL CB */
   if (rlcId->ueId == 0)
   {
      KwDlCellCb *cellCb;
      
      if(rlcId->rbId >= KW_MAX_RB_PER_CELL)
      {
         RLOG_ARG3(L_ERROR,DBG_RBID,rlcId->rbId ,
               "Invalid RbId, Max is [%d] UEID:%d CELLID:%d",
               KW_MAX_RB_PER_CELL,
               rlcId->ueId,
               rlcId->cellId);
         RETVOID;
      }

      kwDbmFetchDlCellCb(gCb,rlcId->cellId, &cellCb);
      if(!cellCb)
      {
         RLOG_ARG2(L_ERROR,DBG_CELLID,rlcId->cellId,
               "CellCb not found UEID:%d RBID:%d",
               rlcId->ueId, 
               rlcId->rbId);
         RETVOID;
      }

      *rbCb = cellCb->rbCb[rlcId->rbId];
   }
   else
   {
      KwDlUeCb *ueCb;
      if (!(KW_VALIDATE_UE_RBID(rlcId->rbType, rlcId->rbId)))
      {
         RLOG_ARG3(L_ERROR,DBG_RBID, rlcId->rbId,
               "Invalid RbId for RbType[%d] RBID:%d CELLID:%d",
               rlcId->rbType,
               rlcId->ueId,
               rlcId->cellId);
         RETVOID;
      }
      
      if (ROK != kwDbmFetchDlUeCb(gCb,rlcId->ueId, rlcId->cellId, &ueCb))
      {
         RLOG_ARG2(L_ERROR,DBG_CELLID, rlcId->cellId,
               "UeId [%d]: UeCb not found RBID:%d",
               rlcId->ueId,
               rlcId->rbId);
         RETVOID;
      }

      KW_DBM_GET_RBCB_FROM_UECB(rlcId->rbId, rlcId->rbType, ueCb, *rbCb);
   }

   RETVOID;
} /* kwDbmFetchDlRbCbByRbId */


/**
 * @brief Handler to fetch rbCb from Local Channel Id 
 *
 * @details
 *    This function is invoked by CFG to fetch rbCb from the cellCb/ueCb
 *    from local Channel Id (CKW/KWU).
 *
 * @param[in]    gCb      -  RLC Instance Control Block
 * @param[in]    cellId   -  CELL Identifier 
 * @param[in]    ueId     -  UE Identifier 
 * @param[in]    lcId     -  Logical Channel Identifier
 * @param[out]   rbCb     -  RB Control Block
 *
 * @return  Void
 *
*/
#ifdef ANSI
PUBLIC Void kwDbmFetchDlRbCbFromLchId
(
KwCb          *gCb,
CmLteRnti     ueId,  
CmLteCellId   cellId,
CmLteLcId     lcId,  
KwDlRbCb      **rbCb
)
#else
PUBLIC Void kwDbmFetchDlRbCbFromLchId(gCb, ueId, cellId, lcId, rbCb)
KwCb          *gCb;
CmLteRnti     ueId;  
CmLteCellId   cellId;
CmLteLcId     lcId; 
KwDlRbCb      **rbCb; 
#endif
{
   KwDlUeCb *ueCb;
   
   TRC3(kwDbmFetchDlRbCbFromLchId)


   /* Check for UE CB or CELL CB */
   if (ueId == 0)
   {
      KwDlCellCb *cellCb;
      
      kwDbmFetchDlCellCb(gCb, cellId, &cellCb);
      if(!cellCb)
      {
         RLOG_ARG1(L_ERROR,DBG_CELLID,cellId,"CellCb not found UEID:%d",ueId);
         RETVOID;
      }

      *rbCb = cellCb->lCh[lcId - 1].dlRbCb;
      RETVOID;
   }

   if (kwDbmFetchDlUeCb(gCb, ueId, cellId, &ueCb) != ROK)
   {
      RLOG_ARG1(L_ERROR,DBG_CELLID, cellId,"UeId [%d]: UeCb not found",ueId);
      RETVOID;
   }

   *rbCb = ueCb->lCh[lcId - 1].dlRbCb;

   RETVOID;
} /* kwDbmFetchDlRbCbFromLchId */


/**
 * @brief Handler to delete RbCb
 *       
 * @details 
 *    This function is invoked by CFG to remove RbCb from Ue/Cell hashlist 
 *
 * @param[in] gCb       - RLC Instance Control Block
 * @param[in] rbCbLst   - Rb Cb List
 * @param[in] numRbCb   - Number of rbCbs
 *
 * @return  Void
 *
*/
#ifdef ANSI
PUBLIC Void kwDbmDelAllDlRb
(
KwCb       *gCb,
KwDlRbCb   **rbCbLst,
U8         numRbCb 
)
#else
PUBLIC Void kwDbmDelAllDlRb(gCb, rbCbLst, numRbCb)
KwCb       *gCb;
KwDlRbCb   **rbCbLst;
U8         numRbCb;   
#endif
{
   U32 idx;

   TRC3(kwDbmDelAllDlRb)


   for (idx = 0; idx < numRbCb; idx++)
   {
      if (rbCbLst[idx] != NULLP)
      {
#ifdef LTE_L2_MEAS
         KW_UPD_L2_DECR_NONIP_PER_QCI_RB_COUNT(gCb, (rbCbLst[idx]));
#endif
         if( CM_LTE_MODE_UM == rbCbLst[idx]->mode)
         {
            kwUmmFreeDlRbCb(gCb,rbCbLst[idx]);

            KW_FREE (gCb,rbCbLst[idx], sizeof (KwDlRbCb));       
         }
         else if( CM_LTE_MODE_AM == rbCbLst[idx]->mode)
         {
            kwAmmFreeDlRbCb(gCb,rbCbLst[idx]);
         }
         /* ccpu00136940 */
         else if(CM_LTE_MODE_TM == rbCbLst[idx]->mode)
         {
            cmLListCatLList(&(gCb->u.dlCb->toBeFreed.sduLst),&(rbCbLst[idx]->m.tm.sduQ));
            KW_FREE (gCb,rbCbLst[idx], sizeof (KwDlRbCb));       
         }

      }

   }

   RETVOID;
} /* kwDbmDelAllDlRb */


/**
 * @brief  Handler to create an UeCb
 *
 * @details  
 *    This function is invoked by CFG to create UeCb and insert into the 
 *    Ue hashlist of KwCb.
 *
 * @param[in] gCb       RLC Instance Control Block
 * @param[in] ueId      UE Identifier 
 * @param[in] cellId    Cell Identifier 
 * @param[in] ueCb      UE Control Block 
 *
 * @return  S16
 *    -# ROK 
 *    -# RFAILED 
 *
*/
#ifdef ANSI
PUBLIC S16 kwDbmCreateDlUeCb
(
KwCb          *gCb,
CmLteRnti     ueId,  
CmLteCellId   cellId,
KwDlUeCb      **ueCb 
)
#else
PUBLIC S16 kwDbmCreateDlUeCb(gCb,ueId, cellId, ueCb)
KwCb          *gCb;
CmLteRnti     ueId;
CmLteCellId   cellId;
KwDlUeCb      **ueCb;
#endif
{
   KwDlUeCb *tUeCb;        

   TRC3(kwDbmCreateDlUeCb)


   KW_ALLOC(gCb,*ueCb, sizeof(KwDlUeCb));

#if (ERRCLASS & ERRCLS_ADD_RES)
   if (*ueCb == NULLP)
   {   
      RLOG_ARG1(L_FATAL,DBG_UEID,ueId,
            "Memory allocation failed cellID:%d",
            cellId);
      RETVALUE(RFAILED);
   }
#endif /* ERRCLASS & ERRCLS_ADD_RES */

   tUeCb = *ueCb;
   tUeCb->ueId = ueId;
   tUeCb->cellId = cellId;

   if (ROK != cmHashListInsert(&(gCb->u.dlCb->ueLstCp), 
                               (PTR)tUeCb, 
                               (U8 *)&(tUeCb->ueId),
                               (U16) sizeof(CmLteRnti)))
   {
      RLOG_ARG1(L_ERROR,DBG_CELLID,cellId,
            "UeId[%u] HashList Insertion Failed",
            ueId);
      RETVALUE(RFAILED);
   }
   /* kw005.201 ccpu00117318, updating the statistics */
   gCb->genSts.numUe++;


   RETVALUE(ROK);
} /* kwDbmCreateUeCb */


/**
 * @brief Handler to Fetch an UeCb
 *
 * @details
 *    This function is invoked by CFG to fetch UeCb from the Ue hashlist 
 *    of KwCb.
 *
 * @param[in]    gCb       RLC Instance Control Block 
 * @param[in]    ueId      UE Identifier 
 * @param[in]    cellId    Cell Identifier 
 * @param[out]   ueCb      UE Control Block
 *
 * @return  S16
 *    -# ROK 
 *    -# RFAILED 
*/
#ifdef ANSI
PUBLIC S16 kwDbmFetchDlUeCb
(
KwCb          *gCb,
CmLteRnti     ueId, 
CmLteCellId   cellId,
KwDlUeCb      **ueCb
)
#else
PUBLIC S16 kwDbmFetchDlUeCb(gCb,ueId, cellId, ueCb)
KwCb          *gCb;
CmLteRnti     ueId;  
CmLteCellId   cellId;
KwDlUeCb      **ueCb;
#endif
{

   TRC3(kwDbmFetchDlUeCb)


   RETVALUE(cmHashListFind(&(gCb->u.dlCb->ueLstCp), 
                           (U8 *)&(ueId), 
                           sizeof(CmLteRnti),
                           KW_DEF_SEQ_NUM, 
                           (PTR *) ueCb));
} /* kwDbmFetchDlUeCb */


/**
 * @brief Handler to delete an UeCb
 *
 * @details
 *    This function is invoked by CFG to delete UeCb from the Ue hashlist 
 *    of KwCb.
 *
 *
 * @param[in] gCb      RLC Instance Control Block
 * @param[in] ueCb     UE Control Block
 * @param[in] abrtFlag Abort Flag
 *
 * @return Void
 *
*/
#ifdef ANSI
PUBLIC Void kwDbmDelDlUeCb
(
KwCb       *gCb,
KwDlUeCb   *ueCb,   
Bool       abortFlag 
)
#else
PUBLIC Void kwDbmDelDlUeCb(gCb,eCb, abortFlag)
KwCb       *gCb;
KwDlUeCb   *ueCb; 
Bool       abortFlag;
#endif
{
   TRC3(kwDbmDelDlUeCb)


#if  (!defined(KW_PDCP) || !(defined(PJ_SEC_ASYNC) || defined(PJ_CMP_ASYNC)))
   UNUSED(abortFlag);
#endif /* (!defined(KW_PDCP) || ! (defined(PJ_SEC_ASYNC) || 
          defined(PJ_CMP_ASYNC)))*/

   /* Delete all logical channels */
   KW_MEM_ZERO(ueCb->lCh,sizeof(KwDlLch) * KW_MAX_LCH_PER_UE);

   /* Delete all SRB RbCbs in UeCb */
   kwDbmDelAllDlRb(gCb,ueCb->srbCb, KW_MAX_SRB_PER_UE);

   /* Delete all DRB RbCbs in UeCb */
   kwDbmDelAllDlRb(gCb,ueCb->drbCb, KW_MAX_DRB_PER_UE);

   /* Delete ueCb entry from ueLstCp */
   if(ROK != cmHashListDelete(&(gCb->u.dlCb->ueLstCp), (PTR) ueCb))
   {
      RLOG_ARG1(L_ERROR,DBG_CELLID,ueCb->cellId,
            "UeId[%u] HashList Insertion Failed",
            ueCb->ueId);
   }
   /* kw005.201 ccpu00117318, updating the statistics */
   gCb->genSts.numUe--;
   
   /* Deallocate ueCb */
   KW_FREE(gCb,ueCb, sizeof(KwDlUeCb));

   RETVOID;
} /* kwDbmDelUeCb */


/**
 * @brief Handler to delete all UeCbs
 *
 * @details
 *    This function is invoked by CFG to delete all UeCbs from the Ue 
 *    hashlist of KwCb.
 *
 * @param[in] gCb      RLC Instance Control Block
 *
 * @return  Void
*/
#ifdef ANSI
PUBLIC Void kwDbmDelAllDlUe
(
KwCb  *gCb
)
#else
PUBLIC Void kwDbmDelAllDlUe(gCb)
KwCb  *gCb;
#endif
{
   KwDlUeCb *ueCb = NULLP; 

   TRC3(kwDbmDelAllDlUe)


   /* Until no more ueCb is ueLstCp hash list get and delete ueCb */
   while (ROK == cmHashListGetNext(&(gCb->u.dlCb->ueLstCp), 
                                   (PTR) ueCb, 
                                   (PTR *)&ueCb))
   {
      /* Delete ueCb */
      kwDbmDelDlUeCb(gCb,ueCb, TRUE);

      ueCb = NULLP;
   }

   RETVOID;
} /* kwDbmDelAllUe */

/* kw005.201 added support for L2 Measurement */         
#ifdef LTE_L2_MEAS
#ifdef ANSI
PUBLIC Void kwDbmDelAllDlL2MeasTbFrmUe
(
KwCb      *gCb,
KwDlUeCb  *ueCb
)
#else
PUBLIC Void kwDbmDelAllDlL2MeasTbFrmUe(gCb,ueCb)
KwCb      *gCb;
KwDlUeCb  *ueCb;
#endif
{
   U8           tbIdx;
   KwL2MeasTb   *l2MeasTb = NULLP;
   for(tbIdx = 0; tbIdx < KW_MAX_TB_PER_UE; tbIdx++)
   { 
       l2MeasTb = ueCb->l2MeasTbCb[tbIdx];
       if(l2MeasTb != NULLP)
       {
          KW_FREE(gCb,l2MeasTb, sizeof(KwL2MeasTb));
          ueCb->l2MeasTbCb[tbIdx] = NULLP;
       }       
   }  
   RETVOID;   
}/* End of kwDbmDelL2MeasTb */
#endif /* LTE_L2_MEAS */

/**
 * @brief Handler to create CellCb
 *
 * @details
 *    This function is invoked by CFG to create CellCb and insert into
 *    the Cell hashlist of KwCb.
 *
 *  @param[in] gCb      RLC Instance Control Block
 *  @param[in] cellId   Cell Identifier 
 *  @param[in] cellCb   Cell Control Block 
 *
 *  @return  S16
 *     -# ROK 
 *     -# RFAILED 
*/
#ifdef ANSI
PUBLIC S16 kwDbmCreateDlCellCb
(
KwCb          *gCb,
CmLteCellId   cellId, 
KwDlCellCb    **cellCb 
)
#else
PUBLIC S16 kwDbmCreateDlCellCb(gCb,cellId, cellCb)
KwCb          *gCb;
CmLteCellId   cellId;   
KwDlCellCb    **cellCb;
#endif
{
   KwDlCellCb *tCellCb; 
   
   TRC3(kwDbmCreateDlCellCb)

   KW_ALLOC(gCb,*cellCb, sizeof(KwDlCellCb));
#if (ERRCLASS & ERRCLS_ADD_RES)
   if (*cellCb == NULLP)
   {
      RLOG_ARG0(L_FATAL, DBG_CELLID,cellId,"Memory allocation failed");
      RETVALUE(RFAILED);
   }
#endif /* ERRCLASS & ERRCLS_ADD_RES */

   tCellCb = *cellCb;
   tCellCb->cellId = cellId;

   if(ROK != cmHashListInsert(&(gCb->u.dlCb->cellLstCp), 
                              (PTR) tCellCb,
                              (U8 *)&(tCellCb->cellId), 
                              (U16) sizeof(CmLteCellId)))
   {
      RLOG_ARG0(L_ERROR,DBG_CELLID,cellId,"HashList Insertion Failed");
      RETVALUE(RFAILED);
   }

   RETVALUE(ROK);
} /* kwDbmCreateDlCellCb */


/**
 * @brief Handler to Fetch an CellCb
 *       
 * @details
 *    This function is invoked by CFG to fetch UeCb from the Ue hashlist 
 *    of KwCb.
 *
 * @param[in]    gCb      RLC Instance Control Block
 * @param[in]    cellId   Cell Identifier 
 * @param[out]   cellCb   UE Control Block
 *
 * @return  S16
 *    -# ROK 
 *    -# RFAILED 
 *
*/
#ifdef ANSI
PUBLIC S16 kwDbmFetchDlCellCb
(
KwCb          *gCb,
CmLteCellId   cellId, 
KwDlCellCb    **cellCb
)
#else
PUBLIC S16 kwDbmFetchDlCellCb(gCb,cellId, cellCb)
KwCb          *gCb;
CmLteCellId   cellId;  
KwDlCellCb    **cellCb;
#endif
{
   TRC3(kwDbmFetchDlCellCb)


   *cellCb = NULLP;
   
   if(ROK != cmHashListFind(&(gCb->u.dlCb->cellLstCp), 
                            (U8 *)&(cellId),
                            sizeof(CmLteCellId), 
                            KW_DEF_SEQ_NUM, 
                            (PTR*) cellCb))
   {
      RLOG_ARG0(L_ERROR,DBG_CELLID, cellId,"CellCb not found");
      RETVALUE(RFAILED);
   }

   RETVALUE(ROK);
} /* kwDbmFetchCellCb */


/**
 * @brief Handler to delete CellCb
 *
 * @details
 *    This function is invoked by CFG to delete CellCb from the Cell hashlist 
 *    of KwCb.
 *
 *  @param[in] gCb      RLC Instance Control Block
 *  @param[in] cellCb   Cell Control Block
 *
 *  @return  Void
*/
#ifdef ANSI
PUBLIC Void kwDbmDelDlCellCb
(
KwCb         *gCb,
KwDlCellCb   *cellCb  
)
#else
PUBLIC Void kwDbmDelDlCellCb(gCb,cellCb)
KwCb         *gCb;
KwDlCellCb   *cellCb;
#endif
{
   TRC3(kwDbmDelDlCellCb)


   /* Delete all rbCbs in cellCb */
   kwDbmDelAllDlRb(gCb,cellCb->rbCb, KW_MAX_RB_PER_CELL);
   
   /* Delete cellCb entry in hash list cellLstCp */
   if(ROK != cmHashListDelete(&(gCb->u.dlCb->cellLstCp), (PTR) cellCb))
   {
      RLOG_ARG0(L_ERROR,DBG_CELLID,cellCb->cellId,"HashList Deletion Failed");
   }

   /* Deallocate cellCb */
   KW_FREE(gCb, cellCb, sizeof(KwDlCellCb));

   RETVOID;
} /* kwDbmDelCellCb */


/**
 * @brief Handler to delete all UeCbs
 *       
 * @details
 *    This function is invoked by CFG to delete all UeCbs from the Ue
 *    hashlist of KwCb.
 * @param[in] gCb      RLC Instance Control Block
 *
 * @return  Void
*/
#ifdef ANSI
PUBLIC Void kwDbmDelAllDlCell
(
KwCb *gCb
)
#else
PUBLIC Void kwDbmDelAllDlCell(gCb)
KwCb *gCb;
#endif
{
   KwDlCellCb *cellCb = NULLP;

   TRC3(kwDbmDelAllDlCell)


   /* Until no more cellCb is ueLstCp hash list get and delete cellCb */
   while (ROK == cmHashListGetNext(&(gCb->u.dlCb->cellLstCp), 
                                   (PTR) cellCb, 
                                   (PTR *)&cellCb))
   {
      kwDbmDelDlCellCb(gCb, cellCb);
      
      cellCb = NULLP;
   }

   RETVOID;
} /* kwDbmDelAllDlCell */


/**
 * @brief Handler to delete all UeCbs and CellCbs
 *       
 * @details
 *    This function is invoked by LMM to delete all UeCbs from the Ue
 *    hashlist of KwCb and cellCbs from the Cell hashlist of kwCb.
 * 
 * @param[in] gCb      RLC Instance Control Block
 *
 * @return  Void
*/
#ifdef ANSI
PUBLIC S16 kwDbmDlShutdown
(
KwCb *gCb
)
#else
PUBLIC S16 kwDbmDlShutdown(gCb)
KwCb *gCb;
#endif
{
   TRC3(kwDbmDlShutdown)

   kwDbmDelAllDlCell(gCb);

   kwDbmDelAllDlUe(gCb);

   kwDbmDlDeInit(gCb);


   RETVALUE(ROK);
} /* kwDbmShutdown */


/********************************************************************30**
  
         End of file
**********************************************************************/
