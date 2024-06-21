/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2016 MediaTek Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
/*
 ** Id:
 */

/*! \file   "roaming_fsm.c"
 *    \brief  This file defines the FSM for Roaming MODULE.
 *
 *    This file defines the FSM for Roaming MODULE.
 */


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"

#if CFG_SUPPORT_ROAMING
/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
static uint8_t *apucDebugRoamingState[ROAMING_STATE_NUM] = {
	(uint8_t *) DISP_STRING("IDLE"),
	(uint8_t *) DISP_STRING("DECISION"),
	(uint8_t *) DISP_STRING("DISCOVERY"),
	(uint8_t *) DISP_STRING("ROAM")
};

static uint8_t *apucEventStr[ROAMING_EVENT_NUM] = {
	(uint8_t *) DISP_STRING("START"),
	(uint8_t *) DISP_STRING("DISCOVERY"),
	(uint8_t *) DISP_STRING("ROAM"),
	(uint8_t *) DISP_STRING("FAIL"),
	(uint8_t *) DISP_STRING("ABORT"),
	(uint8_t *) DISP_STRING("THRESHOLD_UPDATE"),
};

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
/*----------------------------------------------------------------------------*/
/*!
 * @brief Initialize the value in ROAMING_FSM_INFO_T for ROAMING FSM operation
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void roamingFsmInit(struct ADAPTER *prAdapter, uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	struct ROAMING_REPORT_INFO *prReportInfo;
	struct CONNECTION_SETTINGS *prConnSettings;
	const uint8_t aucZeroMacAddr[] = NULL_MAC_ADDR;

	DBGLOG(ROAMING, LOUD,
	       "[%d]->Init: Current Time = %u\n",
	       ucBssIndex,
	       kalGetTimeTick());

	prRoamingFsmInfo = aisGetRoamingInfo(prAdapter, ucBssIndex);
	prReportInfo = &prRoamingFsmInfo->rReportInfo;
	prConnSettings = aisGetConnSettings(prAdapter, ucBssIndex);

	/* 4 <1> Initiate FSM */
	prRoamingFsmInfo->fgIsEnableRoaming =
		prConnSettings->fgIsEnableRoaming;
	prRoamingFsmInfo->eCurrentState = ROAMING_STATE_IDLE;
	prRoamingFsmInfo->rRoamingDiscoveryUpdateTime = 0;
	prRoamingFsmInfo->fgDrvRoamingAllow = TRUE;
	prRoamingFsmInfo->eReason = ROAMING_REASON_POOR_RCPI;
	kalMemZero(&(prAdapter->rWifiVar.rRoamSlotInfo),
		sizeof(struct ROAMING_IDLE_INFO));
	prRoamingFsmInfo->ucRecoverBitmap = 0;
	kalMemZero(&prRoamingFsmInfo->rSkipBtmInfo,
			sizeof(struct ROAMING_SKIP_CONFIG));
	kalMemZero(&prRoamingFsmInfo->rSkipPerInfo,
		sizeof(struct ROAMING_SKIP_CONFIG));
	prRoamingFsmInfo->ucLastEssNum = 0;

	/* Initialize roaming report variables */
	prReportInfo->rRoamingStartTime = 0;
	COPY_MAC_ADDR(prReportInfo->aucPrevBssid, aucZeroMacAddr);
	COPY_MAC_ADDR(prReportInfo->aucCandBssid, aucZeroMacAddr);
	prReportInfo->cPrevRssi = 0;
	prReportInfo->cCandRssi = 0;
	prReportInfo->ucPrevChannel = 0;
	prReportInfo->ucCandChannel = 0;
	prReportInfo->eFailReason = ROAMING_FAIL_REASON_NOCANDIDATE;
}				/* end of roamingFsmInit() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Uninitialize the value in AIS_FSM_INFO_T for AIS FSM operation
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void roamingFsmUninit(struct ADAPTER *prAdapter, uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;

	DBGLOG(ROAMING, LOUD,
	       "[%d]->Uninit: Current Time = %u\n",
	       ucBssIndex,
	       kalGetTimeTick());

	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);

	prRoamingFsmInfo->eCurrentState = ROAMING_STATE_IDLE;
} /* end of roamingFsmUninit() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Send commands to firmware
 *
 * @param [IN P_ADAPTER_T]       prAdapter
 *        [IN P_ROAMING_PARAM_T] prParam
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void roamingFsmSendCmd(struct ADAPTER *prAdapter,
	struct CMD_ROAMING_TRANSIT *prTransit)
{
	uint32_t rStatus;
	uint8_t ucBssIndex = prTransit->ucBssidx;

	DBGLOG(ROAMING, INFO,
		"[%d]->Send(%s): Current Time = %u\n",
		ucBssIndex, apucEventStr[prTransit->u2Event],
		kalGetTimeTick());

	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				      CMD_ID_ROAMING_TRANSIT,	/* ucCID */
				      TRUE,	/* fgSetQuery */
				      FALSE,	/* fgNeedResp */
				      FALSE,	/* fgIsOid */
				      NULL,	/* pfCmdDoneHandler */
				      NULL,	/* pfCmdTimeoutHandler */
				      sizeof(struct CMD_ROAMING_TRANSIT),
				      /* u4SetQueryInfoLen */
				      (uint8_t *)prTransit, /* pucInfoBuffer */
				      NULL,	/* pvSetQueryBuffer */
				      0	/* u4SetQueryBufferLen */
				     );

	/* ASSERT(rStatus == WLAN_STATUS_PENDING); */
}				/* end of roamingFsmSendCmd() */

