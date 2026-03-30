/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _UFSHCD_ADD_INFO_H_
#define _UFSHCD_ADD_INFO_H_

/*
 * Compared to the upstream equivalent, @hpb_dev has been moved from struct
 * ufs_hba into struct ufs_hba_add_info to satisfy the Android ABI checks.
 * Do NOT use this data structure in any out-of-tree driver since it is not
 * covered by the GKI.
 */
struct ufshpb_dev_info {
	int num_lu;
	int rgn_size;
	int srgn_size;
	atomic_t slave_conf_cnt;
	bool hpb_disabled;
	u8 max_hpb_single_cmd;
	bool is_legacy;
	u8 control_mode;
};

struct ufs_hba_add_info {
	struct ufs_hba hba;
	u32 reserved_slot;
	struct request **tmf_rqs;
	unsigned char desc_size[QUERY_DESC_IDN_MAX];
#ifdef CONFIG_SCSI_UFS_HPB
	struct ufshpb_dev_info hpb_dev;
#endif
};

static inline struct ufs_hba_add_info *ufs_hba_add_info(struct ufs_hba *hba)
{
	return container_of(hba, struct ufs_hba_add_info, hba);
}

#endif /* _UFSHCD_ADD_INFO_H_ */
