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
  
     File:     rg_dbm.c
  
**********************************************************************/

/** @file rg_dbm.c
@brief This file contains the APIs exposed for the insertion/fetching/deletion of cellCb/ueCb and traversal of LcCbLst.
*/

/* header include files -- defines (.h) */
#include "envopt.h"        /* environment options */
#include "envdep.h"        /* environment dependent */
#include "envind.h"        /* environment independent */
#include "gen.h"           /* general layer */
#include "ssi.h"           /* system service interface */
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
#include "rg.h"

/* header/extern include files (.x) */
#include "gen.x"           /* general layer */
#include "ssi.x"           /* system service interface */
#include "cm5.x"           /* common timers */
#include "cm_lib.x"        /* common library */
#include "cm_hash.x"       /* common hash list */
#include "cm_mblk.x"       /* common memory link list library */
#include "cm_llist.x"      /* common linked list library */
#include "cm_tkns.x"       /* common tokens */
#include "cm_lte.x"        /* common LTE */
#include "lrg.x"
#include "crg.x"
#include "rgu.x"
#include "tfu.x"
#include "rg_sch_inf.x"
#include "rg_prg.x"       /* PRG interface typedefs*/
#include "rg.x"


/* local defines */
PRIVATE S16 rgDBMInitUeCbLst ARGS(( RgCellCb *cellCb, U16 numBins));
PRIVATE Void rgDBMInitDedLcLst ARGS((RgUeCb *ueCb));
PRIVATE Void rgDBMInitCmnLcLst ARGS((RgCellCb *cellCb));
PRIVATE Void rgDBMInitRachLst ARGS((RgCellCb *cellCb));
#ifdef LTEMAC_SPS
PRIVATE S16 rgDBMInitSpsUeCbLst ARGS((
RgCellCb       *cellCb,
U16            numBins
));
#endif

/* local typedefs */
 
/* local externs */
 
/* forward references */

/**
 * @brief Handler for Initializing the cell.
 *
 * @details
 *
 *     Function : rgDBMInitCell
 *     
 *   Initializes the lists belonging to the cell.
 *     
 *           
 *  @param[in]  RgCellCb *cellCb
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED
 **/
#ifdef ANSI
PUBLIC S16 rgDBMInitCell
(
RgCellCb       *cellCb
)
#else
PUBLIC S16 rgDBMInitCell(cellCb)
RgCellCb       *cellCb;
#endif
{
   S16 ret;
   
   TRC2(rgDBMInitCell);

   /* Initialize ue list */
   if ((ret = rgDBMInitUeCbLst(cellCb, RG_MAX_UE_BIN_PER_CELL)) != ROK)
      RETVALUE(ret);

#ifdef LTEMAC_SPS
   /* Initialize SPS Ue list */
   if ((ret = rgDBMInitSpsUeCbLst(cellCb, RG_MAX_UE_BIN_PER_CELL)) != ROK)
      RETVALUE(ret);
#endif /* LTEMAC_SPS */

   /* Initialize BCCH/PCCH logical channels */
   rgDBMInitCmnLcLst(cellCb);


   /* Initialize rach ue list */
   rgDBMInitRachLst(cellCb);

   RETVALUE(ret);

} /* rgDBMInitCell */

/**
 * @brief Handler for initializing the ueCbLst under the cellCb.
 *
 * @details
 *
 *     Function : rgDBMInitUeCbLst
 *     
 *           
 *  @param[in] *cellCb 
 *  @param[in] numBins 
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PRIVATE S16 rgDBMInitUeCbLst
(
RgCellCb       *cellCb,
U16            numBins
)
#else
PRIVATE S16 rgDBMInitUeCbLst(cellCb, numBins)
RgCellCb       *cellCb;
U16            numBins;
#endif
{
   Inst inst = cellCb->macInst - RG_INST_START;
   RgUeCb ue;  
   TRC2(rgDBMInitUeCbLst)

   RETVALUE(cmHashListInit(&cellCb->ueLst, numBins, (U16)((PTR)&(ue.ueLstEnt) - (PTR)&ue), FALSE, 
               CM_HASH_KEYTYPE_CONID, rgCb[inst].rgInit.region, rgCb[inst].rgInit.pool));

}  /* rgDBMInitUeCbLst */

/**
 * @brief Handler for de-initializing the ueCbLst under the cellCb.
 *
 * @details
 *
 *     Function : rgDBMDeInitUeCbLst
 *     
 *           
 *  @param[in] *cellCb 
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PUBLIC S16 rgDBMDeInitUeCbLst
(
RgCellCb       *cellCb
)
#else
PUBLIC S16 rgDBMDeInitUeCbLst(cellCb)
RgCellCb       *cellCb;
#endif
{
   TRC2(rgDBMDeInitUeCbLst)

   RETVALUE(cmHashListDeinit(&cellCb->ueLst));

}  /* rgDBMDeInitUeCbLst */

