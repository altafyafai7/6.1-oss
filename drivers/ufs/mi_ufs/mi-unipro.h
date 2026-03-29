/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * drivers/scsi/ufs/unipro.h
 *
 * Copyright (C) 2013 Samsung Electronics Co., Ltd.
 */

#ifndef _MI_UNIPRO_H_
#define _MI_UNIPRO_H_

#include <ufs/unipro.h>

#define is_mphy_tx_attr(attr)			(attr < RX_MODE)
#define RX_ADV_FINE_GRAN_STEP(x)		((((x) & 0x3) << 1) | 0x1)
#define SYNC_LEN_FINE(x)			((x) & 0x3F)
#define SYNC_LEN_COARSE(x)			((1 << 6) | ((x) & 0x3F))
#define PREP_LEN(x)				((x) & 0xF)

#define RX_MIN_ACTIVATETIME_UNIT_US		100
#define HIBERN8TIME_UNIT_US			100

#define UNIPRO_CB_OFFSET(x)			(0x8000 | x)

/* Adpat type for PA_TXHSADAPTTYPE attribute */
#define PA_REFRESH_ADAPT       0x00
#define PA_INITIAL_ADAPT       0x01
#define PA_NO_ADAPT            0x03

#define PA_TACTIVATE_TIME_UNIT_US	10
#define PA_HIBERN8_TIME_UNIT_US		100

/*Other attributes*/
#define VS_MPHYCFGUPDT		0xD085
#define VS_DEBUGOMC		0xD09E
#define VS_POWERSTATE		0xD083

#define PA_GRANULARITY_MIN_VAL	1
#define PA_GRANULARITY_MAX_VAL	6

#define DL_FC0ProtectionTimeOutVal_Default	8191
#define DL_TC0ReplayTimeOutVal_Default		65535
#define DL_AFC0ReqTimeOutVal_Default		32767
#define DL_FC1ProtectionTimeOutVal_Default	8191
#define DL_TC1ReplayTimeOutVal_Default		65535
#define DL_AFC1ReqTimeOutVal_Default		32767

#define PWRMODE_MASK		0xF
#define PWRMODE_RX_OFFSET	4

#ifndef _MI_BOOLEAN_DEFINED
#define _MI_BOOLEAN_DEFINED
#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif
/* Boolean attribute values */
enum {
	FALSE = 0,
	TRUE,
};
#endif

/* CPort setting */
#define E2EFC_ON	(1 << 0)
#define E2EFC_OFF	(0 << 0)
#define CSD_N_ON	(0 << 1)
#define CSD_N_OFF	(1 << 1)
#define CSV_N_ON	(0 << 2)
#define CSV_N_OFF	(1 << 2)
#define CPORT_DEF_FLAGS	(CSV_N_OFF | CSD_N_OFF | E2EFC_OFF)

#endif /* _MI_UNIPRO_H_ */
