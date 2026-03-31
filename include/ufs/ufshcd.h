/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Universal Flash Storage Host controller driver
 * Copyright (C) 2011-2013 Samsung India Software Operations
 * Copyright (c) 2013-2016, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, 2023 Google LLC
 */

#ifndef _UFSHCD_H
#define _UFSHCD_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/ratelimit.h>
#include <linux/scatterlist.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/pm_qos.h>
#include <linux/debugfs.h>
#include <linux/android_kabi.h>
#include <ufs/ufs.h>
#include <ufs/ufshci.h>
#include <ufs/unipro.h>

#include <linux/bitfield.h>
#include <linux/completion.h>
#include <linux/device.h>
#include <linux/device-mapper.h>
#include <linux/devfreq.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/rwsem.h>
#include <linux/semaphore.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/blk-crypto-profile.h>

#define UFSHCD_QUIRK_BROKEN_LCC				0x1
#define UFSHCD_QUIRK_DME_PEER_GET_CONFIG_ATTR_TIMEOUT	0x2
#define UFSHCD_QUIRK_BROKEN_PA_RXHSUNTERMINADAPT	0x4
#define UFSHCD_QUIRK_BROKEN_UFS_HCI_VERSION		0x8
#define UFSHCD_QUIRK_BROKEN_HCE				0x10
#define UFSHCD_QUIRK_BROKEN_PA_TACTIVATE		0x20
#define UFSHCD_QUIRK_SKIP_RESET_INTR_AGGR		0x40
#define UFSHCD_QUIRK_DELAY_BEFORE_LPM			0x80
#define UFSHCD_QUIRK_HOST_PA_TACTIVATE			0x100
#define UFSHCD_QUIRK_ANY_OFFLINE_LUN			0x200
#define UFSHCD_QUIRK_BROKEN_64BIT_ADDRESS		0x400
#define UFSHCD_QUIRK_BROKEN_DWORD_UTRD			0x800
#define UFSHCD_QUIRK_PRDT_BYTE_COUNT_WRONG		0x1000
#define UFSHCD_QUIRK_SKIP_PH_CONFIGURATION		0x2000
#define UFSHCD_QUIRK_BROKEN_CRYPTO			0x4000
#define UFSHCD_QUIRK_REINIT_AFTER_MAX_HIBERN8_ERR	0x8000
#define UFSHCD_QUIRK_MCQ_BROKEN_INTR_CONF		0x10000
#define UFSHCD_QUIRK_BROKEN_INTR_AGGR			0x20000
#define UFSHCD_QUIRK_HIBERN8_EXIT_WITH_LCC		0x40000
#define UFSHCD_QUIRK_REINIT_ON_DEV_LOSS			0x80000

#define UFSHCD_ANDROID_QUIRK_CUSTOM_PA_TACTIVATE	0x1

enum ufs_ref_clk_freq {
	REF_CLK_FREQ_19_2_MHZ	= 0,
	REF_CLK_FREQ_26_MHZ	= 1,
	REF_CLK_FREQ_38_4_MHZ	= 2,
	REF_CLK_FREQ_52_MHZ	= 3,
	REF_CLK_FREQ_INVAL	= -1,
};

struct ufs_pa_layer_attr {
	u32 gear_rx;
	u32 gear_tx;
	u32 lane_rx;
	u32 lane_tx;
	u32 pwr_rx;
	u32 pwr_tx;
	u32 hs_rate;
};

struct ufs_pwr_mode_info {
	bool is_valid;
	struct ufs_pa_layer_attr pwr_mode;
};

struct ufshcd_lrb {
	struct utp_transfer_req_desc *utr_descriptor_ptr;
	struct utp_transfer_cmd_desc *ucd_req_ptr;
	struct utp_transfer_cmd_desc *ucd_rsp_ptr;
	dma_addr_t utr_desc_dma_addr;
	dma_addr_t ucd_req_dma_addr;
	dma_addr_t ucd_rsp_dma_addr;

	struct scsi_cmnd *cmd;
	enum ufs_dev_pwr_mode command_type;
	int task_tag;
	unsigned int lun;
	bool intr_cmd;
	union ufs_crypto_cfg_entry crypto_cfg;
	bool crypto_enabled;

	ktime_t compl_time_stamp;
	ktime_t queue_time_stamp;
	ANDROID_KABI_RESERVE(1);
};

struct ufs_query {
	struct ufs_query_req request;
	void *descriptor;
	struct ufs_query_res response;
};

struct ufs_dev_cmd {
	struct ufs_query query;
	struct completion *done;
	struct mutex lock;
};

struct ufs_vreg {
	struct regulator *reg;
	const char *name;
	u32 min_uV;
	u32 max_uV;
	bool enabled;
	bool is_always_on;
	u32 max_uA;
};

struct ufs_vreg_info {
	struct ufs_vreg *vcc;
	struct ufs_vreg *vccq;
	struct ufs_vreg *vccq2;
	struct ufs_vreg *vdd_hba;
};