#ifdef LTEMAC_SPS
/**
 * @brief Handler for initializing the spsUeCbLst under the cellCb.
 *
 * @details
 *
 *     Function : rgDBMInitSpsUeCbLst
 *     
 *           
 *  @param[in] *cellCb 
 *  @param[in] numBins 
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PRIVATE S16 rgDBMInitSpsUeCbLst
(
RgCellCb       *cellCb,
U16            numBins
)
#else
PRIVATE S16 rgDBMInitSpsUeCbLst(cellCb, numBins)
RgCellCb       *cellCb;
U16            numBins;
#endif
{
   Inst inst = cellCb->macInst - RG_INST_START;
   RgUeCb ue;
   TRC2(rgDBMInitSpsUeCbLst)

   RETVALUE(cmHashListInit(&cellCb->spsUeLst, numBins, (U16) ((PTR) &(ue.spsUeLstEnt) - (PTR) &ue), FALSE, 
               CM_HASH_KEYTYPE_CONID, rgCb[inst].rgInit.region, rgCb[inst].rgInit.pool));

}  /* rgDBMInitSpsUeCbLst */

/**
 * @brief Handler for de-initializing the spsUeCbLst under the cellCb.
 *
 * @details
 *
 *     Function : rgDBMDeInitSpsUeCbLst
 *     
 *           
 *  @param[in] *cellCb 
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PUBLIC S16 rgDBMDeInitSpsUeCbLst
(
RgCellCb       *cellCb
)
#else
PUBLIC S16 rgDBMDeInitSpsUeCbLst(cellCb)
RgCellCb       *cellCb;
#endif
{
   TRC2(rgDBMDeInitSpsUeCbLst)

   RETVALUE(cmHashListDeinit(&cellCb->spsUeLst));

}  /* rgDBMDeInitSpsUeCbLst */

#endif /* LTEMAC_SPS */

/**
 * @brief Handler for inserting the ueCb in to the ueCbLst under the cellCb.
 *
 * @details
 *
 *     Function : rgDBMInsUeCb
 *     
 *           
 *  @param[in] *cellCb 
 *  @param[in] *ueCb 
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PUBLIC S16 rgDBMInsUeCb
(
RgCellCb       *cellCb,
RgUeCb         *ueCb
)
#else
PUBLIC S16 rgDBMInsUeCb(cellCb, ueCb)
RgCellCb       *cellCb;
RgUeCb         *ueCb;
#endif
{
   TRC2(rgDBMInsUeCb)

   RETVALUE(cmHashListInsert(&cellCb->ueLst, (PTR)ueCb, 
      (U8 *)&ueCb->ueId, (U16)sizeof(ueCb->ueId)));

}  /* rgDBMInsUeCb */

#ifdef LTEMAC_SPS
/**
 * @brief Handler for inserting the ueCb in to the spsUeCbLst under the cellCb.
 *
 * @details
 *
 *     Function : rgDBMInsSpsUeCb
 *     
 *           
 *  @param[in] *cellCb 
 *  @param[in] *ueCb 
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PUBLIC S16 rgDBMInsSpsUeCb
(
RgCellCb       *cellCb,
RgUeCb         *ueCb
)
#else
PUBLIC S16 rgDBMInsSpsUeCb(cellCb, ueCb)
RgCellCb       *cellCb;
RgUeCb         *ueCb;
#endif
{
   TRC2(rgDBMInsSpsUeCb)

   RETVALUE(cmHashListInsert(&cellCb->spsUeLst, (PTR)ueCb, 
      (U8 *)&ueCb->spsRnti, (U16)sizeof(ueCb->spsRnti)));

}  /* end of rgDBMInsSpsUeCb */

#endif /* LTEMAC_SPS */

/**
 * @brief Handler for accessing the existing ueCb identified by the key ueId
 * in the ueCbLst under the cellCb.
 *
 * @details
 *
 *     Function : rgDBMGetUeCb
 *     
 *           
 *  @param[in]  *cellCb
 *  @param[in]  ueId
 *  @return  RgUeCb*
 **/
#ifdef ANSI
PUBLIC RgUeCb* rgDBMGetUeCb
(
RgCellCb       *cellCb, 
CmLteRnti      ueId
)
#else
PUBLIC RgUeCb* rgDBMGetUeCb(cellCb, ueId)
RgCellCb       *cellCb;
CmLteRnti      ueId;
#endif
{
   RgUeCb *ueCb = NULLP; 

   TRC2(rgDBMGetUeCb)

   cmHashListFind(&cellCb->ueLst, (U8 *)&ueId,
      sizeof(ueId), 0, (PTR *)&ueCb);
   RETVALUE(ueCb);
}  /* rgDBMGetUeCb */

#ifdef LTEMAC_SPS
/**
 * @brief Handler for accessing the ueCb identified by the key sps-rnti
 * in the spsUeLst under the cellCb.
 *
 * @details
 *
 *     Function : rgDBMGetSpsUeCb
 *     
 *           
 *  @param[in]  *cellCb
 *  @param[in]  ueId
 *  @return  RgUeCb*
 **/
#ifdef ANSI
PUBLIC RgUeCb* rgDBMGetSpsUeCb
(
RgCellCb       *cellCb, 
CmLteRnti      spsRnti
)
#else
PUBLIC RgUeCb* rgDBMGetSpsUeCb(cellCb, spsRnti)
RgCellCb       *cellCb;
CmLteRnti      spsRnti;
#endif
{
   RgUeCb *ueCb = NULLP; 

   TRC2(rgDBMGetSpsUeCb)

   cmHashListFind(&cellCb->spsUeLst, (U8 *)&spsRnti,
      sizeof(spsRnti), 0, (PTR *)&ueCb);
   RETVALUE(ueCb);
}  /* rgDBMGetSpsUeCb */

