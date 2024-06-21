/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */

#ifndef _FPGA_EXCEPTION_
#define _FPGA_EXCEPTION_

#include <linux/version.h>

#define SLAVER_ERROR     "slave_error"

struct fpga_exception_data {
	void  *private_data;
	unsigned int exception_upload_count;
};

typedef enum {
	EXCEP_DEFAULT = 0,
	EXCEP_SLAVER_ERR,
} fpga_excep_type;

int fpga_exception_report(void *fpga_exception_data, fpga_excep_type excep_tpye, void *summary, unsigned int summary_size);

#endif /*_FPGA_EXCEPTION_*/
