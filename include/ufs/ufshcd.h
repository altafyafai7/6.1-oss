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
#include <linux/msi.h>
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

enum ufshcd_mcq_opr {
	OPR_SQD,
	OPR_SQIS,
	OPR_CQD,
	OPR_CQIS,
	OPR_MAX,
};

#define ufsmcq_writel(hba, val, reg)	writel((val), (hba)->mmio_base + (reg))
#define ufsmcq_writelx(hba, val, reg)	writel((val), (hba)->mmio_base + (reg))

#define UFSHCD "ufshcd"
#define UFSHCD_DRIVER_VERSION "0.2"

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
#define UFSHCD_QUIRK_BROKEN_UIC_CMD			0x2000000
#define UFSHCD_QUIRK_DELAY_BEFORE_DME_CMDS		0x4000000
#define UFSHCD_QUIRK_DME_PEER_ACCESS_AUTO_MODE		0x8000000
#define UFSHCD_QUIRK_BROKEN_PA_RXHSUNTERMCAP		0x10000000
#define UFSHCD_QUIRK_HIBERN_FASTAUTO			0x20000000
#define UFSHCD_QUIRK_SKIP_DEF_UNIPRO_TIMEOUT_SETTING	0x40000000
#define UFSHCI_QUIRK_BROKEN_HCE				UFSHCD_QUIRK_BROKEN_HCE
#define UFSHCI_QUIRK_SKIP_RESET_INTR_AGGR		UFSHCD_QUIRK_SKIP_RESET_INTR_AGGR
#define UFSHCD_QUIRK_MCQ_BROKEN_INTR			UFSHCD_QUIRK_MCQ_BROKEN_INTR_CONF
#define UFSHCD_QUIRK_PERFORM_LINK_STARTUP_ONCE		(1ULL << 31)
#define UFSHCD_QUIRK_4KB_DMA_ALIGNMENT			(1ULL << 32)
#define UFSHCD_QUIRK_BROKEN_OCS_FATAL_ERROR		(1ULL << 33)
#define UFSHCD_QUIRK_MCQ_BROKEN_RTC			(1ULL << 34)

#define DME_LOCAL	0
#define DME_PEER	1
#define ATTR_SET_NOR	0

#define UFSHCD_ANDROID_QUIRK_CUSTOM_PA_TACTIVATE	0x1
#define UFSHCD_ANDROID_QUIRK_KEYS_IN_PRDT		0x2
#define UFSHCD_ANDROID_QUIRK_BROKEN_CRYPTO_ENABLE	0x4
#define UFSHCD_ANDROID_QUIRK_CUSTOM_CRYPTO_PROFILE	0x8

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
	struct ufs_pa_layer_attr info;
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
	ktime_t issue_time_stamp;
	u64 issue_time_stamp_local_clock;
	u64 compl_time_stamp_local_clock;
	bool req_abort_skip;
	ANDROID_KABI_RESERVE(1);
};

struct ufs_query {
	struct ufs_query_req request;
	void *descriptor;
	struct ufs_query_res response;
};

enum dev_cmd_type {
	DEV_CMD_TYPE_NOP		= 0x0,
	DEV_CMD_TYPE_QUERY		= 0x1,
	DEV_CMD_TYPE_RPMB		= 0x2,
};

struct ufs_dev_cmd {
	enum dev_cmd_type type;
	struct mutex lock;
	struct completion *complete;
	struct ufs_query query;
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
	bool keep_link_active;
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
	bool is_suspended;
	struct workqueue_struct *clk_gating_workq;
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
	u32 hba_enable_delay_us;
	u32 wb_flush_threshold;
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
	UFSHCD_CAP_TEMP_NOTIF				= 1 << 10,
	UFSHCD_CAP_RPM_AUTOSUSPEND			= 1 << 11,
	UFSHCD_CAP_AGGR_POWER_COLLAPSE			= 1 << 12,
	UFSHCD_CAP_AUTO_BKOPS_SUSPEND			= 1 << 13,
	UFSHCD_CAP_CRYPTO				= 1 << 14,
};

struct ufshcd_res_info {
	void __iomem *base;
	resource_size_t resource_size;
	const char *name;
	struct resource *resource;
};

struct ufshcd_mcq_opr_info_t {
	unsigned long offset;
	unsigned long stride;
	void __iomem *base;
};

struct ufs_hba;
struct ufs_dev_quirk;

struct ufs_pm_lvl_states {
	enum ufs_dev_pwr_mode dev_state;
	enum uic_link_state link_state;
};