#endif /* LTEMAC_SPS */

/**
 * @brief Handler for accessing the existing next ueCb in the ueCbLst under the
 *  cellCb.
 *
 * @details
 *
 *     Function : rgDBMGetNextUeCb
 *     
 *           
 *  @param[in]  *cellCb
 *  @param[in]  *ueCb
 *  @return  RgUeCb*
 **/
#ifdef ANSI
PUBLIC RgUeCb* rgDBMGetNextUeCb
(
RgCellCb       *cellCb, 
RgUeCb         *ueCb
)
#else
PUBLIC RgUeCb* rgDBMGetNextUeCb(cellCb, ueCb)
RgCellCb       *cellCb;
RgUeCb         *ueCb;
#endif
{
   RgUeCb *nextUeCb = NULLP; 

   TRC2(rgDBMGetNextUeCb)

   cmHashListGetNext(&cellCb->ueLst, (PTR) ueCb, (PTR *)&nextUeCb);
   RETVALUE(nextUeCb);
}  /* rgDBMGetNextUeCb */

#ifdef LTEMAC_SPS
/**
 * @brief Handler for accessing the existing next ueCb stored in the spsUeCbLst
 *         using SPS-Rnti under the cellCb.
 *
 * @details
 *
 *     Function : rgDBMGetNextSpsUeCb
 *     
 *           
 *  @param[in]  *cellCb
 *  @param[in]  *ueCb
 *  @return  RgUeCb*
 **/
#ifdef ANSI
PUBLIC RgUeCb* rgDBMGetNextSpsUeCb
(
RgCellCb       *cellCb, 
RgUeCb         *ueCb
)
#else
PUBLIC RgUeCb* rgDBMGetNextSpsUeCb(cellCb, ueCb)
RgCellCb       *cellCb;
RgUeCb         *ueCb;
#endif
{
   RgUeCb *nextUeCb = NULLP; 

   TRC2(rgDBMGetNextSpsUeCb)

   cmHashListGetNext(&cellCb->spsUeLst, (PTR) ueCb, (PTR *)&nextUeCb);
   RETVALUE(nextUeCb);
}  /* end of rgDBMGetNextSpsUeCb */

#endif /* LTEMAC_SPS */


/**
 * @brief Handler for deleting the existing ueCb from the ueCbLst under the
 * cellCb.
 *
 * @details
 *
 *     Function : rgDBMDelUeCb
 *     
 *           
 *  @param[in]  *cellCb 
 *  @param[in]  *ueCb 
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PUBLIC S16 rgDBMDelUeCb
(
RgCellCb       *cellCb,
RgUeCb         *ueCb
)
#else
PUBLIC S16 rgDBMDelUeCb(cellCb, ueCb)
RgCellCb       *cellCb;
RgUeCb         *ueCb;
#endif
{
   TRC2(rgDBMDelUeCb)

   RETVALUE(cmHashListDelete(&cellCb->ueLst, (PTR)ueCb));
}  /* rgDBMDelUeCb */

#ifdef LTEMAC_SPS
/**
 * @brief Handler for deleting the existing ueCb from the spsUeCbLst under the
 * cellCb.
 *
 * @details
 *
 *     Function : rgDBMDelSpsUeCb
 *     
 *           
 *  @param[in]  *cellCb 
 *  @param[in]  *ueCb 
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PUBLIC S16 rgDBMDelSpsUeCb
(
RgCellCb       *cellCb,
RgUeCb         *ueCb
)
#else
PUBLIC S16 rgDBMDelSpsUeCb(cellCb, ueCb)
RgCellCb       *cellCb;
RgUeCb         *ueCb;
#endif
{
   TRC2(rgDBMDelSpsUeCb)

   RETVALUE(cmHashListDelete(&cellCb->spsUeLst, (PTR)ueCb));
}  /* end of rgDBMDelSpsUeCb */

#endif /* LTEMAC_SPS */

/**
 * @brief Handler for Initializing the UE.
 *
 * @details
 *
 *     Function : rgDBMInitUe
 *     
 *   Initializes the lists belonging to the UE.
 *     
 *           
 *  @param[in]  RgUeCb  *ueCb
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED
 **/
#ifdef ANSI
PUBLIC Void rgDBMInitUe
(
RgUeCb       *ueCb
)
#else
PUBLIC Void rgDBMInitUe(ueCb)
RgUeCb       *ueCb;
#endif
{
#ifdef LTEMAC_SPS
   U8        idx;
#endif

   TRC2(rgDBMInitUe);

   /* Initialize Dedicated logical channels */
   rgDBMInitDedLcLst(ueCb);

#ifdef LTEMAC_SPS
   /* Initialize the SPS LC Ids */
   for (idx=0; idx < RG_MAX_LC_PER_UE; idx++)
   {
      ueCb->ul.spsLcId[idx] = FALSE;
   }
#endif


   RETVOID;
} /* rgDBMInitUe */