struct ufs_clk_info {
	struct list_head list;
	struct clk *clk;
	const char *name;
	unsigned int min_freq;
	unsigned int max_freq;
	bool enabled;
	bool keep_lp;
};

#define UFS_EVENT_HIST_SIZE 8
struct ufs_event_hist {
	u32 pos;
	u32 v[UFS_EVENT_HIST_SIZE];
	u64 t[UFS_EVENT_HIST_SIZE];
};

enum ufs_event_type {
	UFS_EVT_LINK_STARTUP_FAIL,
	UFS_EVT_RESUME_ERR,
	UFS_EVT_SUSPEND_ERR,
	UFS_EVT_HCE_FAIL,
	UFS_EVT_LINK_STUCK,
	UFS_EVT_CLK_GATING_ERR,
	UFS_EVT_FATAL_ERR,
	UFS_EVT_DME_ERR,
	UFS_EVT_AUTO_HIBERN8_ERR,
	UFS_EVT_PA_ERR,
	UFS_EVT_DL_ERR,
	UFS_EVT_DME_SET_GET_ERR,
	UFS_EVT_MAX_ERR_REINIT,
	UFS_EVT_DEV_LOSS,

	UFS_EVT_CNT
};

struct ufs_stats {
	u32 hibern8_exit_cnt;
	u32 last_intr_status;
	struct ufs_event_hist event[UFS_EVT_CNT];
};

enum ufs_clk_gating_state {
	CLKS_OFF,
	CLKS_ON,
	REQ_CLKS_OFF,
	REQ_CLKS_ON,
};

struct ufs_clk_gating {
	struct delayed_work gate_work;
	struct work_struct ungate_work;
	struct device_attribute delay_attr;
	struct device_attribute enable_attr;
	unsigned long delay_ms;
	enum ufs_clk_gating_state state;
	struct completion vops_done;
	bool is_enabled;
	bool active_reqs;
	bool is_initialized;
};

struct ufs_clk_scaling {
	struct devfreq_dev_status prev_status;
	struct delayed_work suspend_work;
	struct delayed_work resume_work;
	struct work_struct ungate_work;
	struct device_attribute enable_attr;
	struct device_attribute window_attr;
	struct device_attribute target_gear_attr;
	unsigned long target_gear;
	unsigned long window_ms;
	bool is_enabled;
	bool is_allowed;
	bool is_initialized;
	bool is_suspended;
	bool is_busy_started;
	bool is_scaling_up;
	ktime_t busy_start_t;
	ktime_t tot_busy_t;
};

#define UFS_HBA_MONITOR_QUEUES_COUNT 32
struct ufs_hba_monitor {
	u32  lat_bins[UFS_HBA_MONITOR_QUEUES_COUNT][10];
	u32  nr_queued[UFS_HBA_MONITOR_QUEUES_COUNT];
	bool enabled;
};

struct ufshpb_dev_info {
	unsigned int num_hw_queues;
	unsigned int num_user_queues;
};

struct ufs_hba_variant_params {
	struct ufs_pa_layer_attr dev_cap;
	u16 hs_rate;
};

enum ufshcd_state {
	UFSHCD_STATE_RESET,
	UFSHCD_STATE_OPERATIONAL,
	UFSHCD_STATE_EH_BEGIN_RESUME,
	UFSHCD_STATE_EH_SCHEDULED,
	UFSHCD_STATE_ERROR,
	UFSHCD_STATE_DEVICE_LOSS,
};

enum ufshcd_eh_flags {
	UFSHCD_EH_IN_PROGRESS = 1 << 0,
};

enum ufshcd_caps {
	UFSHCD_CAP_CLK_GATING				= 1 << 0,
	UFSHCD_CAP_CLK_SCALING				= 1 << 1,
	UFSHCD_CAP_HIBERN8_WITH_CLK_GATING		= 1 << 2,
	UFSHCD_CAP_AUTO_BKOPS_WITH_COMMAND_LOGGING	= 1 << 3,
	UFSHCD_CAP_INTR_AGGR				= 1 << 4,
	UFSHCD_CAP_MCQ					= 1 << 5,
};

struct ufshcd_res_info {
	void __iomem *base;
	resource_size_t resource_size;
};

enum ufshcd_mcq_opr {
	OPR_SQD,
	OPR_CQD,
	OPR_MAX,
};

struct ufshcd_mcq_opr_info_t {
	unsigned long offset;
	unsigned long stride;
	void __iomem *base;
};

struct ufs_hba {
	void __iomem *mmio_base;

	/* Virtual memory reference */
	struct utp_transfer_cmd_desc *ucdl_base_addr;
	struct utp_transfer_req_desc *utrdl_base_addr;
	struct utp_task_req_desc *utmrdl_base_addr;

	/* DMA memory reference */
	dma_addr_t ucdl_dma_addr;
	dma_addr_t utrdl_dma_addr;
	dma_addr_t utmrdl_dma_addr;

