#ifndef _UFS_QCOM_H_
#define _UFS_QCOM_H_

#include <linux/reset-controller.h>
#include <linux/reset.h>
#include <ufs/ufshcd.h>
#include <ufs/unipro.h>
#include "../mi_ufs/mi-ufs.h"

#define MAX_UFS_QCOM_HOSTS	2
#define MAX_QS 16
#define MAX_LUNS 32

#define MPHY_TX_FSM_STATE       0x41
#define TX_FSM_HIBERN8          0x1
#define HBRN8_POLL_TOUT_MS      100
#define DEFAULT_CLK_RATE_HZ     1000000

#define UFS_QCOM_LIMIT_HS_RATE		PA_HS_MODE_B

/* QCOM UFS host controller vendor specific registers */
enum {
	REG_UFS_SYS1CLK_1US                 = 0xC0,
	REG_UFS_TX_SYMBOL_CLK_NS_US         = 0xC4,
	REG_UFS_LOCAL_PORT_ID_REG           = 0xC8,
	REG_UFS_PA_ERR_CODE                 = 0xCC,
	REG_UFS_RETRY_TIMER_REG             = 0xD0,
	REG_UFS_CFG0                        = 0xD4,
	REG_UFS_PA_LINK_STARTUP_TIMER       = 0xD8,
	REG_UFS_CFG1                        = 0xDC,
	REG_UFS_CFG2                        = 0xE0,
	REG_UFS_HW_VERSION                  = 0xE4,

	UFS_TEST_BUS				= 0xE8,
	UFS_TEST_BUS_CTRL_0			= 0xEC,
	UFS_TEST_BUS_CTRL_1			= 0xF0,
	UFS_TEST_BUS_CTRL_2			= 0xF4,
	UFS_UNIPRO_CFG				= 0xF8,

	/*
	 * QCOM UFS host controller vendor specific registers
	 * added in HW Version 3.0.0
	 */
	UFS_AH8_CFG				= 0xFC,
};

/* QCOM UFS host controller vendor specific debug registers */
enum {
	UFS_DBG_RD_REG_UAWM			= 0x100,
	UFS_DBG_RD_REG_UARM			= 0x200,
	UFS_DBG_RD_REG_TXUC			= 0x300,
	UFS_DBG_RD_REG_RXUC			= 0x400,
	UFS_DBG_RD_REG_DFC			= 0x500,
	UFS_DBG_RD_REG_TRLUT			= 0x600,
	UFS_DBG_RD_REG_TMRLUT			= 0x700,
	UFS_UFS_DBG_RD_REG_OCSC			= 0x800,

	UFS_UFS_DBG_RD_DESC_RAM			= 0x1500,
	UFS_UFS_DBG_RD_PRDT_RAM			= 0x1700,
	UFS_UFS_DBG_RD_RESP_RAM			= 0x1800,
	UFS_UFS_DBG_RD_EDTL_RAM			= 0x1900,
};

#define QUNIPRO_G4_SEL		BIT(0)
#define QUNIPRO_SEL		0x1

/* bit definitions for REG_UFS_CFG1 register */
#define UTP_DBG_RAMS_EN		0x20000
#define TEST_BUS_EN		BIT(18)
#define TEST_BUS_SEL		GENMASK(22, 19)
#define UFS_REG_TEST_BUS_EN	BIT(30)

/* bit definitions for REG_UFS_CFG2 register */
#define UAWM_HW_CGC_EN		(1 << 0)
#define UARM_HW_CGC_EN		(1 << 1)
#define TXUC_HW_CGC_EN		(1 << 2)
#define RXUC_HW_CGC_EN		(1 << 3)
#define DFC_HW_CGC_EN		(1 << 4)
#define TRLUT_HW_CGC_EN		(1 << 5)
#define TMRLUT_HW_CGC_EN	(1 << 6)
#define OCSC_HW_CGC_EN		(1 << 7)