/**
 * @brief Handler for Initializing the dedicated logical channels.
 *
 * @details
 *
 *     Function : rgDBMInitDedLcLst
 *     
 *   Initializes dedicated logical channels.
 *           
 *  @param[in]  RgUeCb *ueCb
 *  @return     Void 
 **/
#ifdef ANSI
PRIVATE Void rgDBMInitDedLcLst
(
RgUeCb       *ueCb
)
#else
PRIVATE Void rgDBMInitDedLcLst(ueCb)
RgUeCb       *ueCb;
#endif
{
   U8 idx;
   
   TRC2(rgDBMInitDedLcLst);

   for (idx = 0; idx < RG_MAX_LC_PER_UE; ++idx)
   {
      /* Set Dedicated LCs as not configured */
      ueCb->ul.lcCb[idx].lcId = RG_INVALID_LC_ID;
      ueCb->ul.lcCb[idx].lcgId = RG_INVALID_LCG_ID;
      ueCb->dl.lcCb[idx].lcId = RG_INVALID_LC_ID;
   }

   for (idx = 0; idx < RG_MAX_LCG_PER_UE; ++idx)
   {
      /* Set LCGs as not configured */
      ueCb->ul.lcgArr[idx].lcgId = RG_INVALID_LCG_ID;
      ueCb->ul.lcgArr[idx].lcCount = 0;
   }

   /* Stack Crash problem for TRACE5 Changes. Added the return below */
  RETVOID;
} /* rgDBMInitDedLcLst */

/**
 * @brief Handler for Initializing the common logical channel list of the cell.
 *
 * @details
 *
 *     Function : rgDBMInitCmnLcLst
 *     
 *   Initializes following common logical channels belonging to the cell.
 *     - BCCH on BCH
 *     - BCCH on DLSCH
 *     - PCCH
 *           
 *  @param[in]  RgCellCb *cellCb
 *  @return  Void
 **/
#ifdef ANSI
PRIVATE Void rgDBMInitCmnLcLst
(
RgCellCb       *cellCb
)
#else
PRIVATE Void rgDBMInitCmnLcLst(cellCb)
RgCellCb       *cellCb;
#endif
{
   U8 idx;
   
   TRC2(rgDBMInitCmnLcLst);
   
   cellCb->bcchBchInfo.lcId = RG_INVALID_LC_ID;
   cellCb->pcchInfo.lcId = RG_INVALID_LC_ID;

   cellCb->numBcchDlschInfo = 0;
   for (idx = 0; idx < RG_MAX_BCCH_DLSCH; idx++)
   {
      cellCb->bcchDlschInfo[idx].lcId = RG_INVALID_LC_ID;
      cellCb->bcchDlschInfo[idx].tb = NULLP;
   }

   /* Stack Crash problem for TRACE5 Changes. Added the return below */
  RETVOID;
} /* rgDBMInitCmnLcLst */

/**
 * @brief Handler for Initializing the common logical channel list of the cell.
 *
 * @details
 *
 *     Function : rgDBMFreeCmnLcLst
 *     
 *   Initializes following common logical channels belonging to the cell.
 *     - BCCH on BCH
 *     - BCCH on DLSCH
 *     - PCCH
 *           
 *  @param[in]  RgCellCb *cellCb
 *  @return  Void
 **/
#ifdef ANSI
PUBLIC Void rgDBMFreeCmnLcLst
(
RgCellCb       *cellCb
)
#else
PUBLIC Void rgDBMFreeCmnLcLst(cellCb)
RgCellCb       *cellCb;
#endif
{
   U8 idx;
   
   TRC2(rgDBMFreeCmnLcLst);
   
   cellCb->bcchBchInfo.lcId = RG_INVALID_LC_ID;
   cellCb->pcchInfo.lcId = RG_INVALID_LC_ID;

   for (idx = 0; idx < cellCb->numBcchDlschInfo; idx++)
   {
      cellCb->bcchDlschInfo[idx].lcId = RG_INVALID_LC_ID;
      RG_FREE_MSG(cellCb->bcchDlschInfo[idx].tb);
   }
   cellCb->numBcchDlschInfo = 0;

   /* Stack Crash problem for TRACE5 Changes. Added the return below */
  RETVOID;
} /* rgDBMFreeCmnLcLst */

/**
 * @brief Handler for inserting dedicated DL logical channel.
 *
 * @details
 *
 *     Function : rgDBMInsDlDedLcCb
 *     
 *  @param[in]  RgUeCb *ueCb
 *  @param[in]  RgDlLcCb* dlLcCb
 *  @return  Void
 **/
#ifdef ANSI
PUBLIC Void rgDBMInsDlDedLcCb
(
RgUeCb         *ueCb, 
CmLteLcId        idx
)
#else
PUBLIC Void rgDBMInsDlDedLcCb(ueCb, idx)
RgUeCb         *ueCb; 
CmLteLcId        idx;
#endif
{
   TRC2(rgDBMInsDlDedLcCb);
   if( idx >= RG_DEDLC_MIN_LCID )
   {
      ueCb->dl.lcCb[idx-1].lcId = idx;
   }
  /* Stack Crash problem for TRACE5 Changes. Added the return below */
  RETVOID;
}  /* rgDBMInsDlDedLcCb */

