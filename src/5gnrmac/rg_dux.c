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
  
     File:    rg_dux.c 
  
**********************************************************************/

/** @file rg_dux.c
@brief This module handles de-multiplexing of the data recieved at MAC.
*/

static const char* RLOG_MODULE_NAME="MAC";
static int RLOG_FILE_ID=176;
static int RLOG_MODULE_ID=4096;
/* header include files -- defines (.h) */
#include "envopt.h"        /* environment options */
#include "envdep.h"        /* environment dependent */
#include "envind.h"        /* environment independent */
  
#include "gen.h"           /* general */
#include "ssi.h"           /* system services */

#include "cm_lte.h"        /* Common LTE */
#include "cm_tkns.h"       /* Common Token Defines */
#include "cm_llist.h"      /* Common Link List Defines */
#include "cm_hash.h"       /* Common Hash List Defines */
#include "cm_mblk.h"       /* common memory link list library */

#include "rg_env.h"        /* MAC Environment Defines */
#include "tfu.h"           /* CRG Interface defines */
#include "crg.h"           /* CRG Interface defines */
#include "rg_sch_inf.h"           /* RGR Interface defines */
#include "rgu.h"           /* RGU Interface defines */
#include "lrg.h"           /* LRG Interface defines */

#include "rg.h"            /* MAC defines */
#include "rg_err.h"        /* MAC error defines */

/* header/extern include files (.x) */
#include "gen.x"           /* general */
#include "ssi.x"           /* system services */
#include "cm5.x"           /* common timers */
#include "cm_lib.x"        /* common library */
#include "cm_lte.x"        /* Common LTE */
#include "cm_tkns.x"       /* Common Token Definitions */
#include "cm_llist.x"      /* Common Link List Definitions */
#include "cm_lib.x"        /* Common Library Definitions */
#include "cm_hash.x"       /* Common Hash List Definitions */
#include "cm_mblk.x"       /* common memory link list library */

#include "rgu.x"           /* RGU types */
#include "tfu.x"           /* CRG Interface includes */
#include "crg.x"           /* CRG Interface includes */
#include "rg_sch_inf.x"    /* SCH Interface includes */
#include "rg_prg.x"        /* PRG interface includes */
#include "rgu.x"           /* RGU Interface includes */
#include "lrg.x"           /* LRG Interface includes */

#include "rg.x"            /* MAC includes */

/* local defines */

/* local typedefs */
 
/* local externs */
 
/* forward references */

#define RG_DUX_ALLOC(_pdu, _size, _dataPtr, _ret) {\
   _ret = cmGetMem((Ptr)&_pdu->memCp, _size, (Ptr *)&_dataPtr); \
}

#define RG_INIT_SDU(_sdu, _lcId, _len) {\
   (_sdu)->lcId = (_lcId); \
   (_sdu)->len = (_len); \
   (_sdu)->mBuf = NULLP; \
   (_sdu)->sduLstEnt.next = NULLP; \
   (_sdu)->sduLstEnt.prev = NULLP; \
   (_sdu)->sduLstEnt.node = (PTR)(_sdu); \
}

#define RG_EXT_BS(_bsr, _bs1, _bs2, _bs3, _bs4)  {\
   _bs1 = _bsr[0] >> 2; \
   _bs2 = (((_bsr[0] & 0x3) << 4) | (_bsr[1] >> 4)); \
   _bs3 = (((_bsr[1] & 0x0F) << 2) | (_bsr[2] >> 6)); \
   _bs4 = _bsr[2] & 0x3F; \
}

#define RG_UNPACK_LONG_BSR(_bsr, _mBuf, _ret) {\
   _ret = SRemPreMsgMult((_bsr), 3, (_mBuf)); \
}

#define RG_UNPACK_SHORT_BSR(_bsr, _mBuf, _ret) {\
   _ret = SUnpkU8((_bsr), (_mBuf)); \
}

#define RG_UNPACK_TRUNC_BSR(_bsr, _mBuf, _ret) {\
   _ret = SUnpkU8((_bsr), (_mBuf)); \
}

#define RG_UNPACK_PHR(_phr, _mBuf, _ret) {\
   _ret = SUnpkU8((_phr), (_mBuf)); \
}

#define RG_UNPACK_CRNTI(_rnti, _mBuf, _ret) {\
   Data _unpkArray[2];\
   *_rnti = 0;\
   _ret = SRemPreMsgMult(_unpkArray, (MsgLen) 2, _mBuf);\
   if (_ret == ROK)\
   {\
      *_rnti = (U16) PutHiByte(*_rnti, (U8) _unpkArray[0]);\
      *_rnti = (U16) PutLoByte(*_rnti, (U8) _unpkArray[1]);\
   }\
}

