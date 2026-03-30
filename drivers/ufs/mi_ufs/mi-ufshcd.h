/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Universal Flash Storage Host controller driver
 * Copyright (C) 2011-2013 Samsung India Software Operations
 * Copyright (c) 2013-2016, The Linux Foundation. All rights reserved.
 *
 * Authors:
 *	Santosh Yaraganavi <santosh.sy@samsung.com>
 *	Vinayak Holikatti <h.vinayak@samsung.com>
 */

#ifndef _MI_UFSHCD_H
#define _MI_UFSHCD_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>
#include <linux/workqueue.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/bitops.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/regulator/consumer.h>
#include <linux/bitfield.h>
#include <linux/devfreq.h>
#include <linux/blk-crypto-profile.h>
#include <ufs/ufshcd.h>
#include "mi-unipro.h"

#include <asm/irq.h>
#include <asm/byteorder.h>
#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_tcq.h>
#include <scsi/scsi_dbg.h>
#include <scsi/scsi_eh.h>
#include <linux/android_kabi.h>

#include "mi-ufs.h"
#include "mi_ufs_quirks.h"
#include "mi-ufshci.h"

#if defined(CONFIG_CLD)
#include "cld/mi_cld.h"
#endif

#define UFSHCD_DRIVER_VERSION "0.2"

#define REG_UTP_TRANSFER_REQ_LIST_COMPL		0x64
#define UFSHCD_QUIRK_ALIGN_SG_WITH_PAGE_SIZE	UFSHCD_QUIRK_4KB_DMA_ALIGNMENT

#define ufshcd_is_link_broken(hba) ((hba)->uic_link_state == \
				   UIC_LINK_BROKEN_STATE)
#define ufshcd_set_link_off(hba) ((hba)->uic_link_state = UIC_LINK_OFF_STATE)
#define ufshcd_set_link_active(hba) ((hba)->uic_link_state = \
				    UIC_LINK_ACTIVE_STATE)
#define ufshcd_set_link_hibern8(hba) ((hba)->uic_link_state = \
				    UIC_LINK_HIBERN8_STATE)
#define ufshcd_set_link_broken(hba) ((hba)->uic_link_state = \
				    UIC_LINK_BROKEN_STATE)

#define ufshcd_set_ufs_dev_active(h) \
	((h)->curr_dev_pwr_mode = UFS_ACTIVE_PWR_MODE)
#define ufshcd_set_ufs_dev_sleep(h) \
	((h)->curr_dev_pwr_mode = UFS_SLEEP_PWR_MODE)
#define ufshcd_set_ufs_dev_poweroff(h) \
	((h)->curr_dev_pwr_mode = UFS_POWERDOWN_PWR_MODE)
#define ufshcd_is_ufs_dev_active(h) \
	((h)->curr_dev_pwr_mode == UFS_ACTIVE_PWR_MODE)
#define ufshcd_is_ufs_dev_sleep(h) \
	((h)->curr_dev_pwr_mode == UFS_SLEEP_PWR_MODE)
#define ufshcd_is_ufs_dev_poweroff(h) \
	((h)->curr_dev_pwr_mode == UFS_POWERDOWN_PWR_MODE)

/*
* customer debug interface
*/
struct UFS_ERR_STATE_DEBUG {
       u64 err_occurred; /*if happend err*/
       char err_reason[10][32]; /*err reason*/
};

struct UFS_DATA {
       struct UFS_ERR_STATE_DEBUG ufs_err_state;
};

struct UFS_DATA *get_ufs_data(void);

int mi_ufshcd_query_descriptor_retry(struct ufs_hba *hba,
				     enum query_opcode opcode,
				     enum desc_idn idn, u8 index, u8 selector,
				     u8 *desc_buf, int *buf_len);

int mi_ufshcd_read_desc_param(struct ufs_hba *hba,
				  enum desc_idn desc_id,
				  int desc_index, u8 param_offset,
				  u8 *param_read_buf,
				  u8 param_size);

int mi_ufshcd_query_flag(struct ufs_hba *hba, enum query_opcode opcode,
			enum flag_idn idn, u8 index, bool *flag_res);

int mi_ufshcd_query_flag_retry(struct ufs_hba *hba,
	enum query_opcode opcode, enum flag_idn idn, u8 index, bool *flag_res);

u8 mi_ufshcd_wb_get_query_index(struct ufs_hba *hba);

int mi_ufshcd_read_string_desc(struct ufs_hba *hba, u8 desc_index,
			    u8 **buf, bool ascii);

static inline u32 mi_ufshci_version(u32 major, u32 minor)
{
	return (major << 8) + (minor << 4);
}

static inline bool ufshcd_has_utrlcnr(struct ufs_hba *hba)
{
	return (hba->ufs_version >= mi_ufshci_version(3, 0));
}

int ufshcd_runtime_idle(struct device *dev);

int ufshcd_wb_ctrl(struct ufs_hba *hba, bool enable);
int ufshcd_wb_toggle_flush_during_h8(struct ufs_hba *hba, bool set);
void ufshcd_wb_toggle_flush(struct ufs_hba *hba, bool enable);

#endif /* _MI_UFSHCD_H */