/**
 * @brief Handler for deleting dedicated DL logical channel.
 *
 * @details
 *
 *     Function : rgDBMDelDlDedLcCb
 *     
 *  @param[in]  RgUeCb *ueCb
 *  @param[in]  RgDlLcCb* dlLcCb
 *  @return  Void
 **/
#ifdef ANSI
PUBLIC Void rgDBMDelDlDedLcCb
(
RgUeCb         *ueCb, 
RgDlLcCb       *dlLcCb 
)
#else
PUBLIC Void rgDBMDelDlDedLcCb(ueCb, dlLcCb)
RgUeCb         *ueCb; 
RgDlLcCb       *dlLcCb; 
#endif
{
   TRC2(rgDBMDelDlDedLcCb);

   ueCb->dl.lcCb[dlLcCb->lcId - 1].lcId = RG_INVALID_LC_ID;

  /* Stack Crash problem for TRACE5 Changes. Added the return below */
  RETVOID;
  
}  /* rgDBMDelDlDedLcCb */

/**
 * @brief Handler for accessing the existing DL dedicated lcCb at idx in the 
 * lcCbLst of the ueCb.
 *
 * @details
 *
 *     Function : rgDBMGetDlDedLcCb
 *     
 *  @param[in]  *ueCb
 *  @param[in]  idx
 *  @return  RgDlLcCb*
 **/
#ifdef ANSI
PUBLIC RgDlLcCb* rgDBMGetDlDedLcCb
(
RgUeCb         *ueCb, 
CmLteLcId        idx
)
#else
PUBLIC RgDlLcCb* rgDBMGetDlDedLcCb(ueCb, idx)
RgUeCb         *ueCb; 
CmLteLcId        idx;
#endif
{
   TRC2(rgDBMGetDlDedLcCb);

   if (idx > RG_DEDLC_MAX_LCID || idx <= 0)
   {
      RETVALUE(NULLP);
   }
   if(ueCb->dl.lcCb[idx-1].lcId == RG_INVALID_LC_ID)
   {
      RETVALUE(NULLP);
   }

   RETVALUE(&ueCb->dl.lcCb[idx-1]);

}  /* rgDBMGetDlDedLcCb */

/**
 * @brief Handler for inserting dedicated UL logical channel.
 *
 * @details
 *
 *     Function : rgDBMInsUlDedLcCb
 *     
 *           
 *  @param[in]  RgUeCb *ueCb
 *  @param[in]  CmLteLcId idx
 *  @param[in]  LteLcgId gId
 *  @return  Void
 **/
#ifdef LTE_L2_MEAS
#ifdef ANSI
PUBLIC Void rgDBMInsUlDedLcCb
(
RgUeCb         *ueCb, 
CmLteLcId      idx,
LteLcgId       gId,
U8             qci
)
#else
PUBLIC Void rgDBMInsUlDedLcCb(ueCb, idx, gId, qci)
RgUeCb         *ueCb; 
CmLteLcId      idx;
LteLcgId       gId;
U8             qci;
#endif
#else
#ifdef ANSI
PUBLIC Void rgDBMInsUlDedLcCb
(
RgUeCb         *ueCb, 
CmLteLcId      idx,
LteLcgId       gId
)
#else
PUBLIC Void rgDBMInsUlDedLcCb(ueCb, idx, gId)
RgUeCb         *ueCb; 
CmLteLcId      idx;
LteLcgId       gId;
#endif
#endif
{
   TRC2(rgDBMInsUlDedLcCb);
   if ( idx >= RG_DEDLC_MIN_LCID)
   {
      ueCb->ul.lcCb[idx - 1].lcId = idx;
      ueCb->ul.lcCb[idx - 1].lcgId = gId;
#ifdef LTE_L2_MEAS
      ueCb->ul.lcCb[idx - 1].qci = qci;
#endif
   }
   if(ueCb->ul.lcgArr[gId].lcgId == RG_INVALID_LCG_ID)
   {
      ueCb->ul.lcgArr[gId].lcgId = gId;
   }
   ueCb->ul.lcgArr[gId].lcCount = ueCb->ul.lcgArr[gId].lcCount + 1;


  /* Stack Crash problem for TRACE5 Changes. Added the return below */
  RETVOID;

}  /* rgDBMInsUlDedLcCb */

/**
 * @brief Handler for deleting the dedicated UL logical channel.
 *
 * @details
 *
 *     Function : rgDBMUpdUlDedLcCb
 *     
 *           
 *  @param[in]  RgUeCb *ueCb
 *  @param[in]  RgUlLcCb* ulLcCb
 *  @param[in]  LteLcgId gId
 *  @return  Void
 **/
