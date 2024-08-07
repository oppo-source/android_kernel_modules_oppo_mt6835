// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2022-2024 Oplus. All rights reserved.
 */
/***************************************************************
** OPLUS_FEATURE_DEVICE_FAULT_INJECT
** File : oplus_device_fault_inject.c
** Description : framework for device fault inject test
** Version : 1.0
******************************************************************/

#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/version.h>

#include "../common/oplus_inject_hook.h"
#include "../common/oplus_inject_proc.h"

/* record the hookhandler whether register */
static int bhook = 0;
#define ARG_0    0
#define ARG_1    1
#define ARG_2    2
#define ARG_3    3
#define ARG_4    4

#ifdef CONFIG_HAPTIC_FEEDBACK_MODULE
int oplus_haptic_track_dev_err(uint32_t track_type, uint32_t reg_addr, uint32_t err_code);
#endif
#define HAPTIC_I2C_READ_TRACK_ERR_FB    0
#define HAPTIC_I2C_WRITE_TRACK_ERR_FB   1


struct aw8692x_testcase {
	int haptics_inject_type;
	char *test_case_name;
	int valid_flag;
};

enum aw8692x_testcase_list {
	AW8692x_I2C_READ_INJECT = 0,
	AW8692x_I2C_WRITE_INJECT,
	AW8692x_SET_RTP_DATA_INJECT,
	AW8692x_PLAY_GO_INJECT,
	AW8692x_OSC_DETECT_INJECT,
#if IS_ENABLED(CONFIG_OPLUS_FEATURE_BSP_DRV_VND_INJECT_TEST)
	AW8692x_SET_GAIN_INJECT,
	AW8692x_SET_BST_VOL_INJECT,
	AW8692x_SET_AUTO_BREAK_MODE_INJECT,
#endif
	AW8692x_INJECT_MAX,
};

static struct aw8692x_testcase g_aw8692x_inject_cases[AW8692x_INJECT_MAX] = {
	{AW8692x_I2C_READ_INJECT, "aw8692x_i2c_read", 0},
	{AW8692x_I2C_WRITE_INJECT, "aw8692x_i2c_write", 0},
	{AW8692x_SET_RTP_DATA_INJECT, "aw8692x_set_rtp_data", 0},
	{AW8692x_PLAY_GO_INJECT, "aw8692x_play_fault", 0},
	{AW8692x_OSC_DETECT_INJECT, "aw8692x_rtp_osc_cali", 0},
#if IS_ENABLED(CONFIG_OPLUS_FEATURE_BSP_DRV_VND_INJECT_TEST)
	{AW8692x_SET_GAIN_INJECT, "aw8692x_set_gain", 0},
	{AW8692x_SET_BST_VOL_INJECT, "aw8692x_set_bst_vol", 0},
	{AW8692x_SET_AUTO_BREAK_MODE_INJECT, "aw8692x_auto_break_mode", 0},
#endif
};

#define SEQ_printf(m, x...)     \
        do {                        \
                if (m)                  \
                seq_printf(m, x);   \
                else                    \
                pr_debug(x);        \
        } while (0)

static int register_haptics_hook(void);

