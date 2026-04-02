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

#define UFSHCI_QUIRK_SKIP_MANUAL_WB_FLUSH_CTRL		0x100000
#define UFSHCD_QUIRK_BROKEN_AUTO_HIBERN8		0x200000
#define UFSHCI_QUIRK_BROKEN_REQ_LIST_CLR		0x800000
#define UFSHCD_QUIRK_PRDT_BYTE_GRAN			0x1000000

#define DME_LOCAL	0
#define DME_PEER	1
#define ATTR_SET_NOR	0

#define UFSHCD_ANDROID_QUIRK_CUSTOM_PA_TACTIVATE	0x1
#define UFSHCD_ANDROID_QUIRK_KEYS_IN_PRDT		0x2

/* Used to differentiate the power management options */
enum ufs_pm_op {
	UFS_RUNTIME_PM,
	UFS_SYSTEM_PM,
	UFS_SHUTDOWN_PM,
};

enum ufs_notify_change_status {
	PRE_CHANGE,
	POST_CHANGE,
};

/* Host <-> Device UniPro Link state */
enum uic_link_state {
	UIC_LINK_OFF_STATE	= 0, /* Link powered down or disabled */
	UIC_LINK_ACTIVE_STATE	= 1, /* Link is in Fast/Slow/Sleep state */
	UIC_LINK_HIBERN8_STATE	= 2, /* Link is in Hibernate state */
	UIC_LINK_BROKEN_STATE	= 3, /* Link is in broken state */
};

/*
 * UFS Power management levels.
 * Each level is in increasing order of power savings, except DeepSleep
 * which is lower than PowerDown with power on but not PowerDown with
 * power off.
 */
enum ufs_pm_level {
	UFS_PM_LVL_0,
	UFS_PM_LVL_1,
	UFS_PM_LVL_2,
	UFS_PM_LVL_3,
	UFS_PM_LVL_4,
	UFS_PM_LVL_5,
	UFS_PM_LVL_6,
	UFS_PM_LVL_MAX
};

/**
 * struct uic_command - UIC command structure
 * @command: UIC command
 * @argument1: UIC command argument 1
 * @argument2: UIC command argument 2
 * @argument3: UIC command argument 3
 * @cmd_active: Indicate if UIC command is outstanding
 * @done: UIC command completion
 */
struct uic_command {
	u32 command;
	u32 argument1;
	u32 argument2;
	u32 argument3;
	int cmd_active;
	struct completion done;
};

enum ufshcd_res {
       RES_UFS,
       RES_MCQ,
       RES_MCQ_SQD,
       RES_MCQ_SQIS,
       RES_MCQ_CQD,
       RES_MCQ_CQIS,
       RES_MCQ_VS,
       RES_MAX,
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
	struct utp_upiu_req *ucd_req_ptr;
	struct utp_upiu_rsp *ucd_rsp_ptr;
	dma_addr_t utrd_dma_addr;
	dma_addr_t ucd_req_dma_addr;
	dma_addr_t ucd_rsp_dma_addr;

	struct scsi_cmnd *cmd;
	enum ufs_dev_pwr_mode command_type;
	int task_tag;
	unsigned int lun;
	bool intr_cmd;
	union ufs_crypto_cfg_entry crypto_cfg;
	bool crypto_enabled;
	int crypto_key_slot;
	u64 data_unit_num;
	struct ufshcd_sg_entry *ucd_prdt_ptr;
	dma_addr_t ucd_prdt_dma_addr;

	ktime_t compl_time_stamp;
	ktime_t queue_time_stamp;
	u64 issue_time_stamp_local_clock;
	u64 compl_time_stamp_local_clock;
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

struct ufs_clk_info {
	struct list_head list;
	struct clk *clk;
	const char *name;
	unsigned int min_freq;
	unsigned int max_freq;
	unsigned int curr_freq;
	bool enabled;
	bool keep_lp;
};

#define UFS_EVENT_HIST_SIZE 8
#define UFS_EVENT_HIST_LENGTH UFS_EVENT_HIST_SIZE
struct ufs_event_hist {
	int pos;
	u32 val[UFS_EVENT_HIST_LENGTH];
	u64 tstamp[UFS_EVENT_HIST_LENGTH];
	unsigned long long cnt;
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
	UFS_EVT_NL_ERR,
	UFS_EVT_TL_ERR,
	UFS_EVT_DME_SET_GET_ERR,
	UFS_EVT_MAX_ERR_REINIT,
	UFS_EVT_DEV_LOSS,
	UFS_EVT_DEV_RESET,
	UFS_EVT_HOST_RESET,
	UFS_EVT_ABORT,
	UFS_EVT_WL_RES_ERR,
	UFS_EVT_WL_SUSP_ERR,