/* For EXT PHR DEMUX */
#define RG_UNPACK_EXT_PHR_CI(_ci, _mBuf, _ret) {\
   _ret = SUnpkU8((_ci), (_mBuf)); \
}

#define RG_UNPACK_EXT_PHR(_extPhr, _mBuf, _ret) {\
   _ret = SUnpkU8((_extPhr), (_mBuf)); \
}



/**
 * @brief Handles the insertion of SDU in to PDU.
 *
 * @details
 *
 *     Function: rgDUXInsSdu
 *     
 *     This API handles the insertion of SDU in to PDU.
 *     
 *     Processing Steps: 
 *      - Append the sdu to the sduLst of pdu.
 *
 *  @param[in]  Inst        inst
 *  @param[out] *pdu
 *  @param[in]  lcId
 *  @param[in]  sduLen
 *  @param[out] *err
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PRIVATE S16 rgDUXInsSdu
(
Inst        inst,
RgMacPdu    *pdu,
RgMacSdu    **sdu,
U8          lcId,
U16         sduLen,
RgErrInfo   *err
)
#else
PRIVATE S16 rgDUXInsSdu(inst,pdu, sdu, lcId, sduLen, err)
Inst        inst;
RgMacPdu    *pdu;
RgMacSdu    **sdu,
U8          lcId;
U16         sduLen;
RgErrInfo   *err;
#endif
{
   S16         ret;
   RgMacSdu    *sduAloc = NULLP;

   TRC2(rgDUXInsSdu)

   RG_DUX_ALLOC(pdu, sizeof(RgMacSdu), sduAloc, ret);
   if(ret != ROK)
   {
      RLOG1(L_ERROR, "Allocation of RgSubHdr failed for LCID:%d",lcId);
      err->errCause = RGERR_DUX_MEM_EXHAUST;
      RETVALUE(RFAILED);
   }
   *sdu = sduAloc; 
   RG_INIT_SDU(sduAloc, lcId, sduLen);
   cmLListAdd2Tail(&pdu->sduLst, &sduAloc->sduLstEnt);
   RETVALUE(ROK);
}

/**
 * @brief Handles extracting the CE sub headers from the MAC PDU.
 *
 * @details
 *
 *     Function: rgDUXExtSubHdr
 *     
 *     This API handles extracting the  sub headers from the MAC PDU.
 *     
 *     Processing Steps: 
 *      - Extract the each sub header.
 *
 *  @param[in]  Inst        inst
 *  @param[out]  *pdu
 *  @param[in]  *mBuf
 *  @param[out] *lcId
 *  @param[out] *len
 *  @param[out] *err
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PRIVATE S16 rgDUXExtSubHdr
(
Inst        inst,
RgMacPdu    *pdu,
Buffer      *mBuf,
U8          *lcId,
U16         *len,
RgErrInfo   *err
)
#else
PRIVATE S16 rgDUXExtSubHdr(inst,pdu, mBuf, lcId,
len, err)
Inst        inst;
RgMacPdu    *pdu;
Buffer      *mBuf;
U8          *lcId;
U16         *len;
RgErrInfo   *err;
#endif
{
   U8             byte;
   U8             fmt=0;
 
   TRC2(rgDUXExtSubHdr)

   *len = 0;   
   if(SUnpkU8(&byte,mBuf) != ROK)
   {
      RLOG0(L_ERROR, "SUnpkU8 failed");
      err->errCause = RGERR_DUX_UNPACK_FAILURE;
      RETVALUE(RFAILED);
   }
   /* Extract the lcid */
   RG_EXT_LCID(*lcId, byte);

   /*note: RG_EXT_PHR_LCID currently not considered */
   if(*lcId <= RG_DEDLC_MAX_LCID)
   {  /* variable size MAC Sub PDU */
      RG_EXT_FORMT_BIT(fmt,byte);
      if(SUnpkU8(&byte, mBuf) != ROK)
      {
         RLOG0(L_ERROR, "SUnpkU8 failed");
         err->errCause = RGERR_DUX_UNPACK_FAILURE;
         RETVALUE(RFAILED);
      }
      *len = byte;
      if(fmt)
      {
         if(SUnpkU8(&byte,mBuf) != ROK)
         {
            RLOG0(L_ERROR, "SUnpkU8 failed");
            err->errCause = RGERR_DUX_UNPACK_FAILURE;
            RETVALUE(RFAILED);
         }
         *len = (*len << 8) | byte;
      }
   }
   RETVALUE(ROK);
} /* rgDUXExtSubHdr */