/*----------------------------------------------------------------------------*/

/*
 * @brief Check if need to do scan for roaming
 *
 * @param[out] fgIsNeedScan Set to TRUE if need to scan since
 *		there is roaming candidate in current scan result
 *		or skip roaming times > limit times
 * @return
 */
/*----------------------------------------------------------------------------*/
static u_int8_t roamingFsmIsNeedScan(
	struct ADAPTER *prAdapter,
	uint8_t ucBssIndex)
{
	struct AIS_SPECIFIC_BSS_INFO *asbi = NULL;
	struct LINK *prEssLink = NULL;
	u_int8_t fgIsNeedScan = TRUE;

	asbi = aisGetAisSpecBssInfo(prAdapter, ucBssIndex);
	if (asbi == NULL) {
		DBGLOG(ROAMING, WARN, "ais specific bss info is NULL\n");
		return TRUE;
	}

	prEssLink = &asbi->rCurEssLink;

#if CFG_SUPPORT_ROAMING_SKIP_ONE_AP
	/*
	 * Start skip roaming scan mechanism if only one ESSID AP
	 */
	if (prEssLink->u4NumElem == 1) {
		struct BSS_DESC *prBssDesc;

		/* Get current BssDesc */
		prBssDesc = aisGetTargetBssDesc(prAdapter, ucBssIndex);
		if (prBssDesc) {
			DBGLOG(ROAMING, INFO,
				"roamingFsmSteps: RCPI:%d RoamSkipTimes:%d\n",
				prBssDesc->ucRCPI, asbi->ucRoamSkipTimes);
			if (prBssDesc->ucRCPI > 90) {
				/* Set parameters related to Good Area */
				asbi->ucRoamSkipTimes = 3;
				asbi->fgGoodRcpiArea = TRUE;
				asbi->fgPoorRcpiArea = FALSE;
			} else {
				if (asbi->fgGoodRcpiArea) {
					asbi->ucRoamSkipTimes--;
				} else if (prBssDesc->ucRCPI > 67) {
					/*Set parameters related to Poor Area*/
					if (!asbi->fgPoorRcpiArea) {
						asbi->ucRoamSkipTimes = 2;
						asbi->fgPoorRcpiArea = TRUE;
						asbi->fgGoodRcpiArea = FALSE;
					} else {
						asbi->ucRoamSkipTimes--;
					}
				} else {
					asbi->fgPoorRcpiArea = FALSE;
					asbi->fgGoodRcpiArea = FALSE;
					asbi->ucRoamSkipTimes--;
				}
			}

			if (asbi->ucRoamSkipTimes == 0) {
				asbi->ucRoamSkipTimes = 3;
				asbi->fgPoorRcpiArea = FALSE;
				asbi->fgGoodRcpiArea = FALSE;
				DBGLOG(ROAMING, INFO, "Need Scan\n");
			} else {
				struct CMD_ROAMING_SKIP_ONE_AP cmd = {0};

				cmd.fgIsRoamingSkipOneAP = 1;

				wlanSendSetQueryCmd(prAdapter,
				    CMD_ID_SET_ROAMING_SKIP,
				    TRUE,
				    FALSE,
				    FALSE, NULL, NULL,
				    sizeof(struct CMD_ROAMING_SKIP_ONE_AP),
				    (uint8_t *)&cmd, NULL, 0);

				fgIsNeedScan = FALSE;
			}
		} else {
			DBGLOG(ROAMING, WARN, "Target BssDesc is NULL\n");
		}
	}
#endif

	if (cnmP2pIsActive(prAdapter))
		fgIsNeedScan = FALSE;

	return fgIsNeedScan;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief The Core FSM engine of ROAMING for AIS Infra.
 *
 * @param [IN P_ADAPTER_T]          prAdapter
 *        [ENUM_ROAMING_STATE_T] eNextState Enum value of next AIS STATE
 *
 * @return (none)
 */
/*----------------------------------------------------------------------------*/
void roamingFsmSteps(struct ADAPTER *prAdapter,
	enum ENUM_ROAMING_STATE eNextState,
	uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	struct ROAMING_REPORT_INFO *prReportInfo;
	enum ENUM_ROAMING_STATE ePreviousState;
	u_int8_t fgIsTransition = (u_int8_t) FALSE;
	u_int32_t u4ScnResultsTimeout = prAdapter->rWifiVar.u4DiscoverTimeout;

	prRoamingFsmInfo = aisGetRoamingInfo(prAdapter, ucBssIndex);
	prReportInfo = &prRoamingFsmInfo->rReportInfo;
	do {
		if (prRoamingFsmInfo->eCurrentState < 0 ||
			prRoamingFsmInfo->eCurrentState >= ROAMING_STATE_NUM ||
			eNextState < 0 || eNextState >= ROAMING_STATE_NUM) {
			DBGLOG(ROAMING, STATE, "Invalid stat eNextState[%d]\n",
				eNextState);
			return;
		}
		/* Do entering Next State */
		DBGLOG(ROAMING, STATE,
		       "[ROAMING%d] TRANSITION: [%s] -> [%s]\n",
		       ucBssIndex,
		       apucDebugRoamingState[prRoamingFsmInfo->eCurrentState],
		       apucDebugRoamingState[eNextState]);

		/* NOTE(Kevin): This is the only place to
		 *    change the eCurrentState(except initial)
		 */
		ePreviousState = prRoamingFsmInfo->eCurrentState;
		prRoamingFsmInfo->eCurrentState = eNextState;

		fgIsTransition = (u_int8_t) FALSE;

		/* Do tasks of the State that we just entered */
		switch (prRoamingFsmInfo->eCurrentState) {
		/* NOTE(Kevin): we don't have to rearrange the sequence of
		 *   following switch case. Instead I would like to use a common
		 *   lookup table of array of function pointer
		 *   to speed up state search.
		 */
		case ROAMING_STATE_IDLE:
			prReportInfo->eFailReason =
					ROAMING_FAIL_REASON_NOCANDIDATE;
			break;
		case ROAMING_STATE_DECISION:
#if CFG_SUPPORT_DRIVER_ROAMING
			GET_CURRENT_SYSTIME(
				&prRoamingFsmInfo->rRoamingLastDecisionTime);
#endif
			prRoamingFsmInfo->eReason = ROAMING_REASON_POOR_RCPI;
			prReportInfo->eFailReason =
					ROAMING_FAIL_REASON_NOCANDIDATE;
			break;

		case ROAMING_STATE_DISCOVERY: {
			OS_SYSTIME rCurrentTime;
			u_int8_t fgIsNeedScan = FALSE;

#if CFG_SUPPORT_NCHO
			if (prAdapter->rNchoInfo.fgNCHOEnabled == TRUE)
				u4ScnResultsTimeout = 0;
#endif

			GET_CURRENT_SYSTIME(&rCurrentTime);
			if (CHECK_FOR_TIMEOUT(rCurrentTime,
			      prRoamingFsmInfo->rRoamingDiscoveryUpdateTime,
			      SEC_TO_SYSTIME(u4ScnResultsTimeout))) {
				DBGLOG(ROAMING, LOUD,
					"roamingFsmSteps: DiscoveryUpdateTime Timeout\n");

				fgIsNeedScan = roamingFsmIsNeedScan(prAdapter,
								ucBssIndex);
			}
			aisFsmRunEventRoamingDiscovery(
				prAdapter, fgIsNeedScan, ucBssIndex);
		}
		break;
		case ROAMING_STATE_ROAM:
			break;

		default:
			ASSERT(0); /* Make sure we have handle all STATEs */
		}
	} while (fgIsTransition);

	return;

}				/* end of roamingFsmSteps() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Transit to Decision state after join completion
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void roamingFsmRunEventStart(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	enum ENUM_ROAMING_STATE eNextState;
	struct BSS_INFO *prAisBssInfo;
	struct CMD_ROAMING_TRANSIT rTransit;
	struct AIS_FSM_INFO *prAisFsmInfo;

	prAisFsmInfo = aisGetAisFsmInfo(prAdapter, ucBssIndex);
	prRoamingFsmInfo = aisGetRoamingInfo(prAdapter, ucBssIndex);
	kalMemZero(&rTransit, sizeof(struct CMD_ROAMING_TRANSIT));

	/* Check Roaming Conditions */
	if (!(prRoamingFsmInfo->fgIsEnableRoaming))
		return;


	prAisBssInfo = aisGetAisBssInfo(prAdapter,
		ucBssIndex);
	if (prAisBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		return;

	DBGLOG(ROAMING, EVENT,
	       "[%d] EVENT-ROAMING START: Current Time = %u\n",
	       ucBssIndex,
	       kalGetTimeTick());

	/* IDLE, ROAM -> DECISION */
	/* Errors as DECISION, DISCOVERY -> DECISION */
	if (!(prRoamingFsmInfo->eCurrentState == ROAMING_STATE_IDLE
	      || prRoamingFsmInfo->eCurrentState == ROAMING_STATE_ROAM))
		return;

	eNextState = ROAMING_STATE_DECISION;
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		uint8_t i;

		/* start to monitor all links */
		for (i = 0; i < MLD_LINK_MAX; i++) {
			struct BSS_INFO *bss =
				aisGetLinkBssInfo(prAisFsmInfo, i);

			if (!bss ||
			     bss->eConnectionState != MEDIA_STATE_CONNECTED)
				continue;

			rTransit.u2Event = ROAMING_EVENT_START;
			rTransit.u2Data = bss->ucBssIndex;
			rTransit.ucBssidx = bss->ucBssIndex;
			roamingFsmSendCmd(prAdapter,
				(struct CMD_ROAMING_TRANSIT *) &rTransit);
		}

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState, ucBssIndex);
	}
	prRoamingFsmInfo->ucRecoverBitmap = 0;
}				/* end of roamingFsmRunEventStart() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Transit to Discovery state when deciding to find a candidate
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void roamingFsmRunEventDiscovery(struct ADAPTER *prAdapter,
	struct CMD_ROAMING_TRANSIT *prTransit)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	enum ENUM_ROAMING_STATE eNextState;
	uint8_t ucBssIndex = prTransit->ucBssidx;

	prRoamingFsmInfo = aisGetRoamingInfo(prAdapter, ucBssIndex);

	/* Check Roaming Conditions */
	if (!(prRoamingFsmInfo->fgIsEnableRoaming))
		return;

	DBGLOG(ROAMING, EVENT,
	       "[%d] EVENT-ROAMING DISCOVERY: Current Time = %u\n",
	       ucBssIndex,
	       kalGetTimeTick());

	/* DECISION -> DISCOVERY */
	/* Errors as IDLE, DISCOVERY, ROAM -> DISCOVERY */
	if (prRoamingFsmInfo->eCurrentState !=
	    ROAMING_STATE_DECISION) {
#if (CFG_SUPPORT_CONNAC3X == 1)
		struct CMD_ROAMING_TRANSIT rTransit = {0};

		/* fail when roaming is ongoing on anther link */
		if (prRoamingFsmInfo->eCurrentState > ROAMING_STATE_DECISION &&
			prRoamingFsmInfo->eCurrentState < ROAMING_STATE_NUM)
			DBGLOG(ROAMING, EVENT,
				"There's ongoing roaming link - ignore bssidx:%d\n",
				ucBssIndex);

		rTransit.u2Event = ROAMING_EVENT_ROAM;
		rTransit.ucBssidx = ucBssIndex;
		roamingFsmSendCmd(prAdapter,
			(struct CMD_ROAMING_TRANSIT *) &rTransit);

		rTransit.u2Event = ROAMING_EVENT_FAIL;
		rTransit.u2Data = ROAMING_FAIL_REASON_NOCANDIDATE;
		rTransit.ucBssidx = ucBssIndex;
		roamingFsmSendCmd(prAdapter,
			(struct CMD_ROAMING_TRANSIT *) &rTransit);
#endif
		return;
	}

	eNextState = ROAMING_STATE_DISCOVERY;
	/* DECISION -> DISCOVERY */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		struct BSS_INFO *prAisBssInfo;
		struct BSS_DESC *prBssDesc;
		struct BSS_DESC *prBssDescTarget;
		uint8_t arBssid[PARAM_MAC_ADDR_LEN];
		struct PARAM_SSID rSsid;
		struct CONNECTION_SETTINGS *prConnSettings;

		kalMemZero(&rSsid, sizeof(struct PARAM_SSID));
		prConnSettings =
			aisGetConnSettings(prAdapter, ucBssIndex);

		/* sync. rcpi with firmware */
		prAisBssInfo =
			&(prAdapter->rWifiVar.arBssInfoPool[NETWORK_TYPE_AIS]);
		prBssDesc = aisGetTargetBssDesc(prAdapter, ucBssIndex);
		if (prBssDesc) {
			COPY_SSID(rSsid.aucSsid, rSsid.u4SsidLen,
				  prBssDesc->aucSSID, prBssDesc->ucSSIDLen);
			COPY_MAC_ADDR(arBssid, prBssDesc->aucBSSID);
		} else  {
			COPY_SSID(rSsid.aucSsid, rSsid.u4SsidLen,
				  prConnSettings->aucSSID,
				  prConnSettings->ucSSIDLen);
			COPY_MAC_ADDR(arBssid, prConnSettings->aucBSSID);
		}

		prRoamingFsmInfo->ucRcpi = (uint8_t)(prTransit->u2Data & 0xff);
		prRoamingFsmInfo->ucThreshold =	prTransit->u2RcpiLowThreshold;

		prBssDesc = scanSearchBssDescByBssidAndSsid(prAdapter,
				arBssid, TRUE, &rSsid);
		if (prBssDesc) {
			prBssDesc->ucRCPI = prRoamingFsmInfo->ucRcpi;
			DBGLOG(ROAMING, INFO, "RCPI %u(%d)\n",
			     prBssDesc->ucRCPI, RCPI_TO_dBm(prBssDesc->ucRCPI));
		}

		prBssDescTarget = aisGetTargetBssDesc(prAdapter, ucBssIndex);
		if (prBssDescTarget && prBssDescTarget != prBssDesc) {
			prBssDescTarget->ucRCPI = prRoamingFsmInfo->ucRcpi;
			DBGLOG(ROAMING, WARN, "update target bss\n");
		}

		/* Save roaming reason code and PER value for AP selection */
		prRoamingFsmInfo->eReason = prTransit->eReason;
		if (prTransit->eReason == ROAMING_REASON_TX_ERR) {
			prRoamingFsmInfo->ucPER =
				(prTransit->u2Data >> 8) & 0xff;
			DBGLOG(ROAMING, INFO, "ucPER %u\n",
				prRoamingFsmInfo->ucPER);
		} else {
			prRoamingFsmInfo->ucPER = 0;
		}