static int oplus_sample_help_show(struct seq_file *m, void *v)
{
	SEQ_printf(m, "=== oplus_sample_fault inject test ===\n");

	/* print current status value */
	SEQ_printf(m, "aw8692x_i2c_read %d \n", g_aw8692x_inject_cases[AW8692x_I2C_READ_INJECT].valid_flag);
	SEQ_printf(m, "aw8692x_i2c_write %d \n", g_aw8692x_inject_cases[AW8692x_I2C_WRITE_INJECT].valid_flag);
	SEQ_printf(m, "aw8692x_set_rtp_data %d \n", g_aw8692x_inject_cases[AW8692x_SET_RTP_DATA_INJECT].valid_flag);
	SEQ_printf(m, "aw8692x_play_fault %d \n", g_aw8692x_inject_cases[AW8692x_PLAY_GO_INJECT].valid_flag);
	SEQ_printf(m, "aw8692x_rtp_osc_cali %d \n", g_aw8692x_inject_cases[AW8692x_OSC_DETECT_INJECT].valid_flag);
#if IS_ENABLED(CONFIG_OPLUS_FEATURE_BSP_DRV_VND_INJECT_TEST)
	SEQ_printf(m, "aw8692x_aw8692x_set_gain %d \n", g_aw8692x_inject_cases[AW8692x_SET_GAIN_INJECT].valid_flag);
	SEQ_printf(m, "aw8692x_set_bst_vol %d \n", g_aw8692x_inject_cases[AW8692x_SET_BST_VOL_INJECT].valid_flag);
	SEQ_printf(m, "aw8692x_auto_break_mode %d \n", g_aw8692x_inject_cases[AW8692x_SET_AUTO_BREAK_MODE_INJECT].valid_flag);
#endif
	return 0;
}

static int oplus_sample_fault_inject_open(struct inode *inode, struct file *file)
{
	if (!inode)
		return -1;
	return single_open(file, oplus_sample_help_show, inode->i_private);
}

static ssize_t oplus_sample_fault_inject_write(struct file *file,
		const char __user *buf, size_t count, loff_t *off)
{
	char buffer[64] = {0};
	int ret = 0;
	char testcase[64] = {0};
	int nval = 0;
	int i = 0;

	if (count > 64) {
		count = 64;
	}

	if (copy_from_user(buffer, buf, count)) {
		pr_err("%s: read proc input error.\n", __func__);
		return count;
	}

	ret = sscanf(buffer, "%s %d", testcase, &nval);

	if (ret <= 0) {
		pr_err("%s input param error\n", __func__);
		return count;
	}

	if (!bhook) {
		/*
		 * no need register hook when init, may caused performance issue.
		 * suggest when inject test trigger do the hook
		 */
		ret = register_haptics_hook();
		if (!ret) {
			bhook = 1;
		}
	}

	for(i = 0; i < AW8692x_INJECT_MAX; i++) {
		if (strcmp(testcase, g_aw8692x_inject_cases[i].test_case_name) == 0) {
			g_aw8692x_inject_cases[i].valid_flag = nval;
			break;
		}
	}

	return count;
}

static struct proc_ops oplus_aw8692x_inject_ops = {
	.proc_open = oplus_sample_fault_inject_open,
	.proc_read = seq_read,
	.proc_write = oplus_sample_fault_inject_write,
	.proc_release = single_release,
};

int oplus_hook_handler_entry(i2c_r_bytes)(struct oplus_hook_instance *ri, struct pt_regs *regs)
{
	uint8_t addr = 0;
	struct OplusHookRegs *rd = (struct OplusHookRegs *)ri->data;

	if (g_aw8692x_inject_cases[AW8692x_I2C_READ_INJECT].valid_flag == 1) {
		addr = *(uint8_t *)(&(rd->args[1]));
#ifdef CONFIG_HAPTIC_FEEDBACK_MODULE
		oplus_haptic_track_dev_err(AW8692x_I2C_READ_INJECT, addr, -1);
#endif
		oplus_hook_setarg(regs, ARG_3, 0);
	}
	return 0;
}

int oplus_hook_handler_entry(i2c_w_bytes)(struct oplus_hook_instance *ri, struct pt_regs *regs)
{
	uint8_t addr = 0;
	struct OplusHookRegs *rd = (struct OplusHookRegs *)ri->data;

	if (g_aw8692x_inject_cases[AW8692x_I2C_WRITE_INJECT].valid_flag == 1) {
		addr = *(uint8_t *)(&(rd->args[1]));
#ifdef CONFIG_HAPTIC_FEEDBACK_MODULE
		oplus_haptic_track_dev_err(AW8692x_I2C_WRITE_INJECT, addr, -1);
#endif
		oplus_hook_setarg(regs, ARG_3, 0);
	}

	return 0;
}