/**
 * @brief Handles extracting the CEs from the MAC PDU.
 *
 * @details
 *
 *     Function: rgDUXExtCe
 *     
 *     This API handles extracting the CEs from the MAC PDU.
 *     
 *     Processing Steps: 
 *      - Based on the ce sub header extract the ce.
 *
 *  @param[in]  Inst        inst
 *  @param[in,out]  *pdu
 *  @param[out] *ceInfo
 *  @param[in]  *mBuf
 *  @param[in] lcId
 *  @param[in] subPduLen
 *  @param[out] *err
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PRIVATE S16 rgDUXExtCe
(
Inst        inst,
RgMacPdu    *pdu,
RgInfCeInfo  *ceInfo,
Buffer       *mBuf,
U8          lcId,
U16         subPduLen,
RgErrInfo    *err
)
#else
PRIVATE S16 rgDUXExtCe(inst,pdu, ceInfo, mBuf,lcId,subPduLen, err)
Inst        inst;
RgMacPdu    *pdu;
RgInfCeInfo  *ceInfo;
Buffer       *mBuf;
U8          lcId;
U16         subPduLen;
RgErrInfo    *err;
#endif
{
   S16            ret;

   TRC2(rgDUXExtCe);

   switch(lcId)
   {
      case RG_EXT_PHR_LCID:
         {
            U8 Ci;
            U8 sCellIdx;
            U8 extPhrOctet;
            U8 extPhrPCmax;
            RgInfExtPhrCEInfo *extPhr;

            RG_UNPACK_EXT_PHR_CI(&Ci,mBuf,ret);
            if(ret != ROK)
            {
               RLOG1(L_ERROR,"Unpacking of EXT PHR failed LCID:%d",lcId);
               err->errCause = RGERR_DUX_UNPACK_FAILURE;
               RETVALUE(RFAILED);
            }

            /* Not handling Type 2 PHR report as simultaneous PUSCH/PUCCH
               is not supported as of now */
            extPhr = &ceInfo->ces.extPhr;
            extPhr->numServCells = 0;

            /* Setting first BIT as PCELL field even though reserved is always
               reported by UE */
            Ci |= 0x1;
            for (sCellIdx = 0; (Ci && sCellIdx < CM_LTE_MAX_CELLS); sCellIdx++)
            {
               if (Ci & 0x1)
               {
                  extPhr->servCellPhr[extPhr->numServCells].sCellIdx = sCellIdx;
                  RG_UNPACK_EXT_PHR(&extPhrOctet,mBuf,ret);
                  if(ret != ROK)
                  {
                     RLOG1(L_ERROR,"Unpacking of EXT PHR failed LCID:%d",lcId);
                     err->errCause = RGERR_DUX_UNPACK_FAILURE;
                     RETVALUE(RFAILED);
                  }

                  /* extPhrOctet: Bits : 7 6 5 4 3 2 1 0
                   *                     P V x x x x x x
                   *                         <6x Bit phr>
                   */
                  /* P : P Back off applied or not */
                  extPhr->servCellPhr[extPhr->numServCells].phr = (extPhrOctet & 0x3F);
                  extPhr->servCellPhr[extPhr->numServCells].pBackOff = 
                     ((extPhrOctet >> 7) & 0x01);

                  /* V: Virtual PCMAX or Real Tx PCMAX */
                  if (extPhrOctet & 0x40)
                  {
                     extPhr->servCellPhr[extPhr->numServCells].pCmax = RG_REF_PCMAX;
                  }
                  else
                  {
                     RG_UNPACK_EXT_PHR(&extPhrPCmax,mBuf,ret);
                     if(ret != ROK)
                     {
                        RLOG1(L_ERROR,"Unpacking of EXT PHR failed LCID:%d",lcId);
                        err->errCause = RGERR_DUX_UNPACK_FAILURE;
                        RETVALUE(RFAILED);
                     }
                     extPhr->servCellPhr[extPhr->numServCells].pCmax = (extPhrPCmax & 0x3F);
                  }
                  extPhr->numServCells++;
               }
               Ci >>= 1;
            }

            ceInfo->bitMask |= RG_EXT_PHR_CE_PRSNT;
         }
         break;

      case RG_PHR_LCID:
         {
            RG_UNPACK_PHR(&ceInfo->ces.phr,mBuf,ret);
            if(ret != ROK)
            {
               RLOG1(L_ERROR,"Unpacking of PHR failed LCID:%d",lcId);
               err->errCause = RGERR_DUX_UNPACK_FAILURE;
               RETVALUE(RFAILED);
            }
            ceInfo->bitMask |= RG_PHR_CE_PRSNT;
         }
         break;
      case RG_TRUNC_BSR_LCID:
         {
            RG_UNPACK_TRUNC_BSR(&ceInfo->ces.bsr.truncBsr,mBuf,ret);
            if(ret != ROK)
            {
               RLOG1(L_ERROR,"Unpacking of Trunc BSR failed LCID:%d",lcId);
               err->errCause = RGERR_DUX_UNPACK_FAILURE;
               RETVALUE(RFAILED);
            }
            ceInfo->bitMask |= RG_TRUNC_BSR_CE_PRSNT;
         }
         break;
      case RG_SHORT_BSR_LCID:
         {
            RG_UNPACK_SHORT_BSR(&ceInfo->ces.bsr.shortBsr,mBuf,ret);
            if(ret != ROK)
            {
               RLOG1(L_ERROR,"Unpacking of Short BSR failed LCID:%d",lcId);
               err->errCause = RGERR_DUX_UNPACK_FAILURE;
               RETVALUE(RFAILED);
            }
            ceInfo->bitMask |= RG_SHORT_BSR_CE_PRSNT;
         }
         break;
      case RG_LONG_BSR_LCID:
         {
            U8 longBsr[3] = {0}; /* KW_FIXX */
            RG_UNPACK_LONG_BSR(longBsr,mBuf,ret);
            if(ret != ROK)
            {
               RLOG1(L_ERROR,"Unpacking of Long BSR failed LCID:%d",lcId);
               err->errCause = RGERR_DUX_UNPACK_FAILURE;
               RETVALUE(RFAILED);
            }
            RG_EXT_BS(longBsr, 
                  ceInfo->ces.bsr.longBsr.bs1, 
                  ceInfo->ces.bsr.longBsr.bs2, 
                  ceInfo->ces.bsr.longBsr.bs3, 
                  ceInfo->ces.bsr.longBsr.bs4);
            ceInfo->bitMask |= RG_LONG_BSR_CE_PRSNT;
         }
         break;
      case RG_CRNTI_LCID:
         {
            RG_UNPACK_CRNTI(&ceInfo->ces.cRnti,mBuf,ret);
            if(ret != ROK)
            {
               RLOG1(L_ERROR,"Unpacking of C-RNTI failed LCID:%d",lcId);
               err->errCause = RGERR_DUX_UNPACK_FAILURE;
               RETVALUE(RFAILED);
            }
            ceInfo->bitMask |= RG_CRNTI_CE_PRSNT;
         }
         break;
      default:
         RLOG1(L_ERROR, "Invalid LCID:%u received",lcId); 
         err->errCause = RGERR_DUX_INV_LCID_RX;
         RETVALUE(RFAILED);
   }
   RETVALUE(ROK);
} /* rgDUXExtCe  */


