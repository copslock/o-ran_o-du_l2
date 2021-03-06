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

#ifndef __DU_COMMON_H__
#define __DU_COMMON_H__

#include "envdep.h"
#include "lcl.h"

#define DU_PROC  0
#define DU_INST 0
#define DU_POOL  1

#define SCTP_INST 0

/* Events */
#define EVTCFG 0

/* Selector */
#define DU_SELECTOR_LC   0
#define DU_SELECTOR_TC   1
#define DU_SELECTOR_LWLC 2


/* Macro definitions for F1 procedures */
#define CU_DU_NAME_LEN_MAX 30      /* Max length of CU/DU name string */
#define MAX_F1_CONNECTIONS 65536    /* Max num of F1 connections */
#define MAXCELLINGNBDU     2      /* Max num of cells served by gNB-DU */
#define MAXNUMOFBPLMN      1        /* Max num of broadcast PLMN ids */
#define MAXNRARFCN         3279165  /* Maximum values of NRAFCN */
#define MAXNRCELLBANDS     2       /* Maximum number of frequency bands */
#define MAXNUMOFSLICEITEMS 1024     /* Maximum number of signalled slice support items */
#define MAXBPLMNNRMINUS1   1       /* Maximum number of PLMN Ids broadcast in an NR cell minus 1 */
#define MAXNUMOFSIBTYPES   32       /* Maximum number of SIB types */
#define MAX_UEID           512      /* Maximum number of UE Ids */
#define MAXNUMOFTNLASSOCS  32       /* Max num of TNL Assoc between CU and DU */
#define MAXCELLINENB       256      /* Max num of cells served by eNB */
#define MAXNUMOFUEID       65536    /* Max num of UEs served by DU */
#define MAXNUMOFUACPLMN    12       /* Maximum number of PLMN ids*/
#define MAXNUMOFUACPERPLMN 64       /* Maximum number of signalled categories per PLMN */

#define MAX_IPV6_LEN       16       /* Max lenght of IPV6 Address */



typedef enum
{
   GNBDU,
   GNBCU 
}F1Entity;

typedef enum
{
   UNSPECIFIED,
   RL_FAILURE_RLC,
   INVALID_CU_UE_F1AP_ID,
   INVALID_DU_UE_F1AP_ID,
   INTERACTION_WITH_OTHER_PROC,
   INVALID_QCI_VALUE,
   RADIO_RELATED_REASON,
   INSUFFICIENT_RADIO_RESOURCE,
   PROCEDURE_CANCELLED,
   NORMAL_RELEASE,
   UNAVAILABLE_CELL,
   RL_FAILURE_OTHERS,
   UE_ACCESS_REJECTED,
   SLICE_RESOURCE_UNAVAILABLE,
   AMF_ABNORMAL_RELEASE,
   INVALID_PLMN
}RadioNwCause;

typedef enum
{
   TRANS_CAUSE_UNSPECIFIED,
   TRANSPORT_RESOURCE_UNAVAIL
}TransLayerCause;

typedef enum
{
   TRANSFER_SYNTAX_ERR,
   ABSTRACT_SYNTAX_ERR_REJECT,
   ABSTRACT_SYNTAX_ERR_NOTIFY,
   INCOMPATIBLE_MESSAGE,
   SEMANTIC_ERROR,
   ABSTRACT_SYNTAX_ERR_MSG_CONSTRUCT,
   PROTOCOL_CAUSE_UNSPECIFIED
}ProtocolCause;

typedef enum
{
   CONTROL_PROC_OVERLOAD,
   INSUFF_USER_PLANE_RESOURCE,
   HARDWARE_FAILURE,
   O_AND_M_INTERVENTION,
   MISC_CAUSE_UNSPECIFIED
}MiscCause;

typedef enum
{
  ALL
}F1ResetAll;

typedef enum 
{
  IN_SERVICE,
  OUT_OF_SERVICE
}SrvState;

