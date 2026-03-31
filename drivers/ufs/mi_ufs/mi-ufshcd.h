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
#include "../host/ufs-qcom.h"

struct ufscld_dev;
static inline struct ufscld_dev *mi_ufshcd_get_cld(struct ufs_hba *hba)
{
	struct ufs_qcom_host *host = (struct ufs_qcom_host *)ufshcd_get_variant(hba);

	return host ? host->cld : NULL;
}

/* Core UFS functions exported for mi_ufs */
int ufshcd_wait_for_doorbell_clr(struct ufs_hba *hba, u64 wait_timeout_us);
void ufshcd_scsi_block_requests(struct ufs_hba *hba);
void ufshcd_scsi_unblock_requests(struct ufs_hba *hba);

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

int mi_ufshcd_read_desc_param_sel(struct ufs_hba *hba, enum desc_idn desc_id,
			       int desc_index, u8 selector, u8 param_offset,
			       u8 *param_read_buf, u8 param_size);

void mi_ufshcd_map_desc_id_to_length(struct ufs_hba *hba, enum desc_idn desc_id,
				  int *desc_len);

int mi_ufshcd_query_flag(struct ufs_hba *hba, enum query_opcode opcode,
			enum flag_idn idn, u8 index, bool *flag_res);

int mi_ufshcd_query_flag_sel(struct ufs_hba *hba, enum query_opcode opcode,
			enum flag_idn idn, u8 index, u8 selector, bool *flag_res);

int mi_ufshcd_query_flag_retry(struct ufs_hba *hba,
	enum query_opcode opcode, enum flag_idn idn, u8 index, bool *flag_res);

int mi_ufshcd_query_attr(struct ufs_hba *hba, enum query_opcode opcode,
		      enum attr_idn idn, u8 index, u8 selector, u32 *attr_val);

int mi_ufshcd_dme_set_attr(struct ufs_hba *hba, u32 attr_sel,
			u8 attr_set, u32 mib_val, u8 peer);

int mi_ufshcd_dme_get_attr(struct ufs_hba *hba, u32 attr_sel,
			u32 *mib_val, u8 peer);

int mi_ufshcd_read_string_desc(struct ufs_hba *hba, u8 desc_index,
			    u8 **buf, bool ascii);

int mi_ufshcd_hold(struct ufs_hba *hba, bool async);
void mi_ufshcd_release(struct ufs_hba *hba);

int mi_ufshcd_send_uic_cmd(struct ufs_hba *hba, struct uic_command *uic_cmd);
int mi_ufshcd_link_recovery(struct ufs_hba *hba);
int mi_ufshcd_uic_hibern8_enter(struct ufs_hba *hba);
int mi_ufshcd_uic_hibern8_exit(struct ufs_hba *hba);

void mi_ufshcd_auto_hibern8_update(struct ufs_hba *hba, u32 ahit);
void mi_ufshcd_auto_hibern8_enable(struct ufs_hba *hba);

int mi_ufshcd_config_pwr_mode(struct ufs_hba *hba,
		struct ufs_pa_layer_attr *desired_pwr_mode);

int mi_ufshcd_make_hba_operational(struct ufs_hba *hba);
void mi_ufshcd_hba_stop(struct ufs_hba *hba);
int mi_ufshcd_hba_enable(struct ufs_hba *hba);

void mi_ufshcd_update_evt_hist(struct ufs_hba *hba, u32 id, u32 val);
int mi_ufshcd_bkops_ctrl(struct ufs_hba *hba,
			     enum bkops_status status);

int mi_ufshcd_dump_regs(struct ufs_hba *hba, size_t offset, size_t len,
		     const char *prefix);

void mi_ufshcd_scsi_unblock_requests(struct ufs_hba *hba);
void mi_ufshcd_scsi_block_requests(struct ufs_hba *hba);
void mi_ufshcd_delay_us(unsigned long us, unsigned long tolerance);

u32 mi_ufshcd_get_local_unipro_ver(struct ufs_hba *hba);

int mi_ufshcd_alloc_host(struct device *dev, struct ufs_hba **hba_handle);
void mi_ufshcd_dealloc_host(struct ufs_hba *hba);
int mi_ufshcd_init(struct ufs_hba *hba, void __iomem *mmio_base, unsigned int irq);
void mi_ufshcd_remove(struct ufs_hba *hba);
int mi_ufshcd_shutdown(struct ufs_hba *hba);
int mi_ufshcd_runtime_suspend(struct device *dev);
int mi_ufshcd_runtime_resume(struct device *dev);
int mi_ufshcd_system_suspend(struct device *dev);
int mi_ufshcd_system_resume(struct device *dev);
int mi_ufshcd_get_vreg(struct device *dev, struct ufs_vreg *vreg);
void mi_ufshcd_parse_dev_ref_clk_freq(struct ufs_hba *hba, struct clk *refclk);
int mi_ufshcd_exec_raw_upiu_cmd(struct ufs_hba *hba,
			     struct utp_upiu_req *req_upiu,
			     struct utp_upiu_req *rsp_upiu,
			     int msgcode,
			     u8 *desc_buff, int *buff_len,
			     enum query_opcode desc_op);
int mi_ufshcd_wait_for_doorbell_clr(struct ufs_hba *hba,
					u64 wait_timeout_us);

static inline u32 mi_ufshci_version(u32 major, u32 minor)
{
	return (major << 8) + (minor << 4);
}

static inline bool ufshcd_has_utrlcnr(struct ufs_hba *hba)
{
	return (hba->ufs_version >= mi_ufshci_version(3, 0));
}

int mi_ufshcd_runtime_idle(struct device *dev);

int mi_ufshcd_wb_ctrl(struct ufs_hba *hba, bool enable);
int mi_ufshcd_wb_toggle_flush_during_h8(struct ufs_hba *hba, bool set);
void ufshcd_wb_toggle_flush(struct ufs_hba *hba, bool enable);

#endif /* _MI_UFSHCD_H */