#if CFG_SUPPORT_NCHO
		if (prRoamingFsmInfo->eReason == ROAMING_REASON_RETRY)
			DBGLOG(ROAMING, INFO,
				"NCHO enable=%d,trigger=%d,delta=%d,period=%d\n",
				prAdapter->rNchoInfo.fgNCHOEnabled,
				prAdapter->rNchoInfo.i4RoamTrigger,
				prAdapter->rNchoInfo.i4RoamDelta,
				prAdapter->rNchoInfo.u4RoamScanPeriod);
#endif

		/* Triggered by BTM/low RSSI/high PER */
		roamingRecordCurrentStatus(prAdapter, ucBssIndex);

		roamingFsmSteps(prAdapter, eNextState, ucBssIndex);
	}
}				/* end of roamingFsmRunEventDiscovery() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Transit to Roam state after Scan Done
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void roamingFsmRunEventRoam(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	enum ENUM_ROAMING_STATE eNextState;
	struct CMD_ROAMING_TRANSIT rTransit;

	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);
	kalMemZero(&rTransit, sizeof(struct CMD_ROAMING_TRANSIT));

	/* Check Roaming Conditions */
	if (!(prRoamingFsmInfo->fgIsEnableRoaming))
		return;


	DBGLOG(ROAMING, EVENT,
	       "[%d] EVENT-ROAMING ROAM: Current Time = %u\n",
	       ucBssIndex,
	       kalGetTimeTick());

	/* IDLE, ROAM -> DECISION */
	/* Errors as IDLE, DECISION, ROAM -> ROAM */
	if (prRoamingFsmInfo->eCurrentState !=
	    ROAMING_STATE_DISCOVERY)
		return;

	eNextState = ROAMING_STATE_ROAM;
	/* DISCOVERY -> ROAM */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		rTransit.u2Event = ROAMING_EVENT_ROAM;
		rTransit.ucBssidx = ucBssIndex;
		roamingFsmSendCmd(prAdapter,
			(struct CMD_ROAMING_TRANSIT *) &rTransit);

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState, ucBssIndex);
	}
}				/* end of roamingFsmRunEventRoam() */