/**
 * @brief Handles extracting the SDU from the MAC PDU.
 *
 * @details
 *
 *     Function: rgDUXExtSdu
 *     
 *     This API handles extracting the SDU corresponding to a logical channel.
 *     
 *     Processing Steps: 
 *      - Based on the length stored in the sub header extract the SDU.
 *
 *  @param[in]  Inst        inst
 *  @param[in,out]  *pdu
 *  @param[out] *ceInfo
 *  @param[in]  *mBuf
 *  @param[in] lcId
 *  @param[in] subPduLen
 *  @param[out] *err
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PRIVATE S16 rgDUXExtSdu
(
Inst        inst,
RgMacPdu    *pdu,
RgInfCeInfo  *ceInfo,
Buffer      **mBuf,
U8          lcId,
U16         subPduLen,
RgErrInfo   *err
)
#else
PRIVATE S16 rgDUXExtSdu(inst,pdu, ceInfo,mBuf,lcId,subPduLen,err)
Inst        inst;
RgMacPdu    *pdu;
RgInfCeInfo  *ceInfo;
Buffer      **mBuf;
U8          lcId;
U16         subPduLen;
RgErrInfo   *err;
#endif
{
   S16         ret;
   Buffer      *tmpBuf1;
   Buffer      *tmpBuf2 = NULLP;
   RgMacSdu    *sdu;

   TRC2(rgDUXExtSdu)

   if(lcId == RG_CCCH_LCID)
   {
      ceInfo->bitMask |= RG_CCCH_SDU_PRSNT;
   }

   if(rgDUXInsSdu(inst,pdu, &sdu,lcId, subPduLen, err) != ROK)
   {
      RG_FREE_MSG(*mBuf);
      RETVALUE(RFAILED);
   }

   tmpBuf1 = *mBuf;
   {
      ret = SSegMsg(tmpBuf1,subPduLen,&tmpBuf2);
      if((ret != ROK) && (!((ret == ROKDNA) )))
      {
         RG_FREE_MSG(tmpBuf1);
         RLOG0(L_ERROR,"SSegMsg failed");
         err->errCause = RGERR_DUX_RLC_PDU_CREAT_FAIL;
         RETVALUE(RFAILED);
      }
      sdu->mBuf = tmpBuf1;
      *mBuf = tmpBuf2;
   }
   RETVALUE(ROK);
}   /* rgDUXExtSdu */