#define REG_UFS_CFG2_CGC_EN_ALL (UAWM_HW_CGC_EN | UARM_HW_CGC_EN |\
				 TXUC_HW_CGC_EN | RXUC_HW_CGC_EN |\
				 DFC_HW_CGC_EN | TRLUT_HW_CGC_EN |\
				 TMRLUT_HW_CGC_EN | OCSC_HW_CGC_EN)

/* bit definition for UFS_UFS_TEST_BUS_CTRL_n */
#define TEST_BUS_SUB_SEL_MASK	0x1F

/* QUniPro Vendor specific attributes */
#define PA_VS_CONFIG_REG1	0x9000
#define DME_VS_CORE_CLK_CTRL	0xD002

/* bit and mask definitions for DME_VS_CORE_CLK_CTRL attribute */
#define DME_VS_CORE_CLK_CTRL_CORE_CLK_DIV_EN_BIT		BIT(8)
#define DME_VS_CORE_CLK_CTRL_MAX_CORE_CLK_1US_CYCLES_MASK	0xFF

#define UFS_CNTLR_2_x_x_VEN_REGS_OFFSET(x)	(0x000 + x)
#define UFS_CNTLR_3_x_x_VEN_REGS_OFFSET(x)	(0x400 + x)

/* bit offset */
enum {
	OFFSET_UFS_PHY_SOFT_RESET           = 1,
	OFFSET_CLK_NS_REG                   = 10,
};

/* bit masks */
enum {
	MASK_UFS_PHY_SOFT_RESET             = 0x2,
	MASK_CLK_NS_REG                     = 0xFFFC00,
};

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
	u8 select_major;
	u8 select_minor;
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

static inline bool ufs_qcom_cap_qunipro(struct ufs_qcom_host *host)
{
	if (host->caps & UFS_QCOM_CAP_QUNIPRO)
		return true;
	else
		return false;
}

static inline void
ufs_qcom_get_controller_revision(struct ufs_hba *hba,
				 u8 *major, u16 *minor, u16 *step)
{
	u32 ver = ufshcd_readl(hba, REG_UFS_HW_VERSION);

	*major = (ver & 0xF0000000) >> 28;
	*minor = (ver & 0x0FFF0000) >> 16;
	*step = (ver & 0x0000FFFF);
}

static inline void ufs_qcom_assert_reset(struct ufs_hba *hba)
{
	ufshcd_rmwl(hba, MASK_UFS_PHY_SOFT_RESET,
			1 << OFFSET_UFS_PHY_SOFT_RESET, REG_UFS_CFG1);
	mb();
}

static inline void ufs_qcom_deassert_reset(struct ufs_hba *hba)
{
	ufshcd_rmwl(hba, MASK_UFS_PHY_SOFT_RESET,
			0 << OFFSET_UFS_PHY_SOFT_RESET, REG_UFS_CFG1);
	mb();
}

/* ufs-qcom-ice.c */
#ifdef CONFIG_SCSI_UFS_CRYPTO
int ufs_qcom_ice_init(struct ufs_qcom_host *host);
int ufs_qcom_ice_enable(struct ufs_qcom_host *host);
int ufs_qcom_ice_resume(struct ufs_qcom_host *host);
int ufs_qcom_ice_program_key(struct ufs_hba *hba,
			     const union ufs_crypto_cfg_entry *cfg, int slot);
void ufs_qcom_ice_disable(struct ufs_qcom_host *host);
#else
static inline int ufs_qcom_ice_init(struct ufs_qcom_host *host) { return 0; }
static inline int ufs_qcom_ice_enable(struct ufs_qcom_host *host) { return 0; }
static inline int ufs_qcom_ice_resume(struct ufs_qcom_host *host) { return 0; }
static inline void ufs_qcom_ice_disable(struct ufs_qcom_host *host) { }
#define ufs_qcom_ice_program_key NULL
#endif /* CONFIG_SCSI_UFS_CRYPTO */

#endif /* _UFS_QCOM_H_ */