void roamingFsmLogEvent(struct ADAPTER *adapter, uint8_t *event)
{
	char uevent[64];

	kalSnprintf(uevent, sizeof(uevent), "roam=Event:%s", event);
	kalSendUevent(uevent);
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief Transit to Decision state as being failed to find out any candidate
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void roamingFsmRunEventFail(struct ADAPTER *prAdapter,
	uint8_t ucReason, uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	enum ENUM_ROAMING_STATE eNextState;
	struct CMD_ROAMING_TRANSIT rTransit;

	prRoamingFsmInfo =
		aisGetRoamingInfo(prAdapter, ucBssIndex);
	kalMemZero(&rTransit, sizeof(struct CMD_ROAMING_TRANSIT));

	/* Check Roaming Conditions */
	if (!(prRoamingFsmInfo->fgIsEnableRoaming))
		return;


	DBGLOG(ROAMING, STATE,
	       "[%d] EVENT-ROAMING FAIL: reason %x Current Time = %u\n",
	       ucBssIndex,
	       ucReason, kalGetTimeTick());

	kalRoamingReport(prAdapter, ucBssIndex, FALSE);

	/* IDLE, ROAM -> DECISION */
	/* Errors as IDLE, DECISION, DISCOVERY -> DECISION */
	if (prRoamingFsmInfo->eCurrentState != ROAMING_STATE_ROAM)
		return;

	eNextState = ROAMING_STATE_DECISION;
	/* ROAM -> DECISION */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		rTransit.u2Event = ROAMING_EVENT_FAIL;
		rTransit.u2Data = (uint16_t) (ucReason & 0xffff);
		rTransit.ucBssidx = ucBssIndex;
		roamingFsmSendCmd(prAdapter,
			(struct CMD_ROAMING_TRANSIT *) &rTransit);

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState, ucBssIndex);
	}
}				/* end of roamingFsmRunEventFail() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Transit to Idle state as beging aborted by other moduels, AIS
 *
 * @param [IN P_ADAPTER_T] prAdapter
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
void roamingFsmRunEventAbort(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamingFsmInfo;
	enum ENUM_ROAMING_STATE eNextState;
	struct CMD_ROAMING_TRANSIT rTransit;
	struct AIS_FSM_INFO *prAisFsmInfo;

	prAisFsmInfo = aisGetAisFsmInfo(prAdapter, ucBssIndex);
	prRoamingFsmInfo = aisGetRoamingInfo(prAdapter, ucBssIndex);
	kalMemZero(&rTransit, sizeof(struct CMD_ROAMING_TRANSIT));

	/* Check Roaming Conditions */
	if (!(prRoamingFsmInfo->fgIsEnableRoaming))
		return;

	DBGLOG(ROAMING, EVENT,
	       "[%d] EVENT-ROAMING ABORT: Current Time = %u\n",
	       ucBssIndex,
	       kalGetTimeTick());

	eNextState = ROAMING_STATE_IDLE;
	/* IDLE, DECISION, DISCOVERY, ROAM -> IDLE */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		uint8_t i;

		/* start to monitor all links */
		for (i = 0; i < MLD_LINK_MAX; i++) {
			struct BSS_INFO *bss =
				aisGetLinkBssInfo(prAisFsmInfo, i);

			if (!bss)
				continue;

			rTransit.u2Event = ROAMING_EVENT_ABORT;
			rTransit.ucBssidx = bss->ucBssIndex;
			roamingFsmSendCmd(prAdapter,
				(struct CMD_ROAMING_TRANSIT *) &rTransit);
		}

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState, ucBssIndex);
	}
	prRoamingFsmInfo->ucRecoverBitmap = 0;
}				/* end of roamingFsmRunEventAbort() */