/**
 * @brief Handles de-multiplexing of the data recieved at MAC.
 *
 * @details
 *
 *     Function: rgDUXDemuxData
 *     
 *     This API handles de-multiplexing of the data recieved at MAC.
 *     
 *     Invoked by: rgTOMTfuDatInd of TOM 
 *     
 *     Processing Steps: 
 *      - De-multiplex the mBuf
 *
 *  @param[in]  Inst        inst
 *  @param[in]  *pdu 
 *  @param[in]  *mBuf 
 *  @param[out] *err 
 *  @return  S16
 *      -# ROK 
 *      -# RFAILED 
 **/
#ifdef ANSI
PUBLIC S16 rgDUXDemuxData
(
Inst          inst,
RgMacPdu      *pdu,
RgInfCeInfo   *ceInfo,
Buffer        **mBuf,
RgErrInfo     *err
)
#else
PUBLIC S16 rgDUXDemuxData(inst,pdu, ceInfo, mBuf, err)
Inst          inst;
RgMacPdu      *pdu;
RgInfCeInfo   *ceInfo;
Buffer        **mBuf;
RgErrInfo     *err;
#endif
{
   U8          lcId;
   U16          subPduLen;
   MsgLen      len;  
   TRC2(rgDUXDemuxData)

   ceInfo->bitMask = 0x0000;

   /* Initialize the sdu list */
   cmLListInit(&pdu->sduLst);

   if(*mBuf == NULLP) 
   {
      RLOG0(L_ERROR, "Null Buffer Recevived");
      RETVALUE(RFAILED);
   }
   do
   {
      /* UL Message format  order : 
           PduSubHdr+SubPDU,PduSubHdr+SubPDU,...CeSubHdr+Ce,CeSubPdu+Ce,...,PADSubHdr+PAD */
      /* Extract the Sub headers */
      if(rgDUXExtSubHdr(inst,pdu, *mBuf, &lcId, 
               &subPduLen, err) != ROK)
      {
         RG_FREE_MSG(*mBuf);	      
         RLOG0(L_ERROR, "Failed to extract pad sub headers");
         RETVALUE(RFAILED);
      }
      if(lcId == RG_PAD_LCID)
      { /*at end of MAC PDU,  Padding started */ 
         RG_FREE_MSG(*mBuf);	      
         RETVALUE(ROK);
      }
      if(lcId <= RG_DEDLC_MAX_LCID)
      {
         /* Extract the sdus */
         if(rgDUXExtSdu(inst,pdu,ceInfo, mBuf,lcId,subPduLen, err) != ROK)
         {
            /* Fix : syed rgDUXExtSdu would have segmented mBuf and hence
             * will be responsible for freeing mBuf */
            *mBuf = NULLP;
            RLOG0(L_ERROR, "failed to Extract the sdus");
            RETVALUE(RFAILED);
         }
         if(*mBuf == NULLP) /* if message read completes then return */
         {
            RETVALUE(ROK);
         }
      }
      else
      {
         /* Extract the ces */
         if(rgDUXExtCe(inst,pdu,ceInfo,*mBuf, lcId,subPduLen, err) != ROK)
         {
            RG_FREE_MSG(*mBuf);	      
            RLOG0(L_ERROR, " failed to Extract the ces");
            RETVALUE(RFAILED);
         }
      }
      if(SFndLenMsg(*mBuf,&len) != ROK)
      {
         RG_FREE_MSG(*mBuf);
         RLOG0(L_ERROR,"mBuf length check failed");
         err->errCause = RGERR_DUX_UNPACK_FAILURE;
         RETVALUE(RFAILED);
      }
   }while(len);

   RG_FREE_MSG(*mBuf);
   RETVALUE(ROK);
}  /* rgDUXDemuxData */

/**********************************************************************
 
         End of file
**********************************************************************/
