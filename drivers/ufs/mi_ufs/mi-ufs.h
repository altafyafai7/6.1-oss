/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Universal Flash Storage Host controller driver
 * Copyright (C) 2011-2013 Samsung India Software Operations
 *
 * Authors:
 *	Santosh Yaraganavi <santosh.sy@samsung.com>
 *	Vinayak Holikatti <h.vinayak@samsung.com>
 */

#ifndef _MI_UFS_H
#define _MI_UFS_H

#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/android_kabi.h>
#include <uapi/scsi/scsi_bsg_ufs.h>
#include <ufs/ufs.h>

#define QUERY_DESC_MAX_SIZE       255
#define QUERY_DESC_MIN_SIZE       2
#define QUERY_DESC_HDR_SIZE       2
#define QUERY_OSF_SIZE            (GENERAL_UPIU_REQUEST_SIZE - \
					(sizeof(struct utp_upiu_header)))

#define UPIU_HEADER_DWORD(byte3, byte2, byte1, byte0)\
			cpu_to_be32((byte3 << 24) | (byte2 << 16) |\
			 (byte1 << 8) | (byte0))

#define UFS_RPMB_UNIT		0xC4

#ifndef _UFS_DESC_DEF_SIZE_DEFINED
#define _UFS_DESC_DEF_SIZE_DEFINED
enum ufs_desc_def_size {
	QUERY_DESC_DEVICE_DEF_SIZE		= 0x59,
	QUERY_DESC_CONFIGURATION_DEF_SIZE	= 0x90,
	QUERY_DESC_UNIT_DEF_SIZE		= 0x2D,
	QUERY_DESC_INTERCONNECT_DEF_SIZE	= 0x06,
	QUERY_DESC_GEOMETRY_DEF_SIZE		= 0x48,
	QUERY_DESC_POWER_DEF_SIZE		= 0x62,
	QUERY_DESC_HEALTH_DEF_SIZE		= 0x25,
};
#endif

struct ufs_query_res {
	u8 response;
	u32 value;
};

#define UFS_VREG_VCC_MIN_UV	   2700000 /* uV */
#define UFS_VREG_VCC_MAX_UV	   3600000 /* uV */
#define UFS_VREG_VCC_1P8_MIN_UV    1700000 /* uV */
#define UFS_VREG_VCC_1P8_MAX_UV    1950000 /* uV */
#define UFS_VREG_VCCQ_MIN_UV	   1140000 /* uV */
#define UFS_VREG_VCCQ_MAX_UV	   1260000 /* uV */
#define UFS_VREG_VCCQ2_MIN_UV	   1700000 /* uV */
#define UFS_VREG_VCCQ2_MAX_UV	   1950000 /* uV */

/*
 * This enum is used in string mapping in include/trace/events/ufs.h.
 */
#ifndef _UFS_TRACE_STR_T_DEFINED
#define _UFS_TRACE_STR_T_DEFINED
enum ufs_trace_str_t {
	UFS_CMD_SEND, UFS_CMD_COMP, UFS_DEV_COMP,
	UFS_QUERY_SEND, UFS_QUERY_COMP, UFS_QUERY_ERR,
	UFS_TM_SEND, UFS_TM_COMP, UFS_TM_ERR
};
#endif

/*
 * Transaction Specific Fields (TSF) type in the UPIU package, this enum is
 * used in include/trace/events/ufs.h for UFS command trace.
 */
#ifndef _UFS_TRACE_TSF_T_DEFINED
#define _UFS_TRACE_TSF_T_DEFINED
enum ufs_trace_tsf_t {
	UFS_TSF_CDB, UFS_TSF_OSF, UFS_TSF_TM_INPUT, UFS_TSF_TM_OUTPUT
};
#endif

#endif /* _MI_UFS_H */
