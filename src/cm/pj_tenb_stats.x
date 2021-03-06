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

/**********************************************************************
 
    Name:  
 
    Type:  
 
    Desc:  
 
    File:  pj_tenb_stats.x 
 
**********************************************************************/
 
/** @file pj_tenb_stats.x
*/

#ifdef TENB_STATS
#ifndef __TENBSTATSL2X__
#define __TENBSTATSL2X__

#include "cm_tenb_stats.x"
#include "pj_tenb_stats.h"
 
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef TSInfL2CellStats TSL2CellStatsCb;

typedef struct tSL2UeStatsCb
{
   CmLList         lnk;
   U32             inUse;

   TSInfL2UeStats  stats;
}TSL2UeStatsCb;

EXTERN Void TSL2AllocStatsMem ARGS((
 Region region,
 Pool   pool
));
EXTERN TSL2UeStatsCb* TSL2AllocUeStatsBlk ARGS((
 U16   rnti
));
EXTERN Void TSL2DeallocUeStatsBlk ARGS((
 U16         rnti,
 TSL2UeStatsCb   *statsCb
));
EXTERN TSL2CellStatsCb* TSL2AllocCellStatsBlk ARGS((
 U32 cellId
));
EXTERN Void TSL2DeallocCellStatsBlk ARGS((
 U32 cellId
));
EXTERN Void TSL2SendStatsToApp ARGS((
 Pst    *pst,
 SuId   suId
));

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __TENBSTATSL2X__ */
#endif /* TENB_STATS */

/**********************************************************************
         End of file
**********************************************************************/