typedef enum
{
   INITIATING_MSG,
   SUCCESSFUL_OUTCOME,
   UNSUCCESSFUL_OUTCOME
}F1TriggMsgVal;

typedef enum
{
   CRIT_REJECT,
   CRIT_IGNORE,
   CRIT_NOTIFY
}F1ProcCritVal;

typedef enum
{
   NOT_UNDERSTOOD,
   MISSING_IE
}F1ErrorType;

typedef enum
{
   SCS_15,
   SCS_30,
   SCS_60,
   SCS_120
}F1NrScs;

typedef enum
{
   NRB_11,
   NRB_18,
   NRB_24,
   NRB_25,
   NRB_31,
   NRB_32,
   NRB_38,
   NRB_51,
   NRB_52,
   NRB_65,
   NRB_66,
   NRB_78,
   NRB_79,
   NRB_93,
   NRB_106,
   NRB_107,
   NRB_121,
   NRB_132,
   NRB_133,
   NRB_135,
   NRB_160,
   NRB_162,
   NRB_189,
   NRB_216,
   NRB_217,
   NRB_245,
   NRB_264,
   NRB_270,
   NRB_273
}F1Nrb;

typedef enum
{
   VERY_SMALL,
   SMALL,
   MEDIUM,
   LARGE
}F1CellType;

typedef enum
{
   DL_ONLY,
   UL_ONLY,
   UL_DL
}F1CellDir;

typedef enum
{
   UE_USAGE,
   NON_UE_USAGE,
   BOTH
}F1AssocUsage;

typedef enum
{
   BARRED,
   NOT_BARRED
}CellBarred;

typedef enum
{
   OFFER,
   EXECUTION
}F1ReqType;

typedef enum
{
   IGNORE_YES,
   IGNORE_NO
}F1IgnoreReq;

typedef enum
{
   OVERLOADED,
   NOT_OVERLOADED
}F1GnbDuOvrloadInfo;

typedef enum
{
   REJECT_RRC_CONN_NON_EMERG_MODT,
   REJECT_RRC_CONN_SIGNALLING,
   PERMIT_EMERG_SESSION_AND_MOBILE_TERM_SERVICE,
   PERMIT_HIGH_PRIOR_SESSION_AND_MOBILE_TERM_SERVICE
}F1UacStandardAction;

typedef struct f1RrcVersion
{
   U8    rrcVer;     /* Latest RRC Version */
   U32   extRrcVer;  /* Latest RRC version extended */
}F1RrcVersion;

typedef struct f1FailCause
{
   union
   {
      RadioNwCause      radioNwCause;     /* Radio Network Layer Cause */
      TransLayerCause   transLayerCause;  /* Transport Layer Cause */
      ProtocolCause     protCause;        /* Protocol Cause */
      MiscCause         miscCause;        /* Miscellaneous Cause */
   }cause;
}F1FailCause;

typedef struct f1ProcCode
{
   Bool   pres;
   U8     value;
}F1ProcCode;

typedef struct f1TriggMsg
{
   Bool   pres;
   F1TriggMsgVal   value;
}F1TriggMsg;

typedef struct f1ProcCrit
{
   Bool   pres;
   F1ProcCritVal   value;
}F1ProcCrit;

typedef struct f1CritDignosIE
{
   F1ProcCritVal   ieCritVal;
   U16             ieId;
   F1ErrorType       type;
}F1CritDignosIE;

typedef struct f1CritDiagnostic
{
   Bool         pres;
   F1ProcCode   procCode;      /* Procedure Code */
   F1TriggMsg   triggeringMsg; /* Triggering Message */
   F1ProcCrit   procCrit;      /* Criticality of Triggering msg/proc */
   U32          transId;
   F1CritDignosIE diagnosIE;
}F1CritDiagnostic;

typedef struct f1FailureIE
{
   U32                transId;        /* Uniquely identify transaction */
   F1FailCause        cause;          /* Failure cause */
   U8                 timeToWait;     /* Seconds to be waited before retry */
   F1CritDiagnostic   critDiagnostic; /* Critical diagnostics */
}F1FailureIE;