#ifdef ANSI
PUBLIC Void rgDBMUpdUlDedLcCb
(
RgUeCb         *ueCb, 
RgUlLcCb       *ulLcCb,
LteLcgId       gId
)
#else
PUBLIC Void rgDBMUpdUlDedLcCb(ueCb, ulLcCb, gId)
RgUeCb         *ueCb; 
RgUlLcCb       *ulLcCb; 
LteLcgId       gId;
#endif
{
   TRC2(rgDBMUpdUlDedLcCb);

   ueCb->ul.lcgArr[ulLcCb->lcgId].lcCount = 
               ueCb->ul.lcgArr[ulLcCb->lcgId].lcCount - 1;
   if(ueCb->ul.lcgArr[ulLcCb->lcgId].lcCount == 0)
   {
      ueCb->ul.lcgArr[ulLcCb->lcgId].lcgId = RG_INVALID_LCG_ID;
   }
   ueCb->ul.lcCb[ulLcCb->lcId - 1].lcgId = gId;

   if(ueCb->ul.lcgArr[gId].lcgId == RG_INVALID_LCG_ID)
   {
      ueCb->ul.lcgArr[gId].lcgId = gId;
   }
   ueCb->ul.lcgArr[gId].lcCount = ueCb->ul.lcgArr[gId].lcCount + 1;

  /* Stack Crash problem for TRACE5 Changes. Added the return below */
  RETVOID;
}  /* rgDBMUpdUlDedLcCb */

/**
 * @brief Handler for updating the dedicated UL logical channel.
 *
 * @details
 *
 *     Function : rgDBMDelUlDedLcCb
 *     
 *           
 *  @param[in]  RgUeCb *ueCb
 *  @param[in]  RgUlLcCb* ulLcCb
 *  @return  Void
 **/
#ifdef ANSI
PUBLIC Void rgDBMDelUlDedLcCb
(
RgUeCb         *ueCb, 
RgUlLcCb       *ulLcCb 
)
#else
PUBLIC Void rgDBMDelUlDedLcCb(ueCb, ulLcCb)
RgUeCb         *ueCb; 
RgUlLcCb       *ulLcCb; 
#endif
{
   TRC2(rgDBMDelUlDedLcCb);

   ueCb->ul.lcgArr[ulLcCb->lcgId].lcCount = 
               ueCb->ul.lcgArr[ulLcCb->lcgId].lcCount - 1;
   if(ueCb->ul.lcgArr[ulLcCb->lcgId].lcCount == 0)
   {
      ueCb->ul.lcgArr[ulLcCb->lcgId].lcgId = RG_INVALID_LCG_ID;
   }
   ueCb->ul.lcCb[ulLcCb->lcId - 1].lcgId = RG_INVALID_LCG_ID;
   ueCb->ul.lcCb[ulLcCb->lcId - 1].lcId = RG_INVALID_LC_ID;

  /* Stack Crash problem for TRACE5 Changes. Added the return below */
  RETVOID;
}  /* rgDBMDelUlDedLcCb */

/**
 * @brief Handler for accessing the existing UL dedicated lcCb at idx in the
 * lcCbLst of the ueCb.
 *
 * @details
 *
 *     Function : rgDBMGetUlDedLcCb
 *     
 *           
 *  @param[in]  *ueCb
 *  @param[in]  idx
 *  @return  RgUlLcCb*
 **/
#ifdef ANSI
PUBLIC RgUlLcCb* rgDBMGetUlDedLcCb
(
RgUeCb         *ueCb, 
CmLteLcId        idx
)
#else
PUBLIC RgUlLcCb* rgDBMGetUlDedLcCb(ueCb, idx)
RgUeCb         *ueCb; 
CmLteLcId        idx;
#endif
{
   TRC2(rgDBMGetUlDedLcCb)

   if (idx > RG_DEDLC_MAX_LCID || idx < RG_DEDLC_MIN_LCID)
   {
      RETVALUE(NULLP);
   }
   if(ueCb->ul.lcCb[idx-1].lcId == RG_INVALID_LC_ID)
   {
      RETVALUE(NULLP);
   }

   RETVALUE(&ueCb->ul.lcCb[idx-1]);
}  /* rgDBMGetDlDedLcCb */

/**
 * @brief Handler for accessing the existing DL common lcCb identified by the key lcId
 * in the lcCbLst of the ueCb.
 *
 * @details
 *
 *     Function : rgDBMChkCmnLcCb
 *     
 *           
 *  @param[in]  *cellCb
 *  @param[in]  lcId
 *  @return  S16
 *      -# ROK
 *      -# RFAILED
 **/
#ifdef ANSI
PUBLIC S16 rgDBMChkCmnLcCb
(
RgCellCb       *cellCb, 
CmLteLcId        lcId
)
#else
PUBLIC S16 rgDBMChkCmnLcCb(cellCb, lcId)
RgCellCb       *cellCb;
CmLteLcId        lcId;
#endif
{
   U8 idx;

   TRC2(rgDBMChkCmnLcCb)

   if(cellCb->bcchBchInfo.lcId == lcId)
   {
      RETVALUE(ROK);
   } 
   if(cellCb->pcchInfo.lcId == lcId)
   {
      RETVALUE(ROK);
   }

   for (idx = 0; idx < cellCb->numBcchDlschInfo; idx++)
   {
      if(cellCb->bcchDlschInfo[idx].lcId == lcId)
      {
         RETVALUE(ROK);
      } 
   }
   RETVALUE(RFAILED);
}  /* rgDBMChkCmnLcCb */

