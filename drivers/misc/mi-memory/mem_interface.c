
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/pm_runtime.h>
#include <linux/nls.h>
#include <linux/blkdev.h>
#include <linux/completion.h>
#include <asm/unaligned.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_request.h>
#include "mem_interface.h"

/* Query request retries */
#define QUERY_REQ_RETRIES 3

/**
 * struct uc_string_id - unicode string
 *
 * @len: size of this descriptor inclusive
 * @type: descriptor type
 * @uc: unicode string character
 */
struct uc_string_id {
	u8 len;
	u8 type;
	wchar_t uc[];
} __packed;

static inline char ufshcd_remove_non_printable(u8 ch)
{
	return (ch >= 0x20 && ch <= 0x7e) ? ch : ' ';
}

static int ufs_read_string_desc(struct ufs_hba *hba, u8 desc_index, u8 **buf, bool ascii)
{
	struct uc_string_id *uc_str;
	u8 *str;
	int ret;

	if (!buf)
		return -EINVAL;

	uc_str = kzalloc(QUERY_DESC_MAX_SIZE, GFP_KERNEL);
	if (!uc_str)
		return -ENOMEM;

	ret = mi_ufshcd_read_desc_param(hba, QUERY_DESC_IDN_STRING, desc_index, 0,
				     (u8 *)uc_str, QUERY_DESC_MAX_SIZE);
	if (ret < 0) {
		dev_err(hba->dev, "Reading String Desc failed after %d retries. err = %d\n",
			QUERY_REQ_RETRIES, ret);
		str = NULL;
		goto out;
	}

	if (uc_str->len <= QUERY_DESC_HDR_SIZE) {
		dev_dbg(hba->dev, "String Desc is of zero length\n");
		str = NULL;
		ret = 0;
		goto out;
	}

	if (ascii) {
		ssize_t ascii_len;
		int i;
		/* remove header and divide by 2 to move from UTF16 to UTF8 */
		ascii_len = (uc_str->len - QUERY_DESC_HDR_SIZE) / 2 + 1;
		str = kzalloc(ascii_len, GFP_KERNEL);
		if (!str) {
			ret = -ENOMEM;
			goto out;
		}

		/*
		 * the descriptor contains string in UTF16 format
		 * we need to convert to utf-8 so it can be displayed
		 */
		ret = utf16s_to_utf8s(uc_str->uc,
				      uc_str->len - QUERY_DESC_HDR_SIZE,
				      UTF16_BIG_ENDIAN, str, ascii_len);

		/* replace non-printable or non-ASCII characters with spaces */
		for (i = 0; i < ret; i++)
			str[i] = ufshcd_remove_non_printable(str[i]);

		str[ret++] = '\0';

	} else {
		str = kmemdup(uc_str, uc_str->len, GFP_KERNEL);
		if (!str) {
			ret = -ENOMEM;
			goto out;
		}
		ret = uc_str->len;
	}
out:
	*buf = str;
	kfree(uc_str);
	return ret;
}

int ufs_get_string_desc(struct ufs_hba *hba, void* buf, int size, enum device_desc_param pname, bool ascii_std)
{
	u8 index;
	int ret = 0;

	int desc_len = QUERY_DESC_MAX_SIZE;
	u8 *desc_buf;

	desc_buf = kzalloc(QUERY_DESC_MAX_SIZE, GFP_ATOMIC);
	if (!desc_buf)
		return -ENOMEM;
	pm_runtime_get_sync(hba->dev);
	ret = mi_ufshcd_query_descriptor_retry(hba,
		UPIU_QUERY_OPCODE_READ_DESC, QUERY_DESC_IDN_DEVICE,
		0, 0, desc_buf, &desc_len);
	if (ret) {
		ret = -EINVAL;
		goto out;
	}
	index = desc_buf[pname];
	kfree(desc_buf);
	desc_buf = NULL;
	ret = ufs_read_string_desc(hba, index, &desc_buf, ascii_std);
	if (ret < 0)
		goto out;
	memcpy(buf, desc_buf, size);
out:
	pm_runtime_put_sync(hba->dev);
	kfree(desc_buf);
	return ret;
}

int ufs_read_desc_param(struct ufs_hba *hba, enum desc_idn desc_id, u8 desc_index, u8 param_offset, void* buf, u8 param_size)
{
	u8 desc_buf[8] = {0};
	int ret;

	if (param_size > 8)
		return -EINVAL;

	pm_runtime_get_sync(hba->dev);
	ret = mi_ufshcd_read_desc_param(hba, desc_id, desc_index,
				param_offset, desc_buf, param_size);
	pm_runtime_put_sync(hba->dev);

	if (ret)
		return -EINVAL;
	switch (param_size) {
	case 1:
		*(u8*)buf = *desc_buf;
		break;
	case 2:
		*(u16*)buf = get_unaligned_be16(desc_buf);
		break;
	case 4:
		*(u32*)buf =  get_unaligned_be32(desc_buf);
		break;
	case 8:
		*(u64*)buf= get_unaligned_be64(desc_buf);
		break;
	default:
		*(u8*)buf = *desc_buf;
		break;
	}

	return ret;
}


