# SPDX-License-Identifier: GPL-2.0

ccflags-y += -I$(srctree)/drivers/ufs/core
ccflags-y += -I$(srctree)/drivers/ufs/mi_ufs

obj-$(CONFIG_SCSI_UFS_DWC_TC_PCI) += tc-dwc-g210-pci.o ufshcd-dwc.o tc-dwc-g210.o
obj-$(CONFIG_SCSI_UFS_DWC_TC_PLATFORM) += tc-dwc-g210-pltfrm.o ufshcd-dwc.o tc-dwc-g210.o
obj-$(CONFIG_SCSI_UFS_CDNS_PLATFORM) += cdns-pltfrm.o
obj-$(CONFIG_SCSI_UFS_QCOM) += ufs_qcom.o
ufs_qcom-y += ufs-qcom.o ../mi_ufs/mi-ufshcd-pltfrm.o ../mi_ufs/mi-ufshcd.o ../mi_ufs/mi-ufs-sysfs.o ../mi_ufs/ufs_ffu.o
ufs_qcom-$(CONFIG_SCSI_UFS_CRYPTO) += ufs-qcom-ice.o
obj-$(CONFIG_SCSI_UFS_EXYNOS) += ufs-exynos.o
obj-$(CONFIG_SCSI_UFSHCD_PCI) += ufshcd-pci.o
obj-$(CONFIG_SCSI_UFSHCD_PLATFORM) += ufshcd-pltfrm.o
obj-$(CONFIG_SCSI_UFS_HISI) += ufs-hisi.o
obj-$(CONFIG_SCSI_UFS_MEDIATEK) += ufs-mediatek.o
obj-$(CONFIG_SCSI_UFS_RENESAS) += ufs-renesas.o
obj-$(CONFIG_SCSI_UFS_SPRD) += ufs-sprd.o
obj-$(CONFIG_SCSI_UFS_TI_J721E) += ti-j721e-ufs.o
ufs_qcom-$(CONFIG_DEBUG_FS)        += ../mi_ufs/mi-ufs-debugfs.o
ufs_qcom-$(CONFIG_SCSI_UFS_BSG)        += ../mi_ufs/mi_ufs_bsg.o
ufs_qcom-$(CONFIG_SCSI_UFS_CRYPTO) += ../mi_ufs/mi-ufshcd-crypto.o
ufs_qcom-$(CONFIG_UFSGKI)          += ../mi_ufs/mi_ufs_common_ops.o
ufs_qcom-$(CONFIG_CLD)             += ../mi_ufs/cld/mi_cld_sysfs.o ../mi_ufs/cld/mi_cld.o \
                                  ../mi_ufs/cld/skhynix_cld.o ../mi_ufs/cld/samsung_cld.o \
                                  ../mi_ufs/cld/kioxia_cld.o \
                                  ../mi_ufs/cld/wdc_cld.o \
                                  ../mi_ufs/cld/micron_cld.o