/*----------------------------------------------------------------------------*/
/*!
 * @brief Process events from firmware
 *
 * @param [IN P_ADAPTER_T]       prAdapter
 *        [IN P_ROAMING_PARAM_T] prParam
 *
 * @return none
 */
/*----------------------------------------------------------------------------*/
uint32_t roamingFsmProcessEvent(struct ADAPTER *prAdapter,
	struct CMD_ROAMING_TRANSIT *prTransit)
{
	uint8_t ucBssIndex = prTransit->ucBssidx;

	DBGLOG(ROAMING, LOUD,
	       "[%d] ROAMING Process Events: Current Time = %u\n",
	       ucBssIndex,
	       kalGetTimeTick());

	if (prTransit->u2Event == ROAMING_EVENT_DISCOVERY) {
		DBGLOG(ROAMING, INFO,
			"ROAMING_EVENT_DISCOVERY Data[%u] RCPI[%u(%d)] PER[%u] Thr[%u(%d)] Reason[%d] Time[%u]\n",
			prTransit->u2Data,
			(prTransit->u2Data) & 0xff,      /* L[8], RCPI */
			RCPI_TO_dBm((prTransit->u2Data) & 0xff),
			(prTransit->u2Data >> 8) & 0xff, /* H[8], PER */
			prTransit->u2RcpiLowThreshold,
			RCPI_TO_dBm(prTransit->u2RcpiLowThreshold),
			prTransit->eReason,
			prTransit->u4RoamingTriggerTime);
		roamingFsmRunEventDiscovery(prAdapter, prTransit);
	} else if (prTransit->u2Event == ROAMING_EVENT_THRESHOLD_UPDATE) {
		DBGLOG(ROAMING, INFO,
			"ROAMING_EVENT_THRESHOLD_UPDATE RCPI H[%d(%d)] L[%d(%d)]\n",
			prTransit->u2RcpiHighThreshold,
			RCPI_TO_dBm(prTransit->u2RcpiHighThreshold),
			prTransit->u2RcpiLowThreshold,
			RCPI_TO_dBm(prTransit->u2RcpiLowThreshold));
	}