/* inject aw8692x_set_rtp_data err */
int oplus_hook_handler_entry(aw8692x_set_rtp_data)(struct oplus_hook_instance *ri, struct pt_regs *regs)
{
	if (g_aw8692x_inject_cases[AW8692x_SET_RTP_DATA_INJECT].valid_flag != 0) {
		oplus_hook_setarg(regs, ARG_2, 0);
	}

	return 0;
}


/* inject aw8692x_play_go err */
int oplus_hook_handler_entry(aw8692x_play_go)(struct oplus_hook_instance *ri, struct pt_regs *regs)
{
	if (g_aw8692x_inject_cases[AW8692x_PLAY_GO_INJECT].valid_flag != 0) {
		oplus_hook_setarg(regs, ARG_1, 0);
	}

	return 0;
}

/* inject rtp_osc_cali err */
int oplus_hook_handler_entry(rtp_osc_cali)(struct oplus_hook_instance *ri, struct pt_regs *regs)
{
	g_aw8692x_inject_cases[AW8692x_PLAY_GO_INJECT].valid_flag =
		g_aw8692x_inject_cases[AW8692x_OSC_DETECT_INJECT].valid_flag;

	return 0;
}

#if IS_ENABLED(CONFIG_OPLUS_FEATURE_BSP_DRV_VND_INJECT_TEST)
int oplus_hook_handler_entry(aw8692x_set_gain)(struct oplus_hook_instance *ri, struct pt_regs *regs)
{
	if (g_aw8692x_inject_cases[AW8692x_SET_GAIN_INJECT].valid_flag != 0) {
		oplus_hook_setarg(regs, ARG_1, g_aw8692x_inject_cases[AW8692x_SET_GAIN_INJECT].valid_flag);
	}
	return 0;
}


int oplus_hook_handler_entry(aw8692x_set_bst_vol)(struct oplus_hook_instance *ri, struct pt_regs *regs)
{
	if (g_aw8692x_inject_cases[AW8692x_SET_BST_VOL_INJECT].valid_flag != 0) {
		oplus_hook_setarg(regs, ARG_1, g_aw8692x_inject_cases[AW8692x_SET_BST_VOL_INJECT].valid_flag);
	}
	return 0;
}


int oplus_hook_handler_entry(aw8692x_auto_break_mode)(struct oplus_hook_instance *ri, struct pt_regs *regs)
{
	if (g_aw8692x_inject_cases[AW8692x_SET_AUTO_BREAK_MODE_INJECT].valid_flag != 0) {
		if (g_aw8692x_inject_cases[AW8692x_SET_AUTO_BREAK_MODE_INJECT].valid_flag == 1) {
			/* set auto break mode true */
			oplus_hook_setarg(regs, ARG_1, true);
		}

		if (g_aw8692x_inject_cases[AW8692x_SET_AUTO_BREAK_MODE_INJECT].valid_flag == 2) {
			/* set auto break mode false */
			oplus_hook_setarg(regs, ARG_1, false);
		}
	}
	return 0;
}
#endif

/* init the hook struct */
oplus_hook_entry_define(i2c_r_bytes);
oplus_hook_entry_define(i2c_w_bytes);
oplus_hook_entry_define(aw8692x_set_rtp_data);
oplus_hook_entry_define(aw8692x_play_go);
oplus_hook_entry_define(rtp_osc_cali);
#if IS_ENABLED(CONFIG_OPLUS_FEATURE_BSP_DRV_VND_INJECT_TEST)
oplus_hook_entry_define(aw8692x_set_gain);
oplus_hook_entry_define(aw8692x_set_bst_vol);
oplus_hook_entry_define(aw8692x_auto_break_mode);
#endif