	struct Scsi_Host *host;
	struct device *dev;
	struct scsi_device *ufs_device_wlun;

#ifdef CONFIG_SCSI_UFS_HWMON
	struct device *hwmon_device;
#endif

	enum ufs_dev_pwr_mode curr_dev_pwr_mode;
	enum uic_link_state uic_link_state;
	/* Desired UFS power management level during runtime PM */
	enum ufs_pm_level rpm_lvl;
	/* Desired UFS power management level during system PM */
	enum ufs_pm_level spm_lvl;
	int pm_op_in_progress;

	/* Auto-Hibernate Idle Timer register value */
	u32 ahit;

	struct ufshcd_lrb *lrb;

	unsigned long outstanding_tasks;
	spinlock_t outstanding_lock;
	unsigned long outstanding_reqs;

	u32 capabilities;
	int nutrs;
	u32 mcq_capabilities;
	int nutmrs;
	u32 reserved_slot;
	u32 ufs_version;
	const struct ufs_hba_variant_ops *vops;
	struct ufs_hba_variant_params *vps;
	void *priv;
#ifdef CONFIG_SCSI_UFS_VARIABLE_SG_ENTRY_SIZE
	size_t sg_entry_size;
#endif
	unsigned int irq;
	bool is_irq_enabled;
	enum ufs_ref_clk_freq dev_ref_clk_freq;

	unsigned int quirks;	/* Deviations from standard UFSHCI spec. */

	unsigned int android_quirks; /* for UFSHCD_ANDROID_QUIRK_* flags */

	/* Device deviations from standard UFS device spec. */
	unsigned int dev_quirks;

	struct blk_mq_tag_set tmf_tag_set;
	struct request_queue *tmf_queue;
	struct request **tmf_rqs;

	struct uic_command *active_uic_cmd;
	struct mutex uic_cmd_mutex;
	struct completion *uic_async_done;

	enum ufshcd_state ufshcd_state;
	bool logical_unit_scan_finished;
	u32 eh_flags;
	u32 intr_mask;
	u16 ee_ctrl_mask;
	u16 ee_drv_mask;
	u16 ee_usr_mask;
	struct mutex ee_ctrl_mutex;
	bool is_powered;
	bool shutting_down;
	struct semaphore host_sem;

	/* Work Queues */
	struct workqueue_struct *eh_wq;
	struct work_struct eh_work;
	struct work_struct eeh_work;

	/* HBA Errors */
	u32 errors;
	u32 uic_error;
	u32 saved_err;
	u32 saved_uic_err;
	struct ufs_stats ufs_stats;
	bool force_reset;
	bool force_pmc;
	bool silence_err_logs;

	/* Device management request data */
	struct ufs_dev_cmd dev_cmd;
	ktime_t last_dme_cmd_tstamp;
	int nop_out_timeout;

	/* Keeps information of the UFS device connected to this host */
	struct ufs_dev_info dev_info;
	bool auto_bkops_enabled;
	struct ufs_vreg_info vreg_info;
	struct list_head clk_list_head;

	/* Number of requests aborts */
	int req_abort_count;

	/* Number of lanes available (1 or 2) for Rx/Tx */
	u32 lanes_per_direction;
	struct ufs_pa_layer_attr pwr_info;
	struct ufs_pwr_mode_info max_pwr_info;

	struct ufs_clk_gating clk_gating;
	/* Control to enable/disable host capabilities */
	u32 caps;

	struct devfreq *devfreq;
	struct ufs_clk_scaling clk_scaling;
	bool system_suspending;
	bool is_sys_suspended;

	enum bkops_status urgent_bkops_lvl;
	bool is_urgent_bkops_lvl_checked;

	struct mutex wb_mutex;
	struct rw_semaphore clk_scaling_lock;
	atomic_t scsi_block_reqs_cnt;

	struct device           bsg_dev;
	struct request_queue    *bsg_queue;
	struct delayed_work rpm_dev_flush_recheck_work;

	ANDROID_KABI_RESERVE(1); /* Original reserve for HPB or other */

	union {
		struct {
			struct ufs_hba_monitor monitor;
			bool scsi_host_added;
		};
		ANDROID_OEM_DATA(1);
	};

	u32 luns_avail;
	unsigned int nr_hw_queues;
	unsigned int nr_queues[HCTX_MAX_TYPES];
	bool complete_put;
	bool ext_iid_sup;
	bool mcq_sup;
	bool mcq_enabled;
	struct ufshcd_res_info res[RES_MAX];
	void __iomem *mcq_base;
	struct ufs_hw_queue *uhq;
	struct ufs_hw_queue *dev_cmd_queue;
	struct ufshcd_mcq_opr_info_t mcq_opr[OPR_MAX];

	ANDROID_KABI_RESERVE(2);
};

/* Additional definitions and exports omitted for brevity, keeping only core structure for KMI fix */

#endif /* UFSHCD_H */