	return WLAN_STATUS_SUCCESS;
}

void roamingFsmSetRecoverBitmap(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex, uint8_t ucScenario)
{
	struct ROAMING_INFO *prRoamingFsmInfo = NULL;

	DBGLOG(ROAMING, INFO, "Set recover scenario: %d\n", ucScenario);

	prRoamingFsmInfo = aisGetRoamingInfo(prAdapter, ucBssIndex);
	prRoamingFsmInfo->ucRecoverBitmap |= BIT(ucScenario);
}

void roamingFsmDoRecover(struct ADAPTER *prAdapter, uint8_t ucBssIndex)
{
	struct ROAMING_INFO *prRoamInfo = NULL;
	struct BSS_INFO *prBssInfo = NULL;

	prRoamInfo = aisGetRoamingInfo(prAdapter, ucBssIndex);
	prBssInfo = aisGetAisBssInfo(prAdapter, ucBssIndex);

	DBGLOG(AIS, INFO, "Recover after join failure[%d]!\n",
		prRoamInfo->ucRecoverBitmap);

	if (prRoamInfo->ucRecoverBitmap & BIT(ROAMING_RECOVER_BSS_UPDATE))
		nicUpdateBss(prAdapter, ucBssIndex);
	else if (prRoamInfo->ucRecoverBitmap & BIT(ROAMING_RECOVER_RLM_SYNC))
		rlmSyncOperationParams(prAdapter, prBssInfo);

	prRoamInfo->ucRecoverBitmap = 0;
}