typedef struct f1LogicalConnUeAssoc
{
   U32   gnbCuUeF1apId;
   U32   gnbDuUeF1apId;
}F1LogicalConnUeAssoc;

typedef struct f1ResetType
{
   union
   {
      F1ResetAll             resetAll; /* Reset All */
      F1LogicalConnUeAssoc   ueAssocLogicalConn[MAX_F1_CONNECTIONS]; /* In case of Partial Reset : UE associated logical F1 conn list*/
   }type;
}F1ResetType;

typedef struct f1PLMN
{
  U8 mcc[3];
  U8 mnc[3];
}F1Plmn;

typedef struct nrEcgi
{
  F1Plmn  plmn;
  U32     cellId;
}NrEcgi;

typedef struct f1SibType
{
   U8   sibType;
   U8   sibMsg;
   U8   value;
}F1SibType;

typedef struct f1TxBw
{
   F1NrScs   nrScs; /* NR subcarrier spacing */
   F1Nrb     nrb;   /* Transmission bandwidth expressed in units of RBs */
}F1TxBw;

typedef struct f1SulInfo
{
   Bool     pres;   
   U32      sulArfcn; /* SUL ARFCN */
   F1TxBw   sulTxBw;  /* SUL transmission bandwidth */ 
}F1SulInfo;

typedef struct f1FreqBand
{
  U16   nrFreqBand;
  U16   sulBand[MAXNRCELLBANDS];
}F1FreqBand;

typedef struct f1NrFreqInfo
{
   U32        nrArfcn;
   F1SulInfo  sulInfo;
   F1FreqBand freqBand[MAXNRCELLBANDS];
}F1NrFreqInfo;

typedef struct f1NrFddInfo
{
   F1NrFreqInfo   ulNrFreqInfo;  /* UL NR Frequency Info */
   F1NrFreqInfo   dlNrFreqInfo;  /* DL NR Frequency Info */
   F1TxBw         ulTxBw;        /* UL Transmission bandwidth */
   F1TxBw         dlTxBw;        /* DL Transmission bandwidth */
}F1NrFddInfo;

typedef struct f1NrTddInfo
{
   F1NrFreqInfo   nrFreqInfo;   /* NR Frequency Info */
   F1TxBw       nrTxBw;       /* NR Transmission Bandwidth */
}F1NrTddInfo;

typedef struct f1NrModeInfo
{
   union
   {
      F1NrFddInfo  fdd;
      F1NrTddInfo  tdd;
   }mode;
}F1NrModeInfo;

typedef struct f1EutraFddInfo
{
   U16   ulOffset;   /* UL offset to Point A */
   U16   dlOffset;   /* DL offset to Point A */
}F1EutraFDDInfo;

typedef struct f1EutraTddInfo
{
   U16   tddOffset;  /* Offset to Point A */
}F1EutraTDDInfo;

typedef struct f1EutraModeInfo
{
   union
   {
      F1EutraFDDInfo  fdd;
      F1EutraTDDInfo  tdd;
   }mode;
}F1EutraModeInfo;

typedef struct f1Tac
{
   Bool    pres;
   U16     tac;
}F1Tac;

typedef struct f1Snsaai
{
   U8   sst;
   U32  sd;
}F1Snsaai;

typedef struct epIpAddr
{
   char transportAddr[20]; /* Transport Layer Address */
}EpIpAddr;

typedef struct epIpAddrPort
{
    EpIpAddr epIpAddr;
    char   port[2];
}EpIpAddrPort;

typedef struct f1TaiSliceSuppLst
{
   Bool       pres;
   F1Snsaai   snssai[MAXNUMOFSLICEITEMS];   
}F1TaiSliceSuppLst;

typedef struct f1SrvdPlmn
{
   F1Plmn              plmn;
   F1TaiSliceSuppLst   taiSliceSuppLst;
}F1SrvdPlmn;