struct ufs_hw_queue {
	struct mutex sq_mutex;
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
	int	(*hce_enable_notify)(struct ufs_hba *, enum ufs_notify_change_status);
	int	(*link_startup_notify)(struct ufs_hba *, enum ufs_notify_change_status);
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

	u64 quirks;	/* Deviations from standard UFSHCI spec. */
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

	unsigned char desc_size[QUERY_DESC_IDN_MAX];

#ifdef CONFIG_SCSI_UFS_CRYPTO
	union ufs_crypto_capabilities crypto_capabilities;
	u32 crypto_cfg_register;
	union ufs_crypto_cap_entry *crypto_cap_array;
	struct blk_crypto_profile crypto_profile;
#endif

#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs_root;
	u32 debugfs_ee_rate_limit_ms;
	struct delayed_work debugfs_ee_work;
#endif

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

/* Wrapper functions for safely calling variant operations */
static inline const char *ufshcd_get_var_name(struct ufs_hba *hba)
{
	if (hba->vops)
		return hba->vops->name;
	return "";
}

static inline int ufshcd_vops_init(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->init)
		return hba->vops->init(hba);
	return 0;
}

static inline void ufshcd_vops_exit(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->exit)
		return hba->vops->exit(hba);
}

static inline int ufshcd_vops_phy_initialization(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->phy_initialization)
		return hba->vops->phy_initialization(hba);
	return 0;
}

static inline u32 ufshcd_vops_get_ufs_hci_version(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->get_ufs_hci_version)
		return hba->vops->get_ufs_hci_version(hba);

	return ufshcd_readl(hba, REG_UFS_VERSION);
}

static inline int ufshcd_vops_clk_scale_notify(struct ufs_hba *hba,
			bool up, enum ufs_notify_change_status status)
{
	if (hba->vops && hba->vops->clk_scale_notify)
		return hba->vops->clk_scale_notify(hba, up, status);
	return 0;
}

static inline void ufshcd_vops_event_notify(struct ufs_hba *hba,
					    enum ufs_event_type evt,
					    void *data)
{
	if (hba->vops && hba->vops->event_notify)
		hba->vops->event_notify(hba, evt, data);
}

static inline int ufshcd_vops_setup_clocks(struct ufs_hba *hba, bool on,
					enum ufs_notify_change_status status)
{
	if (hba->vops && hba->vops->setup_clocks)
		return hba->vops->setup_clocks(hba, on, status);
	return 0;
}

static inline int ufshcd_vops_hce_enable_notify(struct ufs_hba *hba,
						enum ufs_notify_change_status status)
{
	if (hba->vops && hba->vops->hce_enable_notify)
		return hba->vops->hce_enable_notify(hba, status);

	return 0;
}
static inline int ufshcd_vops_link_startup_notify(struct ufs_hba *hba,
						enum ufs_notify_change_status status)
{
	if (hba->vops && hba->vops->link_startup_notify)
		return hba->vops->link_startup_notify(hba, status);

	return 0;
}

static inline int ufshcd_vops_pwr_change_notify(struct ufs_hba *hba,
				  enum ufs_notify_change_status status,
				  struct ufs_pa_layer_attr *dev_max_params,
				  struct ufs_pa_layer_attr *dev_req_params)
{
	if (hba->vops && hba->vops->pwr_change_notify)
		return hba->vops->pwr_change_notify(hba, status,
					dev_max_params, dev_req_params);

	return -ENOTSUPP;
}

static inline void ufshcd_vops_setup_task_mgmt(struct ufs_hba *hba,
					int tag, u8 tm_function)
{
	if (hba->vops && hba->vops->setup_task_mgmt)
		return hba->vops->setup_task_mgmt(hba, tag, tm_function);
}

static inline void ufshcd_vops_hibern8_notify(struct ufs_hba *hba,
					enum uic_cmd_dme cmd,
					enum ufs_notify_change_status status)
{
	if (hba->vops && hba->vops->hibern8_notify)
		return hba->vops->hibern8_notify(hba, cmd, status);
}

static inline int ufshcd_vops_apply_dev_quirks(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->apply_dev_quirks)
		return hba->vops->apply_dev_quirks(hba);
	return 0;
}

static inline void ufshcd_vops_fixup_dev_quirks(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->fixup_dev_quirks)
		hba->vops->fixup_dev_quirks(hba);
}

static inline int ufshcd_vops_suspend(struct ufs_hba *hba, enum ufs_pm_op op,
				enum ufs_notify_change_status status)
{
	if (hba->vops && hba->vops->suspend)
		return hba->vops->suspend(hba, op, status);