uint8_t roamingFsmInDecision(struct ADAPTER *prAdapter, uint8_t ucBssIndex)
{
	struct ROAMING_INFO *roam;
	enum ENUM_PARAM_CONNECTION_POLICY policy;
	struct CONNECTION_SETTINGS *setting;

	roam = aisGetRoamingInfo(prAdapter, ucBssIndex);
	setting = aisGetConnSettings(prAdapter, ucBssIndex);
	policy = setting->eConnectionPolicy;

	return IS_BSS_INDEX_AIS(prAdapter, ucBssIndex) &&
	       roam->fgIsEnableRoaming &&
	       roam->eCurrentState == ROAMING_STATE_DECISION &&
	       policy != CONNECT_BY_BSSID ?
	       TRUE : FALSE;
}

void roamingFillScanInfo(struct ADAPTER *ad, enum ENUM_BAND eBand,
	uint8_t ucChNum, uint16_t u2IdleTime)
{
	struct ROAMING_IDLE_INFO *prRoamSlotInfo =
		&(ad->rWifiVar.rRoamSlotInfo);
	uint8_t index = 0;

	if (eBand == BAND_2G4) {
		if (ucChNum < 1 || ucChNum > 14)
			return;

		index = (ucChNum - 1);
		prRoamSlotInfo->au2ChIdleTime2G4[index] = u2IdleTime;
	} else if (eBand == BAND_5G) {
		if (ucChNum < 36 || ucChNum > 165)
			return;

		if (ucChNum >= 36 && ucChNum <= 64)
			index = (ucChNum - 36) / 4;
		else if (ucChNum >= 100 && ucChNum <= 144)
			index = (ucChNum - 68) / 4;
		else if (ucChNum >= 149 && ucChNum <= 165)
			index = (ucChNum - 69) / 4;
		prRoamSlotInfo->au2ChIdleTime5G[index] = u2IdleTime;
#if (CFG_SUPPORT_WIFI_6G == 1)
	} else if (eBand == BAND_6G) {
		if (ucChNum < 1 || ucChNum > 233)
			return;

		index = (ucChNum - 1) / 4;
		prRoamSlotInfo->au2ChIdleTime6G[index] = u2IdleTime;
#endif
	}
}