	UFS_EVT_CNT
};

struct ufs_stats {
	u32 hibern8_exit_cnt;
	u32 last_intr_status;
	u64 last_hibern8_exit_tstamp;
	u64 last_intr_ts;
	struct ufs_event_hist event[UFS_EVT_CNT];
};

struct ufs_saved_pwr_info {
	struct ufs_pa_layer_attr info;
	bool is_valid;
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
	struct work_struct suspend_work;
	struct work_struct resume_work;
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
	int active_reqs;
	bool suspend_on_no_request;
	struct workqueue_struct *workq;
	ktime_t window_start_t;
	struct ufs_saved_pwr_info saved_pwr_info;
	u32 min_gear;
};

#define UFS_HBA_MONITOR_QUEUES_COUNT 32
struct ufs_hba_monitor {
	u32  lat_bins[UFS_HBA_MONITOR_QUEUES_COUNT][10];
	u32  nr_queued[UFS_HBA_MONITOR_QUEUES_COUNT];
	bool enabled;

	unsigned long chunk_size;
	unsigned long nr_sec_rw[2];
	ktime_t total_busy[2];
	unsigned long nr_req[2];
	ktime_t lat_sum[2];
	ktime_t lat_max[2];
	ktime_t lat_min[2];
	u32 nr_queued_monitor[2];
	ktime_t busy_start_ts[2];
	ktime_t enabled_ts;
};

struct ufshpb_dev_info {
	unsigned int num_hw_queues;
	unsigned int num_user_queues;
};

struct ufs_hba_variant_params {
	struct ufs_pa_layer_attr dev_cap;
	u16 hs_rate;
	struct devfreq_dev_profile devfreq_profile;
	struct devfreq_simple_ondemand_data ondemand_data;
};

enum ufshcd_state {
	UFSHCD_STATE_RESET,
	UFSHCD_STATE_OPERATIONAL,
	UFSHCD_STATE_EH_BEGIN_RESUME,
	UFSHCD_STATE_EH_SCHEDULED,
	UFSHCD_STATE_EH_SCHEDULED_FATAL,
	UFSHCD_STATE_EH_SCHEDULED_NON_FATAL,
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
	UFSHCD_CAP_KEEP_AUTO_BKOPS_ENABLED_EXCEPT_SUSPEND = 1 << 6,
	UFSHCD_CAP_WB_EN				= 1 << 7,
	UFSHCD_CAP_DEEPSLEEP				= 1 << 8,
	UFSHCD_CAP_WB_WITH_CLK_SCALING			= 1 << 9,
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

struct ufs_hba;

struct ufs_pm_lvl_states {
	enum ufs_dev_pwr_mode dev_state;
	enum uic_link_state link_state;
};

struct ufs_hw_queue {
	void __iomem *mcq_sq_head;
	void __iomem *mcq_sq_tail;
	void __iomem *mcq_cq_head;
	void __iomem *mcq_cq_tail;
	void *sqe_base_addr;
	dma_addr_t sqe_dma_addr;
	void *cqe_base_addr;
	dma_addr_t cqe_dma_addr;
	u32 sq_tail_slot;
	u32 cq_tail_slot;
	u32 cq_head_slot;
	u32 max_entries;
	u32 id;
	spinlock_t sq_lock;
	spinlock_t cq_lock;
};

struct ufs_hba_variant_ops {
	const char *name;
	int	(*init)(struct ufs_hba *);
	void	(*exit)(struct ufs_hba *);
	u32	(*get_ufs_hci_version)(struct ufs_hba *);
	int	(*clk_scale_notify)(struct ufs_hba *, bool, enum ufs_notify_change_status);
	void	(*event_notify)(struct ufs_hba *, enum ufs_event_type, void *);
	int	(*setup_clocks)(struct ufs_hba *, bool, enum ufs_notify_change_status);
	int	(*setup_regulators)(struct ufs_hba *, bool);
	int	(*hce_enable_notify)(struct ufs_hba *, bool);
	int	(*link_startup_notify)(struct ufs_hba *, bool);
	int	(*pwr_change_notify)(struct ufs_hba *, enum ufs_notify_change_status, struct ufs_pa_layer_attr *, struct ufs_pa_layer_attr *);
	void	(*setup_xfer_req)(struct ufs_hba *, int, bool);
	void	(*setup_task_mgmt)(struct ufs_hba *, int, u8);
	void	(*hibern8_notify)(struct ufs_hba *, enum uic_cmd_dme, enum ufs_notify_change_status);
	int	(*apply_dev_quirks)(struct ufs_hba *);
	void	(*fixup_dev_quirks)(struct ufs_hba *);
	int	(*suspend)(struct ufs_hba *, enum ufs_pm_op, enum ufs_notify_change_status);
	int	(*resume)(struct ufs_hba *, enum ufs_pm_op);
	void	(*dbg_register_dump)(struct ufs_hba *);
	int	(*phy_initialization)(struct ufs_hba *);
	int	(*device_reset)(struct ufs_hba *);
	void	(*config_scaling_param)(struct ufs_hba *, struct devfreq_dev_profile *, struct devfreq_simple_ondemand_data *);
	int	(*program_key)(struct ufs_hba *, const union ufs_crypto_cfg_entry *, int);
	void	(*reinit_notify)(struct ufs_hba *);
	int	(*mcq_config_resource)(struct ufs_hba *);
	int	(*get_hba_mac)(struct ufs_hba *);
	int	(*op_runtime_config)(struct ufs_hba *);
	int	(*get_outstanding_cqs)(struct ufs_hba *, unsigned long *);
	int	(*config_esi)(struct ufs_hba *);
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

#define ufshcd_writel(hba, val, reg)   \
	writel((val), (hba)->mmio_base + (reg))
#define ufshcd_readl(hba, reg) \
	readl((hba)->mmio_base + (reg))

/**
 * ufshcd_rmwl - perform read/modify/write for a controller register
 * @hba: per adapter instance
 * @mask: mask to apply on read value
 * @val: actual value to write
 * @reg: register address
 */
static inline void ufshcd_rmwl(struct ufs_hba *hba, u32 mask, u32 val, u32 reg)
{
	u32 tmp;

	tmp = ufshcd_readl(hba, reg);
	tmp &= ~mask;
	tmp |= (val & mask);
	ufshcd_writel(hba, tmp, reg);
}

/**
 * ufshcd_set_variant - set variant specific data to the hba
 * @hba: per adapter instance
 * @variant: pointer to variant specific data
 */
static inline void ufshcd_set_variant(struct ufs_hba *hba, void *variant)
{
	hba->priv = variant;
}

/**
 * ufshcd_get_variant - get variant specific data from the hba
 * @hba: per adapter instance
 */
static inline void *ufshcd_get_variant(struct ufs_hba *hba)
{
	return hba->priv;
}

#define ufshcd_is_hs_mode(pwr_info) \
	((pwr_info)->pwr_rx == FAST_MODE || (pwr_info)->pwr_rx == FASTAUTO_MODE)

static inline bool ufshcd_is_wb_allowed(struct ufs_hba *hba)
{
	return hba->caps & UFSHCD_CAP_WB_EN;
}

static inline size_t ufshcd_sg_entry_size(struct ufs_hba *hba)
{
	return hba->sg_entry_size;
}

static inline bool ufshcd_is_auto_hibern8_supported(struct ufs_hba *hba)
{
	return (hba->capabilities & MASK_AUTO_HIBERN8_SUPPORT) &&
		!(hba->quirks & UFSHCD_QUIRK_BROKEN_AUTO_HIBERN8);
}

static inline bool ufshcd_is_clkscaling_supported(struct ufs_hba *hba)
{
	return hba->caps & UFSHCD_CAP_CLK_SCALING;
}

static inline bool ufshcd_enable_wb_if_scaling_up(struct ufs_hba *hba)
{
	return hba->caps & UFSHCD_CAP_WB_WITH_CLK_SCALING;
}

int ufshcd_wb_toggle_buf_flush(struct ufs_hba *hba, bool enable);

static inline void ufshcd_set_link_active(struct ufs_hba *hba)
{
	hba->uic_link_state = UIC_LINK_ACTIVE_STATE;
}

int ufshcd_config_pwr_mode(struct ufs_hba *hba,
			struct ufs_pa_layer_attr *desired_pwr_mode);

#define ufshcd_dme_get(hba, attr_sel, mib_val) \
	ufshcd_dme_get_attr(hba, attr_sel, mib_val, DME_LOCAL)

void ufshcd_remove(struct ufs_hba *hba);
int ufshcd_system_suspend(struct device *dev);
int ufshcd_system_resume(struct device *dev);
int ufshcd_runtime_suspend(struct device *dev);
int ufshcd_runtime_resume(struct device *dev);

void ufshcd_update_evt_hist(struct ufs_hba *hba, u32 id, u32 val);

#endif /* _UFSHCD_H */
