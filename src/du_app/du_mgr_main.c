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

/* This file is the entry point for DU APP */

#include "du_mgr_main.h"
#include "du_sctp.h"

extern S16 kwUlActvTsk (Pst *, Buffer *);
extern S16 kwUlActvInit (Ent, Inst, Region, Reason);
extern S16 kwDlActvTsk (Pst *, Buffer *);
extern S16 kwDlActvInit (Ent, Inst, Region, Reason);
extern S16 rgActvTsk (Pst *, Buffer *);
extern S16 rgActvInit (Ent, Inst, Region, Reason);

/*******************************************************************
 *
 * @brief Initializes DU APP
 *
 * @details
 *
 *    Function : duAppInit
 *
 *    Functionality:
 *       - Registers and attaches TAPA tasks belonging to 
 *         DU_APP sys task
 *
 * @params[in] system task ID
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
S16 duAppInit(SSTskId sysTskId)
{
   /* Register DU APP TAPA Task for DU */
   if(SRegTTsk((Ent)ENTDUAPP, (Inst)DU_INST, (Ttype)TTNORM, (Prior)PRIOR0,
            duActvInit, (ActvTsk)duActvTsk) != ROK)
   {
      return RFAILED;
   }
   /* Attach DU APP TAPA Task for DU */
   if (SAttachTTsk((Ent)ENTDUAPP, (Inst)0, sysTskId)!= ROK)
   {
      return RFAILED;
   }

   printf("\nDU APP created and registered \
         to %d sys task\n", sysTskId);
   return ROK;
}
/*******************************************************************
 *
 * @brief Initializes SCTP task
 *
 * @details
 *
 *    Function : sctpInit
 *
 *    Functionality:
 *       - Registers and attaches TAPA tasks for SCTP receiver 
 *
 * @params[in] system task ID
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
S16 sctpInit(SSTskId sysTskId)
{
   /* Register SCTP TAPA Task */
   if(SRegTTsk((Ent)ENTSCTP, (Inst)0, (Ttype)TTNORM, (Prior)PRIOR0,
            sctpActvInit, (ActvTsk)sctpActvTsk) != ROK)
   {
      return RFAILED;
   }
   /* Attach SCTP TAPA Task */
   if (SAttachTTsk((Ent)ENTSCTP, (Inst)SCTP_INST, sysTskId)!= ROK)
   {
      return RFAILED;
   }

   printf("\nSCTP TAPA task created and registered to %d sys task\n", 
         sysTskId);
   return ROK;
}
/*******************************************************************
 *
 * @brief Initializes RLC DL, MAC TAPA task
 *
 * @details
 *
 *    Function : rlcDlInit
 *
 *    Functionality:
 *       - Registers and attaches TAPA tasks for MAC and RLC DL
 *
 * @params[in] system task ID
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
S16 rlcDlInit(SSTskId sysTskId)
{
   /* Register RLC DL TAPA Task */
   if(SRegTTsk((Ent)ENTKW, (Inst)1, (Ttype)TTNORM, (Prior)PRIOR0,
            kwDlActvInit, (ActvTsk)kwDlActvTsk) != ROK)
   {
      return RFAILED;
   }
   /* Attach RLC DL Task */
   if (SAttachTTsk((Ent)ENTKW, (Inst)1, sysTskId)!= ROK)
   {
      return RFAILED;
   }

   /* Register MAC TAPA Task */
   if(SRegTTsk((Ent)ENTRG, (Inst)0, (Ttype)TTNORM, (Prior)PRIOR0,
            rgActvInit, (ActvTsk)rgActvTsk) != ROK)
   {
      return RFAILED;
   }
   /* Attach MAC Task */
   if (SAttachTTsk((Ent)ENTRG, (Inst)0, sysTskId)!= ROK)
   {
      return RFAILED;
   }

   printf("\nRLC DL and MAC TAPA task created and registered to \
         %d sys task\n", sysTskId);
   return ROK;
}