uint16_t roamingGetChIdleSlot(struct ADAPTER *ad, enum ENUM_BAND eBand,
	uint8_t ucChNum)
{
	struct ROAMING_IDLE_INFO *prRoamSlotInfo =
		&(ad->rWifiVar.rRoamSlotInfo);
	uint8_t index = 0;
	uint16_t u2Slot = 0;

	if (eBand == BAND_2G4) {
		if (ucChNum < 1 || ucChNum > 14)
			return 0;

		index = (ucChNum - 1);
		u2Slot = prRoamSlotInfo->au2ChIdleTime2G4[index];
	} else if (eBand == BAND_5G) {
		if (ucChNum < 36 || ucChNum > 165)
			return 0;

		if (ucChNum >= 36 && ucChNum <= 64)
			index = (ucChNum - 36) / 4;
		else if (ucChNum >= 100 && ucChNum <= 144)
			index = (ucChNum - 68) / 4;
		else if (ucChNum >= 149 && ucChNum <= 165)
			index = (ucChNum - 69) / 4;
		u2Slot = prRoamSlotInfo->au2ChIdleTime5G[index];
#if (CFG_SUPPORT_WIFI_6G == 1)
	} else if (eBand == BAND_6G) {
		if (ucChNum < 1 || ucChNum > 233)
			return 0;

		index = (ucChNum - 1) / 4;
		u2Slot = prRoamSlotInfo->au2ChIdleTime6G[index];
#endif
	}

	return u2Slot;
}

void roamingRecordCurrentStatus(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex)
{
	struct ROAMING_REPORT_INFO *prReportInfo =
		aisGetRoamingReport(prAdapter, ucBssIndex);
	struct BSS_INFO *prAisBssInfo =
		aisGetAisBssInfo(prAdapter, ucBssIndex);
	const uint8_t aucZeroMacAddr[] = NULL_MAC_ADDR;

	/* Record current status */
	COPY_MAC_ADDR(prReportInfo->aucPrevBssid,
		      prAisBssInfo->aucBSSID);
	prReportInfo->cPrevRssi =
		prAdapter->rLinkQuality.rLq[ucBssIndex].cRssi;
	prReportInfo->ucPrevChannel = prAisBssInfo->ucPrimaryChannel;

	/* Reset candidate status */
	COPY_MAC_ADDR(prReportInfo->aucCandBssid, aucZeroMacAddr);
	prReportInfo->cCandRssi = 0;
	prReportInfo->ucCandChannel = 0;

	GET_CURRENT_SYSTIME(&prReportInfo->rRoamingStartTime);
	DBGLOG(AIS, INFO, "Start roaming: %u\n",
			prReportInfo->rRoamingStartTime);
}

void roamingRecordCandiStatus(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex, struct BSS_DESC *prBssDesc)
{
	struct ROAMING_REPORT_INFO *prReportInfo =
			aisGetRoamingReport(prAdapter, ucBssIndex);

	COPY_MAC_ADDR(prReportInfo->aucCandBssid, prBssDesc->aucBSSID);
	prReportInfo->cCandRssi = RCPI_TO_dBm(prBssDesc->ucRCPI);
	prReportInfo->ucCandChannel = prBssDesc->ucChannelNum;
}

void roamingUpdateSaaFailReason(struct ADAPTER *prAdapter,
	uint8_t ucBssIndex, enum ENUM_AA_STATE eAuthAssocState)
{
	struct ROAMING_REPORT_INFO *prReportInfo =
			aisGetRoamingReport(prAdapter, ucBssIndex);

	if (eAuthAssocState >= SAA_STATE_SEND_AUTH1 &&
	    eAuthAssocState <= SAA_STATE_EXTERNAL_AUTH)
		prReportInfo->eFailReason = ROAMING_FAIL_REASON_AUTH_FAIL;
	else if (eAuthAssocState >= SAA_STATE_SEND_ASSOC1 &&
		 eAuthAssocState <= SAA_STATE_WAIT_ASSOC2)
		prReportInfo->eFailReason = ROAMING_FAIL_REASON_ASSOC_FAIL;
	else
		DBGLOG(AIS, ERROR, "Invalid AuthAssocState [%d]\n",
				    eAuthAssocState);
}
#endif