	return 0;
}

static inline int ufshcd_vops_resume(struct ufs_hba *hba, enum ufs_pm_op op)
{
	if (hba->vops && hba->vops->resume)
		return hba->vops->resume(hba, op);

	return 0;
}

static inline void ufshcd_vops_dbg_register_dump(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->dbg_register_dump)
		hba->vops->dbg_register_dump(hba);
}

static inline int ufshcd_vops_device_reset(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->device_reset)
		return hba->vops->device_reset(hba);

	return 0;
}

static inline void ufshcd_vops_config_scaling_param(struct ufs_hba *hba,
		struct devfreq_dev_profile *profile,
		struct devfreq_simple_ondemand_data *data)
{
	if (hba->vops && hba->vops->config_scaling_param)
		hba->vops->config_scaling_param(hba, profile, data);
}

static inline int ufshcd_vops_program_key(struct ufs_hba *hba,
				const union ufs_crypto_cfg_entry *cfg, int slot)
{
	if (hba->vops && hba->vops->program_key)
		return hba->vops->program_key(hba, cfg, slot);

	return 0;
}

static inline void ufshcd_vops_reinit_notify(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->reinit_notify)
		hba->vops->reinit_notify(hba);
}

static inline int ufshcd_vops_mcq_config_resource(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->mcq_config_resource)
		return hba->vops->mcq_config_resource(hba);

	return 0;
}

static inline int ufshcd_vops_get_hba_mac(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->get_hba_mac)
		return hba->vops->get_hba_mac(hba);

	return 0;
}

static inline int ufshcd_vops_op_runtime_config(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->op_runtime_config)
		return hba->vops->op_runtime_config(hba);

	return 0;
}

static inline int ufshcd_vops_get_outstanding_cqs(struct ufs_hba *hba,
						 unsigned long *ocqs)
{
	if (hba->vops && hba->vops->get_outstanding_cqs)
		return hba->vops->get_outstanding_cqs(hba, ocqs);

	return 0;
}

static inline int ufshcd_mcq_vops_config_esi(struct ufs_hba *hba)
{
	if (hba->vops && hba->vops->config_esi)
		return hba->vops->config_esi(hba);

	return -EOPNOTSUPP;
}