typedef struct f1BrdcstPlmnInfo
{
   F1Plmn   plmn[MAXNUMOFBPLMN];     /* PLMN id list */
   F1Plmn  extPlmn[MAXNUMOFBPLMN];   /* Extended PLMN id list */
   F1Tac    tac;                     /* 5GS-TAC */
   U32      nrCellId;                /* NR Cell id */
   U8       ranac;                   /* RAN Area code */

}F1BrdcstPlmnInfo;

typedef struct f1CellInfo
{
   NrEcgi   nrCgi;                   /* Cell global Identity */
   U32      nrPci;                   /* Physical Cell Identity */
   F1Plmn   plmn[MAXNUMOFBPLMN];     /* Available PLMN list */
   F1Plmn   extPlmn[MAXNUMOFBPLMN];  /* Extended available PLMN list */
}F1CellInfo;

typedef struct f1DuCellInfo
{
   F1CellInfo         cellInfo;     /* cell info */
   F1Tac              tac;          /* tracking area code */
   F1Tac              epsTac;       /* Configured EPS TAC */
   F1NrModeInfo       f1Mode;       /* NR mode info : FDD/TDD */
   U8                 measTimeCfg;  /* Measurement timing configuration */
   F1CellDir          cellDir;      /* Cell Direction */
   F1CellType         cellType;     /* Cell Type */
   F1BrdcstPlmnInfo   brdcstPlmnInfo[MAXBPLMNNRMINUS1]; /* Broadcast PLMN Identity Info List */
}F1DuCellInfo;

typedef struct f1DuSysInfo
{
   U8   mibMsg;   /* MIB message */
   U8   sib1Msg;  /* SIB1 message */
}F1DuSysInfo;

typedef struct f1CuSysInfo
{
   F1SibType   sibTypeToUpd[MAXNUMOFSIBTYPES];   /* SIB types to be updated */
}F1CuSysInfo;

typedef struct f1DuSrvdCellInfo
{
   F1DuCellInfo   duCellInfo;   /* gNB-DU served cell info */
   F1DuSysInfo    duSysInfo;    /* gNB-DU System Information */
}F1DuSrvdCellInfo;

typedef struct f1DuSrvdCellToDel
{
   NrEcgi             oldNrCgi;     /* Old NR CGI */
   F1DuSrvdCellInfo   srvdCellInfo; /* Info of served cell to be deleted */ 
}F1DuSrvdCellToDel;

typedef struct f1CuActCellInfo
{
   F1CellInfo    cuCellInfo;   /* gNB-CU available cell info */
   F1CuSysInfo   cuSysInfo;    /* gNB-CU System Information */
}F1CuActCellInfo;

typedef struct f1ActCellFail
{
   NrEcgi        nrCgi;
   F1FailCause   cause;
}F1ActCellFail;

typedef struct srvStatus
{
  SrvState  state;
  Bool      switchOffOngoing;
}SrvStatus;

typedef struct f1CellStatus
{
   NrEcgi      nrEcgi;
   SrvStatus   status;   /* Service status */
}F1CellStatus;

typedef struct f1DedSIDelUE
{
   U32      gnbCuUeF1apId;
   NrEcgi   nrEcgi;
}F1DedSIDelUE;

typedef struct tnlAssocInfo
{
   Bool           pres;
   EpIpAddr       epIpAddr;      /* Endpoint-IP Address */
   EpIpAddrPort   epIpAddrport;  /* Endpoint-IP Address and Port */
}F1TnlAssocAddr;

typedef struct f1TnlAssocUsage
{
    Bool           pres;
    F1AssocUsage   usage;
}F1TnlAssocUsage;

typedef struct f1TnlAssoc
{
   F1TnlAssocAddr   assocInfo;    /* TNL Assoc Transport Layer Info */
   F1TnlAssocUsage  assocUsage;   /* TNL Assoc usage */
}F1TnlAssoc;