/*******************************************************************
 *
 * @brief Initializes RLC UL TAPA task
 *
 * @details
 *
 *    Function : rlcUlInit
 *
 *    Functionality:
 *       - Registers and attaches TAPA task for RLC UL
 *
 * @params[in] system task ID
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
S16 rlcUlInit(SSTskId sysTskId)
{
   /* Register RLC UL TAPA Task */
   if(SRegTTsk((Ent)ENTKW, (Inst)0, (Ttype)TTNORM, (Prior)PRIOR0,
            kwUlActvInit, (ActvTsk)kwUlActvTsk) != ROK)
   {
      return RFAILED;
   }
   /* Attach RLC DL Task */
   if (SAttachTTsk((Ent)ENTKW, (Inst)0, sysTskId)!= ROK)
   {
      return RFAILED;
   }
   printf("\nRLC UL TAPA task created and registered to \
         %d sys task\n", sysTskId);
   return ROK;
}

/*******************************************************************
 *
 * @brief Initializes system and TAPA tasks
 *
 * @details
 *
 *    Function : commonInit
 *
 *    Functionality:
 *       - Registers and attaches system and TAPA tasks
 *
 * @params[in] void
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
S16 commonInit()
{
   /* Declare system task Ids */
   SSTskId du_app_stsk, sctp_stsk, rlc_ul_stsk, rlc_mac_cl_stsk;

   pthread_attr_t attr;

   SSetProcId(DU_PROC);

   /* system task for DU APP */
   SCreateSTsk(PRIOR0, &du_app_stsk);

   /* system task for RLC_DL and MAC */
   SCreateSTsk(PRIOR0, &rlc_mac_cl_stsk);
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

   /* system task for RLC UL */
   SCreateSTsk(PRIOR1, &rlc_ul_stsk);

   /* system task for SCTP receiver thread */
   SCreateSTsk(PRIOR0, &sctp_stsk);

   /* Create TAPA tasks */
   if(duAppInit(du_app_stsk) != ROK)
   {
      printf("\nDU APP TAPA Task initialization failed");
      return RFAILED;
   }

   if(sctpInit(sctp_stsk) != ROK)
   {
      printf("\nSCTP TAPA Task initialization failed");
      return RFAILED;
   }

   if(rlcDlInit(rlc_mac_cl_stsk) != ROK)
   {
      printf("\nRLC DL Tapa Task initialization failed");
      return RFAILED;
   } 

   if(rlcUlInit(rlc_ul_stsk) != ROK)
   {
     printf("\nRLC UL Tapa Task initialization failed");
     return RFAILED;
   } 
   return ROK;
}

/*******************************************************************
 *
 * @brief Initializes the DU
 *
 * @details
 *
 *    Function : duInit
 *
 *    Functionality:
 *       - Calls commonInit
 *       - Registers DU Layers
 *
 * @params[in] void
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
S16 duInit()
{
   int ret = ROK;
   if(commonInit() != ROK)
   {
      ret = RFAILED;
   } 
 
   return ret;
}

/*******************************************************************
 *
 * @brief Entry point for the DU APP 
 *
 * @details
 *
 *    Function : main
 *
 *    Functionality:
 *      - Read config params into duCfgParams
 *      - Initiate the init functions
 *
 * @params[in] void
 * @return ROK     - success
 *         RFAILED - failure
 *
 * ****************************************************************/
S16 tst(void)
{

   //Initialize TAPA layers
   if(duInit() != ROK)
   {
      return RFAILED;
   } 

   //Read all the configs from du_utils.c into duCfgParams
   duReadCfg();

#if 0
   //Establish SCTP connection
   while(ret == -1)
   {
      printf("\nEstablishing SCTP link with CU... \n");
      //ret = establishSctp(); //To be implemeted in du_sctp.c
   }
#endif
   return ROK;
}/* end of main()*/

/**********************************************************************
         End of file
**********************************************************************/