static inline int ufshcd_vops_setup_regulators(struct ufs_hba *hba, bool status)
{
	if (hba->vops && hba->vops->setup_regulators)
		return hba->vops->setup_regulators(hba, status);

	return 0;
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

static inline unsigned int ufshcd_get_ucd_size(struct ufs_hba *hba)
{
	return sizeof(struct utp_transfer_cmd_desc) + SG_ALL * hba->sg_entry_size;
}

static inline bool ufshcd_is_intr_aggr_allowed(struct ufs_hba *hba)
{
	return (hba->caps & UFSHCD_CAP_INTR_AGGR) &&
	       !(hba->quirks & UFSHCD_QUIRK_BROKEN_INTR_AGGR);
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

static inline bool ufshcd_can_autobkops_during_suspend(struct ufs_hba *hba)
{
	return hba->caps & UFSHCD_CAP_AUTO_BKOPS_SUSPEND;
}

static inline bool ufshcd_is_ufs_dev_deepsleep(struct ufs_hba *hba)
{
	return hba->curr_dev_pwr_mode == UFS_DEEPSLEEP_PWR_MODE;
}

static inline void ufshcd_set_sg_entry_size(struct ufs_hba *hba, size_t size)
{
	hba->sg_entry_size = size;
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

#define ufshcd_dme_peer_get(hba, attr_sel, mib_val) \
	ufshcd_dme_get_attr(hba, attr_sel, mib_val, DME_PEER)

#define ufshcd_dme_set(hba, attr_sel, mib_val) \
	ufshcd_dme_set_attr(hba, attr_sel, ATTR_SET_NOR, mib_val, DME_LOCAL)

#define ufshcd_dme_peer_set(hba, attr_sel, mib_val) \
	ufshcd_dme_set_attr(hba, attr_sel, ATTR_SET_NOR, mib_val, DME_PEER)

static inline bool ufshcd_can_hibern8_during_gating(struct ufs_hba *hba)
{
	return hba->caps & UFSHCD_CAP_HIBERN8_WITH_CLK_GATING;
}

static inline bool ufshcd_is_clkgating_allowed(struct ufs_hba *hba)
{
	return hba->caps & UFSHCD_CAP_CLK_GATING;
}

#define ufshcd_is_link_hibern8(hba) ((hba)->uic_link_state == UIC_LINK_HIBERN8_STATE)
#define ufshcd_set_link_hibern8(hba) ((hba)->uic_link_state = UIC_LINK_HIBERN8_STATE)

#define ufshcd_is_link_broken(hba) ((hba)->uic_link_state == UIC_LINK_BROKEN_STATE)
#define ufshcd_set_link_broken(hba) ((hba)->uic_link_state = UIC_LINK_BROKEN_STATE)
#define ufshcd_set_link_off(hba) ((hba)->uic_link_state = UIC_LINK_OFF_STATE)

int ufshcd_uic_hibern8_enter(struct ufs_hba *hba);
int ufshcd_uic_hibern8_exit(struct ufs_hba *hba);

void ufshcd_auto_hibern8_enable(struct ufs_hba *hba);

u32 ufshcd_get_local_unipro_ver(struct ufs_hba *hba);
int ufshcd_disable_host_tx_lcc(struct ufs_hba *hba);
int ufshcd_dme_configure_adapt(struct ufs_hba *hba, int agreed_gear, int adapt_val);

static inline bool ufshcd_is_rpm_autosuspend_allowed(struct ufs_hba *hba)
{
	return hba->caps & UFSHCD_CAP_RPM_AUTOSUSPEND;
}

static inline bool ufshcd_is_auto_hibern8_enabled(struct ufs_hba *hba)
{
	return ufshcd_is_auto_hibern8_supported(hba) && hba->ahit;
}

#define ufshcd_is_ufs_dev_active(hba) \
	((hba)->curr_dev_pwr_mode == UFS_ACTIVE_PWR_MODE)

#define ufshcd_is_ufs_dev_poweroff(hba) \
	((hba)->curr_dev_pwr_mode == UFS_POWERDOWN_PWR_MODE)

#define ufshcd_is_link_active(hba) \
	((hba)->uic_link_state == UIC_LINK_ACTIVE_STATE)

#define ufshcd_is_link_off(hba) \
	((hba)->uic_link_state == UIC_LINK_OFF_STATE)

static inline bool ufshcd_can_aggressive_pc(struct ufs_hba *hba)
{
	return !!(ufshcd_is_link_hibern8(hba) &&
		  (hba->caps & UFSHCD_CAP_AGGR_POWER_COLLAPSE));
}

int ufshcd_alloc_host(struct device *dev, struct ufs_hba **hba_handle);
void ufshcd_dealloc_host(struct ufs_hba *hba);
int ufshcd_init(struct ufs_hba *hba, void __iomem *mmio_base, unsigned int irq);
int ufshcd_shutdown(struct ufs_hba *hba);
void ufshcd_remove(struct ufs_hba *hba);

void ufshcd_update_evt_hist(struct ufs_hba *hba, u32 id, u32 val);

int ufshcd_dump_regs(struct ufs_hba *hba, size_t offset, size_t len,
		     const char *prefix);

void ufshcd_fixup_dev_quirks(struct ufs_hba *hba,
			    const struct ufs_dev_quirk *fixups);

int ufshcd_system_suspend(struct device *dev);
int ufshcd_system_resume(struct device *dev);
int ufshcd_system_freeze(struct device *dev);
int ufshcd_system_restore(struct device *dev);
int ufshcd_system_thaw(struct device *dev);
int __ufshcd_suspend_prepare(struct device *dev, bool rpm_ok_for_spm);
int ufshcd_suspend_prepare(struct device *dev);
void ufshcd_resume_complete(struct device *dev);
int ufshcd_runtime_suspend(struct device *dev);
int ufshcd_runtime_resume(struct device *dev);

void ufshcd_mcq_write_cqis(struct ufs_hba *hba, u32 val, int i);
unsigned long ufshcd_mcq_poll_cqe_lock(struct ufs_hba *hba,
				       struct ufs_hw_queue *hwq);
void ufshcd_mcq_config_esi(struct ufs_hba *hba, struct msi_msg *msg);
void ufshcd_mcq_enable_esi(struct ufs_hba *hba);

int ufshcd_advanced_rpmb_req_handler(struct ufs_hba *hba, struct utp_upiu_req *req_upiu,
			 struct utp_upiu_req *rsp_upiu, struct ufs_ehs *req_ehs,
			 struct ufs_ehs *rsp_ehs, int sg_cnt, struct scatterlist *sg_list,
			 enum dma_data_direction dir);

#endif /* _UFSHCD_H */