typedef struct f1TnlAssocToRmv
{
   F1TnlAssocAddr   assocInfoCu;   /* TNL Assoc Transport Layer Info of CU */
   F1TnlAssocAddr   assocInfoDu;   /* TNL Assoc Transport Layer Info of DU */
}F1TnlAssocToRmv;

typedef struct f1TnlAssocSetupFail
{
   F1TnlAssocAddr   assocInfo;
   F1FailCause      cause;
}F1TnlAssocSetupFail;


typedef struct f1CellBarred
{
   NrEcgi       nrcgi;
   CellBarred   cellBarred;
}F1CellBarred;

typedef struct f1EutraCell
{
   U32                eutraCellId; /* EUTRA cell id */
   F1EutraModeInfo    mode;        /* EUTRA mode info : FDD/TDD */
   U8                 rsrcInd;     /* Protected EUTRA resource Indication */
}F1EutraCell;

typedef struct f1ProtectEUTRARsrc
{
   U32           specShareGrpId;               /* Spectrum sharing group id */
   F1EutraCell   eutraCellList[MAXCELLINENB];  /* EUTRA cells list */
}F1ProtectEUTRARsrc;

typedef struct f1UacOperatorDefined
{
   U8   accessCategory;
   U8   accessId; 
}F1UacOperatorDefined;

typedef struct f1UacCategoryType
{
   union
   {
      F1UacStandardAction    action;      /* UAC standardized action */
      F1UacOperatorDefined   operatorDef; /* UAC operator defined */
   }type;
}F1UacCategoryType;

typedef struct f1UacType
{
   U8                  uacReducInd;     /* Value 0 means no access rate reduction. Value 100 means full access rate reduction */
   F1UacCategoryType   uacCategoryType; /* UAC Category type */
}F1UacType;

/* Unified Access Class Assistance Information */
typedef struct f1UacAssistInfo
{
   F1Plmn      plmn[MAXNUMOFUACPLMN];        /* UAC PLMN list */
   F1UacType   uacType[MAXNUMOFUACPERPLMN];  /* UAC Type list */
}F1UacAssistInfo;

/* F1 setup related structures */

typedef struct f1SetupReq
{
  U32                transId;                       /* Uniquely identify transaction */
  U32                duId;                          /* DU ID */ 
  char               duName[CU_DU_NAME_LEN_MAX];    /* DU name */
  F1DuSrvdCellInfo   srvdCellLst[MAXCELLINGNBDU];   /* Serving cell list */
  F1RrcVersion       rrcVersion;                    /* RRC version */
}F1SetupReq;

typedef struct f1setupRsp
{
  U32               transId;                      /* Uniquely identify transaction */
  U32               cuId;                         /* CU ID */
  char              cuName[CU_DU_NAME_LEN_MAX];   /* CU Name */
  F1CuActCellInfo   actCellInfo;                  /* cells to be activated */
  F1RrcVersion      rrcVersion;                   /* RRC version */
}F1SetupRsp;

typedef struct f1SetupFail
{
   F1FailureIE   setupFail;
}F1SetupFail;

/* At reception of the RESET message the gNB-DU shall release all allocated resources on F1 
 * and radio resources related to the UE association(s) indicated explicitly or implicitly in 
 * the RESET message and remove the indicated UE contexts including F1AP ID. */
typedef struct f1Reset
{
  U32              transId;   /* Uniquely identify transaction */
  F1FailCause      cause;     /* Failure cause */
  F1ResetType      resetType; /* type of reset */
}F1Reset;

/* After the gNB-CU has released all assigned F1 resources and the UE F1AP IDs for all indicated
 * UE associations which can be used for new UE-associated logical F1-connections over the F1 interface,
 *  the gNB-CU shall respond with the RESET ACKNOWLEDGE message. */
typedef struct f1ResetAck
{
  U32                    transId;         /* Uniquely identify transaction */
  F1LogicalConnUeAssoc   ueAssocLogicalConn[MAX_F1_CONNECTIONS]; /* UE associated logical F1-connection list */
  F1CritDiagnostic       critDiagnostic;  /* Critical diagnostics */
}F1ResetAck;