static struct oplus_hook *oplus_aw8692x_interfaces[] = {
	&oplus_hook_name(i2c_r_bytes),
	&oplus_hook_name(i2c_w_bytes),
	&oplus_hook_name(aw8692x_set_rtp_data),
	&oplus_hook_name(aw8692x_play_go),
	&oplus_hook_name(rtp_osc_cali),
#if IS_ENABLED(CONFIG_OPLUS_FEATURE_BSP_DRV_VND_INJECT_TEST)
	&oplus_hook_name(aw8692x_set_gain),
	&oplus_hook_name(aw8692x_set_bst_vol),
	&oplus_hook_name(aw8692x_auto_break_mode),
#endif
};

#if IS_ENABLED(CONFIG_OPLUS_FEATURE_BSP_DRV_VND_INJECT_TEST)
uint8_t g_reg_addr;
extern int aw869xx_i2c_r_byte(uint8_t reg_addr, uint8_t *buf);
extern int aw869xx_i2c_w_byte(uint8_t reg_addr, uint8_t *buf);

static int reg_proc_show(struct seq_file *m, void *v)
{
	uint8_t reg_val = 0;

	aw869xx_i2c_r_byte(g_reg_addr, &reg_val);

	SEQ_printf(m, "reg: %x, reg_val: %x \n", g_reg_addr, reg_val);
	return 0;
}

static int reg_proc_open(struct inode *inode, struct file *file)
{
	if (!inode)
		return -1;
	single_open(file, reg_proc_show, inode->i_private);
	return 0;
}

static ssize_t reg_proc_write(struct file *file,
		const char __user *buf, size_t count, loff_t *off)
{
	uint8_t reg_addr = 0;
	uint8_t reg_val = 0;
	int write_flag, tmp1, tmp2 = 0;
	char buffer[64] = {0};
	int ret = 0;

	if (count > 64) {
		count = 64;
	}

	if (copy_from_user(buffer, buf, count)) {
		pr_err("%s: read proc input error.\n", __func__);
		return count;
	}

	ret = sscanf(buffer, "%d %d %d", &tmp1, &tmp2, &write_flag);
	if (ret <= 0) {
		pr_err("%s input param error\n", __func__);
		return count;
	}

	reg_addr = (uint8_t)tmp1;
	reg_val = (uint8_t)tmp2;
	g_reg_addr = reg_addr;

	if ((reg_addr >= 0) && (reg_addr <= 0x79) && (write_flag == 1)) {
		(void)aw869xx_i2c_w_byte(reg_addr, &reg_val);
	}

	return count;
}
#endif

static int register_haptics_hook(void)
{
	int ret = 0;

	ret = register_oplus_hooks(oplus_aw8692x_interfaces, ARRAY_SIZE(oplus_aw8692x_interfaces));

	if (ret) {
		pr_err("%s failed %d\n", __func__, ret);
	}

	return ret;
}

#if IS_ENABLED(CONFIG_OPLUS_FEATURE_BSP_DRV_VND_INJECT_TEST)
static struct proc_ops oplus_aw8692x_reg_inject_ops = {
	.proc_open = reg_proc_open,
	.proc_read = seq_read,
	.proc_write = reg_proc_write,
	.proc_release = single_release,
};
#endif

static int oplus_inject_sample_init(void)
{
	oplus_proc_inject_init("aw8629x/capacity", &oplus_aw8692x_inject_ops, NULL);
#if IS_ENABLED(CONFIG_OPLUS_FEATURE_BSP_DRV_VND_INJECT_TEST)
	oplus_proc_inject_init("haptics/reg_inject", &oplus_aw8692x_reg_inject_ops, NULL);
#endif

	return 0;
}

static void oplus_inject_sample_exit(void)
{
	if (bhook) {
		unregister_oplus_hooks(oplus_aw8692x_interfaces, ARRAY_SIZE(oplus_aw8692x_interfaces));
	}
}

module_init(oplus_inject_sample_init);
module_exit(oplus_inject_sample_exit);

MODULE_DESCRIPTION("OPLUS device fault inject driver");
MODULE_LICENSE("GPL v2");