/**
 * @brief Handler for accessing the existing BCCH mapped on to BCH in the 
 * lcCbLst of the ueCb.
 *
 * @details
 *
 *     Function : rgDBMGetBcchOnBch
 *     
 *           
 *  @param[in]  *cellCb
 *  @return  RgBcchBchLcCb*
 **/
#ifdef ANSI
PUBLIC RgBcchBchLcCb* rgDBMGetBcchOnBch
(
RgCellCb       *cellCb 
)
#else
PUBLIC RgBcchBchLcCb* rgDBMGetBcchOnBch(cellCb)
RgCellCb       *cellCb;
#endif
{
   TRC2(rgDBMGetBcchOnBch)

   if(cellCb->bcchBchInfo.lcId != RG_INVALID_LC_ID)
   {
      RETVALUE(&(cellCb->bcchBchInfo));
   }
   RETVALUE(NULLP);
}  /* rgDBMGetBcchOnBch */

/**
 * @brief Handler for accessing the existing BCCH mapped on to DLSCH in the 
 * lcCbLst of the ueCb.
 *
 * @details
 *
 *     Function : rgDBMGetBcchOnDlsch
 *     
 *           
 *  @param[in]  *cellCb
 *  @param[in]  lcId
 *  @return  RgBcchDlschLcCb*
 **/
#ifdef ANSI
PUBLIC RgBcchDlschLcCb* rgDBMGetBcchOnDlsch
(
RgCellCb       *cellCb,
CmLteLcId      lcId
)
#else
PUBLIC RgBcchDlschLcCb* rgDBMGetBcchOnDlsch(cellCb,lcId)
RgCellCb       *cellCb;
CmLteLcId      lcId;
#endif
{
   U8 idx;

   TRC2(rgDBMGetBcchOnDlsch)

   for (idx = 0; idx < RG_MAX_BCCH_DLSCH; idx++)
   {
      if(cellCb->bcchDlschInfo[idx].lcId == lcId)
      {
         RETVALUE(&(cellCb->bcchDlschInfo[idx]));
      }
   }
   RETVALUE(NULLP);
}  /* rgDBMGetBcchOnDlsch */

/**
 * @brief Handler for accessing the existing PCCH in the lcCbLst of the ueCb.
 *
 * @details
 *
 *     Function : rgDBMGetPcch
 *     
 *           
 *  @param[in]  *cellCb
 *  @return  RgPcchLcCb*
 **/
#ifdef ANSI
PUBLIC RgPcchLcCb* rgDBMGetPcch
(
RgCellCb       *cellCb
)
#else
PUBLIC RgPcchLcCb* rgDBMGetPcch(cellCb)
RgCellCb       *cellCb;
#endif
{
   TRC2(rgDBMGetPcch)
 
   if(cellCb->pcchInfo.lcId != RG_INVALID_LC_ID)
   {
      RETVALUE(&(cellCb->pcchInfo));
   }
   RETVALUE(NULLP);
}  /* rgDBMGetPcch */

/**
 * @brief Handler for inserting the BCCH mapped on to BCH in the 
 * lcCbLst of the ueCb.
 *
 * @details
 *
 *     Function : rgDBMInsBcchOnBch
 *     
 *           
 *  @param[in]  *cellCb
 *  @return  Void
 **/
#ifdef ANSI
PUBLIC Void rgDBMInsBcchOnBch
(
RgCellCb       *cellCb, 
CmLteLcId      idx
)
#else
PUBLIC Void rgDBMInsBcchOnBch(cellCb, idx)
RgCellCb       *cellCb;
CmLteLcId      idx;
#endif
{
   TRC2(rgDBMInsBcchOnBch)

   cellCb->bcchBchInfo.lcId = idx;

  /* Stack Crash problem for TRACE5 Changes. Added the return below */
  RETVOID;
}  /* rgDBMInsBcchOnBch */

/**
 * @brief Handler for inserting the BCCH mapped on to DLSCH in the 
 * lcCbLst of the ueCb.
 *
 * @details
 *
 *     Function : rgDBMInsBcchOnDlsch
 *     
 *           
 *  @param[in]  *cellCb
 *  @return  Void
 **/
#ifdef ANSI
PUBLIC Void rgDBMInsBcchOnDlsch
(
RgCellCb       *cellCb, 
CmLteLcId      idx
)
#else
PUBLIC Void rgDBMInsBcchOnDlsch(cellCb, idx)
RgCellCb       *cellCb;
CmLteLcId      idx;
#endif
{
   TRC2(rgDBMInsBcchOnDlsch)

   cellCb->bcchDlschInfo[cellCb->numBcchDlschInfo].lcId = idx;
   cellCb->numBcchDlschInfo++;

  /* Stack Crash problem for TRACE5 Changes. Added the return below */
  RETVOID;
}  /* rgDBMInsBcchOnDlsch */


/**
 * @brief Handler for inserting the PCCH in the lcCbLst of the ueCb.
 *
 * @details
 *
 *     Function : rgDBMInsPcch
 *     
 *           
 *  @param[in]  *cellCb
 *  @return  Void
 **/