typedef struct f1ErrorInd
{
  U32                transId;         /* Uniquely identify transaction */
  F1Entity           errorOrigin;     /* Specifies if error is originated at DU or CU */
  F1FailCause        cause;           /* Failure cause */
/* If failure is due to Ue related message. */
  U32                gnbCuUeF1apId;   /* gNB-CU UE F1AP Id */
  U32                gnbDuUeF1apId;   /* gNB-DU UE F1AP Id */
  F1CritDiagnostic   critDiagnostic;  /* Critical diagnostics */
}F1ErrorInd;

typedef struct f1GnbDuCfgUpd
{
  U32                 transId;                             /* Uniquely identify transaction */
  F1DuSrvdCellInfo    srvdCellLstAdd[MAXCELLINGNBDU];      /* Served cell list to be added */
  F1DuSrvdCellToDel   srvdCellLstMod[MAXCELLINGNBDU];      /* Served cell list to be modified */
  NrEcgi              srvdCellLstDel[MAXCELLINGNBDU];      /* Served cell list to be deleted */
  F1CellStatus        cellStatus[MAXCELLINGNBDU];          /* Cell status */
  F1DedSIDelUE        ueLst[MAX_UEID];                     /* Ue list that requires dedicated SI delivery */
  U32                 gnbDuId;
  F1TnlAssocToRmv     gnbDuTnlAssocRmv[MAXNUMOFTNLASSOCS];  /* TNL Assoc list to remove */ 
}F1GnbDuCfgUpd;

typedef struct f1GnbDuCfgUpdAck
{
   U32                transId;                      /* Uniquely identify transaction */
   F1CuActCellInfo    cellLstAct[MAXCELLINGNBDU];    /* List of cells to be activated */   
   F1CritDiagnostic   critDiagnostic;               /* Critical diagnostics */
   NrEcgi             cellLstDeact[MAXCELLINGNBDU]; /* List of cells to be deactivated */
}F1GnbDuCfgUpdAck;

typedef struct f1GnbDuCfgUpdFail
{
   F1FailureIE   gnbDuCfgUpdFail;
}F1GnbDuCfgUpdFail;

/* Sent by the gNB-CU to transfer updated information associated to an F1-C interface instance */
typedef struct f1GnbCuCfgUpd
{
    U32                 transId;                            /* Uniquely identifies transaction */
    F1CuActCellInfo     cellLstAct[MAXCELLINGNBDU];         /* List of cells to be activated */
    NrEcgi              cellLstDeact[MAXCELLINGNBDU];       /* List of cells to be deactivated */
    F1TnlAssoc          assocLstAdd[MAXNUMOFTNLASSOCS];     /* List of TNL assocs to be added */
    F1TnlAssoc          assocLstUpd[MAXNUMOFTNLASSOCS];     /* List of TNL assocs to be updated */
    F1TnlAssocToRmv     assocLstRmv[MAXNUMOFTNLASSOCS];     /* List of TNL assocs to be removed */
    F1CellBarred        cellToBarList[MAXCELLINGNBDU];      /* List of Cells to be barred */
    F1ProtectEUTRARsrc  protectEutraRsrcList[MAXCELLINENB]; /* List of Protected EUTRA resources */
}F1GnbCuCfgUpd;

/* Sent by a gNB-DU to a gNB-CU to acknowledge update of information
 * associated to an F1-C interface instance */
typedef struct f1GnbCuCfgUpdAck
{
    U32                   transId;                              /* Uniquely identify transaction */
    F1ActCellFail         actCellFailList[MAXCELLINGNBDU];      /* Cells failed to be activated list */
    F1CritDiagnostic      critDiagnostic;                       /* Critical diagnostics */
    F1TnlAssocAddr        assocSetupList[MAXNUMOFTNLASSOCS];     /* TNL Assoc Setup list */
    F1TnlAssocSetupFail   assocSetupFailList[MAXNUMOFTNLASSOCS]; /* TNL Assoc Setup fail list */
    F1DedSIDelUE          dedSiDelUelist[MAXNUMOFUEID];          /* Dedicated SI delivery needed UE list */
}F1GnbCuCfgUpdAck;

