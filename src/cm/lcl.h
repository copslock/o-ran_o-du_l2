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

/* Defines APIs exchanged between du_app and cl module of MAC */
#ifndef __LCL_H__
#define __LCL_H__

#define NUM_NUMEROLOGY 5  /* Number of numerology */
#define MAX_TDD_PERIODICITY 5
#define MAX_SYMB_PER_SLOT 14 

typedef enum
{
   DUP_MODE_FDD,
   DUP_MODE_TDD
}DuplexMode;

typedef enum
{
   MAC_GEN_FULL_PBCH_PAYLD,   /* MAC generated the full PBCH Payload */
   PHY_GEN_TIMING_PBCH_BIT,   /* PHY generates the timing PBCH bits */
   PHY_GEN_FULL_PBCH_PAYLD    /* PHY generates full PBCH payload */
}BchPduOpt;

typedef enum
{
   LONG_SEQUENCE,
   SHORT_SEQUENCE
}PrachSeqLen;

typedef enum
{
   UNRESTRICTED,
   RESTRICTED_SET_TYPE_A,
   RESTRICTED_SET_TYPE_B
}RstSetCfg;

typedef enum
{
   DONT_REPORT_RSSI,
   RSSI_UNIT_DBM,
   RSSI_UNIT_DBFS
}RSSIMeasUnit;

typedef enum
{
   DL_SLOT,
   UL_SLOT,
   GUARD_SLOT
}SlotConfig;

typedef enum
{
   TX_PRDCTY_MS_0P5,
   TX_PRDCTY_MS_0P625,
   TX_PRDCTY_MS_1,
   TX_PRDCTY_MS_1P25,
   TX_PRDCTY_MS_2,
   TX_PRDCTY_MS_2P5,
   TX_PRDCTY_MS_5,
   TX_PRDCTY_MS_10
}DlUlTxPeriodicity;

typedef enum
{
   BETA_PSS_0DB,
   BETA_PSS_1DB
}BetaPss;

typedef enum 
{
   SSB_PRDCTY_MS_5,
   SSB_PRDCTY_MS_10,
   SSB_PRDCTY_MS_20,
   SSB_PRDCTY_MS_40,
   SSB_PRDCTY_MS_80,
   SSB_PRDCTY_MS_160
}SSBPeriod;

typedef struct carrierCfg
{
   Bool  pres;
   U16   bw;             /* DL/UL bandwidth */
   U32   freq;           /* Absolute frequency of DL/UL point A in KHz */
   U16   k0[NUM_NUMEROLOGY];          /* K0 for DL/UL */
   U16   gridSize[NUM_NUMEROLOGY];    /* DL/UL Grid size for each numerologies */
   U16   numAnt;         /* Number of Tx/Rx antennas */
}CarrierCfg;

typedef struct ssbCfg
{
   U32         ssbPbchPwr;       /* SSB block power */
   BchPduOpt   bchPayload;       /* Options for generation of payload */
   U8          scsCmn;           /* subcarrier spacing for common */
   U16         ssbPrbOffset;     /* SSB PRB offset from point A */
   BetaPss     betaPss;
   SSBPeriod   ssbPeriod;        /* SSB Periodicity in msec */
   U8          ssbSubcOffset;    /* Subcarrier Offset */
   U32         mibPdu;           /* MIB payload */
   U32         nSSBMask[2];      /* Bitmap for actually transmitted SSB. */
   Bool        multCarrBand;     /* Multiple carriers in a band */
   Bool        multCellCarr;     /* Multiple cells in single carrier */
}SsbCfg;

typedef struct fdmInfo
{
   U16   rootSeqIdx;        /* Root sequence index */
   U8    numRootSeq;        /* Number of root sequences required for FD */
   U16   k1;                /* Frequency Offset for each FD */
   U8    zeroCorrZoneCfg;   /* Zero correlation zone cofig */
   U8    numUnusedRootSeq;  /* Number of unused root sequence */
   U8    *unsuedRootSeq;     /* Unused root sequence per FD */
}FdmInfo;

typedef struct prachCfg
{
   Bool          pres;
   PrachSeqLen   prachSeqLen;         /* RACH Sequence length: Long/short */
   U8            prachSubcSpacing;    /* Subcarrier spacing of RACH */
   RstSetCfg     prachRstSetCfg;      /* PRACH restricted set config */
   U8            prachFdm;            /* PRACH FDM (1,2,4,8) */
   FdmInfo       fdm[8];
   U8            ssbPerRach;          /* SSB per RACH occassion */
   Bool          prachMultCarrBand;    /* Multiple carriers in Band */
}PrachCfg;

typedef struct tddCfg
{
   Bool               pres;
   DlUlTxPeriodicity  tddPeriod;      /* DL UL Transmission periodicity */
   SlotConfig         slotCfg[MAX_TDD_PERIODICITY][MAX_SYMB_PER_SLOT]; 
}TDDCfg;

typedef struct clCellCfg
{
   U8             numTlv;     /* Number of configuration TLVs */
   U8             carrierId;  /* Carrired Index */
   U16            cellId;     /* Cell Id */
   U16            phyCellId;  /* Physical cell id */
   DuplexMode     dupType;    /* Duplex type: TDD/FDD */
   CarrierCfg     dlCarrCfg;  /* DL Carrier configuration */
   CarrierCfg     ulCarrCfg;  /* UL Carrier configuration */
   Bool           freqShft;   /* Indicates presence of 7.5kHz frequency shift */
   SsbCfg         ssbCfg;     /* SSB configuration */          
   PrachCfg       prachCfg;   /* PRACH Configuration */
   TDDCfg         tddCfg;     /* TDD periodicity and slot configuration */
   RSSIMeasUnit   rssiUnit;   /* RSSI measurement unit */
}ClCellCfg;

#endif

/**********************************************************************
         End of file
**********************************************************************/
