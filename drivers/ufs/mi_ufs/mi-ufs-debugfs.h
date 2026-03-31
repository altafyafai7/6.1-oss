/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2020 Intel Corporation
 */

#ifndef __UFS_DEBUGFS_H__
#define __UFS_DEBUGFS_H__

struct ufs_hba;

#ifdef CONFIG_DEBUG_FS
void __init mi_ufs_debugfs_init(void);
void __exit mi_ufs_debugfs_exit(void);
void mi_ufs_debugfs_hba_init(struct ufs_hba *hba);
void mi_ufs_debugfs_hba_exit(struct ufs_hba *hba);
#else
static inline void mi_ufs_debugfs_init(void) {}
static inline void mi_ufs_debugfs_exit(void) {}
static inline void mi_ufs_debugfs_hba_init(struct ufs_hba *hba) {}
static inline void mi_ufs_debugfs_hba_exit(struct ufs_hba *hba) {}
#endif

#endif