typedef struct f1GnbCuCfgUpdFail
{
    F1FailureIE   gnbCuCfgUpdFail;
}F1GnbCuCfgUpdFail;

/* This procedure enables coordination of radio resource allocation between a gNB-CU and
 * a gNB-DU. Coordination request is sent by CU to DU */
typedef struct f1GnbDuRsrcCoordReq
{
   U32           transId;              /* Uniquely identifies transaction */
   F1ReqType     reqType;              /* Request type */
   U8            cellResCoordReqCont;  /* Container for X2AP E-UTRA - NR cell resource coordination request */
   F1IgnoreReq   ignoreReq;            /* Ignore coordination request */
}F1GnbDuRsrcCoordReq;

/* This message is sent by a gNB-DU to a gNB-CU, to express the desired resource allocation
 * for data traffic, as a response to the GNB-DU RESOURCE COORDINATION REQUEST. */
typedef struct f1GnbDuRsrcCoordRsp
{
    U32   transId;               /* Uniquely identifies transaction */
    U8    cellResCoordRspCont;   /* Container for X2AP E-UTRA - NR cell resource coordination response */
}F1GnbDuRsrcCoordRsp;

/* This message is sent by the gNB-DU to indicate to the gNB-CU its status of overload */
typedef struct f1GnbDuStatusInd
{
   U32                  transId;      /* Uniquely identifies transaction */
   F1GnbDuOvrloadInfo   ovrloadInfo;  /* gNB-DU overloaded information */
}F1GnbDuStatusInd;

/* This message is sent by either the gNB-DU or the gNB-CU to intiate the removal
 * of the interface instance and the related resources */
typedef struct f1RmvReq
{
   U32   transId; /* Uniquely identifies transaction */
}F1RmvReq;

/* This message acknowledges the initiation of removal of the interface
 * instance and the related resources */
typedef struct f1RmvRsp
{
   U32                transId;     /* Uniquely identifies transaction */
   F1CritDiagnostic   critDiagnos; /* Crititcality diagnostics */
}F1RmvRsp;

/* This message indicates that removing the interface instance and the related
 * resources cannot be accepted */
typedef struct f1RmvFail
{
   U32                transId;         /* Uniquely identifies transaction */
   F1FailCause        cause;          /* Failure cause */
   F1CritDiagnostic   critDiagnostic; /* Criticality diagnostics */
}F1RmvFail;

/* This message is sent by the gNB-CU to indicate to the gNB-DU a need to reduce
 * the rate at which UEs access the network. */
typedef struct f1NwkAccessRateRed
{
   U32               transId;        /* Uniquely identifies transaction */
   F1UacAssistInfo   uacAssistInfo;  /* UAC Assistance Information */
}F1NwkAccessRateRed;

typedef struct f1Ipaddr
{
 Bool ipV4Pres;
 U32  ipV4Addr; 
 Bool ipV6Pres;
 U8   ipV6Addr[MAX_IPV6_LEN];
}F1IpAddr;

typedef struct f1SctpParams
{
   F1IpAddr  duIpAddr;
   U16       duPort;
   F1IpAddr  cuIpAddr;
   U16       cuPort;
}F1SctpParams;

typedef struct duCfgParams
{
   F1SctpParams       sctpParams;                    /* SCTP Params */
   U32                maxUe;
   U32                duId;
   U8                 duName[CU_DU_NAME_LEN_MAX]; 
   F1DuSrvdCellInfo   srvdCellLst[MAXCELLINGNBDU];   /* Serving cell list */
   F1RrcVersion       rrcVersion;                    /* RRC version */
   ClCellCfg          clCellCfg;                     /* CL cell configuration */

}DuCfgParams;

#endif /* __DU_COMMON_H__ */

/**********************************************************************
         End of file
**********************************************************************/
