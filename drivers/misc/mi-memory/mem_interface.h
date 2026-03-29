#ifndef __MEM_INTERFACE_H__
#define __MEM_INTERFACE_H__

#include <scsi/scsi_device.h>
#include <scsi/scsi_common.h>

#include "../../ufs/mi_ufs/mi-ufshcd.h"

#define SD_ASCII_STD true
#define SD_RAW false

int ufs_get_string_desc(struct ufs_hba *hba, void* buf, int size, enum device_desc_param pname, bool ascii_std);

int ufs_read_desc_param(struct ufs_hba *hba, enum desc_idn desc_id, u8 desc_index, u8 param_offset, void* buf, u8 param_size);



#endif
