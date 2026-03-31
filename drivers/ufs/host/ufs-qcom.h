#ifndef _UFS_QCOM_H_
#define _UFS_QCOM_H_

#include <linux/reset-controller.h>
#include <linux/reset.h>
#include <ufs/ufshcd.h>
#include "../mi_ufs/mi-ufs.h"

#define MAX_QS 16
#define MAX_LUNS 32

#define UFS_CNTLR_2_x_x_VEN_REGS_OFFSET(x)	(0x000 + x)
#define UFS_CNTLR_3_x_x_VEN_REGS_OFFSET(x)	(0x400 + x)

/* Host controller hardware version: major.minor.step */
struct ufs_hw_version {
	u16 step;
	u16 minor;
	u8 major;
};

/* Host UIC error code PHY adapter layer */
#ifndef _UFS_EC_PA_DEFINED
#define _UFS_EC_PA_DEFINED
enum ufshcd_ec_pa {
	UFS_EC_PA_LANE_0,
	UFS_EC_PA_LANE_1,
	UFS_EC_PA_LANE_2,
	UFS_EC_PA_LANE_3,
	UFS_EC_PA_LINE_RESET,
	UFS_EC_PA_MAX,
};
#endif

/* Host UIC error code data link layer */
#ifndef _UFS_EC_DL_DEFINED
#define _UFS_EC_DL_DEFINED
enum ufshcd_ec_dl {
	UFS_EC_DL_NAC_RECEIVED,
	UFS_EC_DL_TCx_REPLAY_TIMER_EXPIRED,
	UFS_EC_DL_AFCx_REQUEST_TIMER_EXPIRED,
	UFS_EC_DL_FCx_PROTECT_TIMER_EXPIRED,
	UFS_EC_DL_CRC_ERROR,
	UFS_EC_DL_RX_BUFFER_OVERFLOW,
	UFS_EC_DL_MAX_FRAME_LENGTH_EXCEEDED,
	UFS_EC_DL_WRONG_SEQUENCE_NUMBER,
	UFS_EC_DL_AFC_FRAME_SYNTAX_ERROR,
	UFS_EC_DL_NAC_FRAME_SYNTAX_ERROR,
	UFS_EC_DL_EOF_SYNTAX_ERROR,
	UFS_EC_DL_FRAME_SYNTAX_ERROR,
	UFS_EC_DL_BAD_CTRL_SYMBOL_TYPE,
	UFS_EC_DL_PA_INIT_ERROR,
	UFS_EC_DL_PA_ERROR_IND_RECEIVED,
	UFS_EC_DL_MAX,
};
#endif

struct ufs_uic_stats {
	u32 pa_err_cnt_total;
	u32 pa_err_cnt[UFS_EC_PA_MAX];
	u32 dl_err_cnt_total;
	u32 dl_err_cnt[UFS_EC_DL_MAX];
	u32 dme_err_cnt;

	u32 last_intr_status;
	u64 last_intr_ts;
	u32 hibern8_exit_cnt;
	u64 last_hibern8_exit_tstamp;
	struct ufs_event_hist event[UFS_EVT_CNT];
};

struct ufs_qcom_testbus {
	u8 select_id;
	u8 config;
};

struct ufs_qcom_host {
	/*
	 * Set this capability if host controller supports the QUniPro mode
	 * and if driver wants the Host controller to operate in QUniPro mode.
	 * Note: By default this capability will be kept enabled if host
	 * controller supports the QUniPro mode.
	 */
	#define UFS_QCOM_CAP_QUNIPRO	0x1

	/*
	 * Set this capability if host controller can retain the secure
	 * configuration even after UFS controller core power collapse.
	 */
	#define UFS_QCOM_CAP_RETAIN_SEC_CFG_AFTER_PWR_COLLAPSE	0x2
	u32 caps;

	struct phy *generic_phy;
	struct ufs_hba *hba;
	struct ufs_uic_stats ufs_stats;
	struct ufs_pa_layer_attr dev_req_params;
	struct clk *rx_l0_sync_clk;
	struct clk *tx_l0_sync_clk;
	struct clk *rx_l1_sync_clk;
	struct clk *tx_l1_sync_clk;
	bool is_lane_clks_enabled;

	void __iomem *dev_ref_clk_ctrl_mmio;
	bool is_dev_ref_clk_enabled;
	struct ufs_hw_version hw_ver;
#ifdef CONFIG_SCSI_UFS_CRYPTO
	void __iomem *ice_mmio;
#endif

	u32 dev_ref_clk_en_mask;

	struct ufs_qcom_testbus testbus;

	/* Reset control of HCI */
	struct reset_control *core_reset;
	struct reset_controller_dev rcdev;

	struct gpio_desc *device_reset;

	int esi_base;
	bool esi_enabled;

	struct ufscld_dev *cld;
};

static inline u32
ufs_qcom_get_debug_reg_offset(struct ufs_qcom_host *host, u32 reg)
{
	if (host->hw_ver.major <= 0x02)
		return UFS_CNTLR_2_x_x_VEN_REGS_OFFSET(reg);

	return UFS_CNTLR_3_x_x_VEN_REGS_OFFSET(reg);
}

#define ufs_qcom_is_link_off(hba) ufshcd_is_link_off(hba)
#define ufs_qcom_is_link_active(hba) ufshcd_is_link_active(hba)
#define ufs_qcom_is_link_hibern8(hba) ufshcd_is_link_hibern8(hba)

int ufs_qcom_testbus_config(struct ufs_qcom_host *host);

#endif /* _UFS_QCOM_H_ */