#ifdef ANSI
PUBLIC Void rgDBMInsPcch
(
RgCellCb       *cellCb, 
CmLteLcId      idx
)
#else
PUBLIC Void rgDBMInsPcch(cellCb, idx)
RgCellCb       *cellCb;
CmLteLcId      idx;
#endif
{
   TRC2(rgDBMInsPcch)

   cellCb->pcchInfo.lcId = idx;

  /* Stack Crash problem for TRACE5 Changes. Added the return below */
  RETVOID;
}  /* rgDBMInsPcch */

/**
 * @brief Handler for initializing the rachLst
 *
 * @details
 *
 *     Function : rgDBMInitRachLst
 *     
 *           
 *  @param[in] *cellCb 
 *  @return  Void
 **/
#ifdef ANSI
PRIVATE Void rgDBMInitRachLst
(
RgCellCb       *cellCb
)
#else
PRIVATE Void rgDBMInitRachLst(cellCb)
RgCellCb       *cellCb;
#endif
{
   TRC2(rgDBMInitRachLst)

   cmLListInit(&cellCb->raInfo.ueRachLst);
   RETVOID;
}  /* rgDBMInitRachLst */

/**
 * @brief Handler for inserting the ueCb in the rachLst.
 *
 * @details
 *
 *     Function : rgDBMInsUeCbInRachLst
 *     
 *           
 *  @param[in]  *cellCb
 *  @param[in]  *ueCb
 *  @return  Void
 **/
#ifdef ANSI
PUBLIC Void rgDBMInsUeCbInRachLst
(
RgCellCb       *cellCb, 
RgUeCb         *ueCb
)
#else
PUBLIC Void rgDBMInsUeCbInRachLst(cellCb, ueCb)
RgCellCb       *cellCb; 
RgUeCb         *ueCb;
#endif
{
   TRC2(rgDBMInsUeCbInRachLst)

   cmLListAdd2Tail(&cellCb->raInfo.ueRachLst,&ueCb->rachLstEnt);

   RETVOID;
}  /* rgDBMInsUeCbInRachLst */

/**
 * @brief Handler for accessing the existing raCb in the rachLst.
 *
 * @details
 *
 *     Function : rgDBMGetUeCbFromRachLst
 *     
 *           
 *  @param[in]  *cellCb
 *  @param[in]  key
 *  @return  RgUeCb*
 **/
#ifdef ANSI
PUBLIC RgUeCb* rgDBMGetUeCbFromRachLst
(
RgCellCb       *cellCb, 
CmLteRnti      key
)
#else
PUBLIC RgUeCb* rgDBMGetUeCbFromRachLst(cellCb, key)
RgCellCb       *cellCb; 
CmLteRnti      key;
#endif
{
   CmLList *tmpNode;

   TRC2(rgDBMGetUeCbFromRachLst)

   CM_LLIST_FIRST_NODE(&cellCb->raInfo.ueRachLst,tmpNode);
   while(tmpNode)
   {
      if(((RgUeCb *)tmpNode->node)->ueId == key)
      {
         RETVALUE((RgUeCb *)(tmpNode->node));
      }
      CM_LLIST_NEXT_NODE(&cellCb->raInfo.ueRachLst,tmpNode);
   }
   RETVALUE(NULLP);
}  /* rgDBMGetUeCbFromRachLst */

/**
 * @brief Handler for accessing the existing ueCb from rachLst.
 *
 * @details
 *
 *     Function : rgDBMGetNextUeCbFromRachLst
 *     
 *           
 *  @param[in]  *cellCb
 *  @param[in]  *ueCb
 *  @return  RgUeCb*
 **/
#ifdef ANSI
PUBLIC RgUeCb* rgDBMGetNextUeCbFromRachLst
(
RgCellCb       *cellCb,
RgUeCb         *ueCb
)
#else
PUBLIC RgUeCb* rgDBMGetNextUeCbFromRachLst(cellCb, ueCb)
RgCellCb       *cellCb; 
RgUeCb         *ueCb;
#endif
{
   TRC2(rgDBMGetNextUeCbFromRachLst)

   if(!ueCb)
   {
      RETVALUE( cellCb->raInfo.ueRachLst.first ? 
               (RgUeCb *)(cellCb->raInfo.ueRachLst.first->node) : NULLP );
   }
   RETVALUE( ueCb->rachLstEnt.next ? 
               (RgUeCb *)(ueCb->rachLstEnt.next->node) : NULLP );
}  /* rgDBMGetNextUeCbFromRachLst */

/**
 * @brief Handler for deleting the existing ueCb in the rachLst.
 *
 * @details
 *
 *     Function : rgDBMDelUeCbFromRachLst
 *     
 *           
 *  @param[in]  *cellCb
 *  @param[in]  key
 *  @return  RgUeCb*
 **/
#ifdef ANSI
PUBLIC Void rgDBMDelUeCbFromRachLst
(
RgCellCb       *cellCb, 
RgUeCb         *ueCb 
)
#else
PUBLIC Void rgDBMDelUeCbFromRachLst(cellCb, ueCb)
RgCellCb       *cellCb; 
RgUeCb         *ueCb;
#endif
{
   TRC2(rgDBMDelUeCbFromRachLst)

   cmLListDelFrm(&cellCb->raInfo.ueRachLst, &ueCb->rachLstEnt);
   RETVOID;
}  /* rgDBMDelUeCbFromRachLst */

/**********************************************************************
 
         End of file
**********************************************************************/
