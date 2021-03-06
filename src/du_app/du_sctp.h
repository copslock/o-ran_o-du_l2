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

/* This file contains all SCTP related functionality */

#ifndef __DU_SCTP_H__
#define __DU_SCTP_H__

#include "du_mgr.h"
#include "cm_inet.h"
#include "cm_tpt.h"

#include "cm_inet.x"
#include "cm_tpt.x"

S16 sctpActvInit(Ent entity, Inst inst, Region region, Reason reason);
S16 sctpActvTsk(Pst *pst, Buffer *mBuf);
void sctpAssocReq();
void sendToDuApp(Buffer *mBuf, Event event);
S16 sctpOutMsgSend(Buffer *mBuf);
typedef S16 (*SctpNtfy) ARGS((Buffer *mBuf, CmInetSctpNotification *ntfy));

S16 cmPkSctpNtfy(CmInetSctpNotification *ntfy);
S16 cmUnpkSctpNtfy(SctpNtfy func, Pst *pst, Buffer *mBuf);

#define MAX_RETRY 5

#endif

/**********************************************************************
         End of file
**********************************************************************/
