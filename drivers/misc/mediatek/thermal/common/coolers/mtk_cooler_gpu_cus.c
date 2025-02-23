/*
 * Copyright (C) 2019 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/kobject.h>

#include "mt-plat/mtk_thermal_monitor.h"
#include <ap_thermal_limit.h>
#include <tscpu_settings.h>
#include "mtk_gpufreq.h"


#define mtk_cooler_gpu_dprintk(fmt, args...)	\
	pr_notice("thermal/cooler/gpu " fmt, ##args)

static struct thermal_cooling_device
*cl_gpu_dev = { 0 };
static unsigned int g_cl_id;
static unsigned int g_gpu_level;

static int mtk_cl_gpu_get_max_state
(struct thermal_cooling_device *cdev, unsigned long *state)
{
	*state = Num_of_GPU_OPP - gpu_max_opp - 1;
  	/*Due to stage loading issues,
         thermal device cannot receive GPU data during update,
        resulting in the kansan version exceeding the bounds.
        Setting a maximum value to prevent exceeding the bounds only for mt6833p*/
  	/* N17 code for HQ-319735 by wangshuai at 2023/8/22 start */
	*state = 44;
  	/* N17 code for HQ-319735 by wangshuai at 2023/8/22 end */
	return 0;
}

static int mtk_cl_gpu_get_cur_state
(struct thermal_cooling_device *cdev, unsigned long *state)
{
	*state = g_gpu_level;
	return 0;
}

static int mtk_cl_gpu_set_cur_state
(struct thermal_cooling_device *cdev, unsigned long state)
{
	if (mtk_gpu_power != NULL && state >= 0 && state < Num_of_GPU_OPP - gpu_max_opp) {
		g_gpu_level = state;
		if (g_gpu_level == 0)
			mtk_cooler_gpu_dprintk("%d\n", g_gpu_level);
		else {
			mtk_cooler_gpu_dprintk("%d %d %d\n", g_gpu_level,
				mtk_gpu_power[g_gpu_level + gpu_max_opp].gpufreq_khz,
				mtk_gpu_power[g_gpu_level + gpu_max_opp].gpufreq_power);
		}

		apthermolmt_set_general_gpu_power_limit((g_gpu_level == 0) ? 0 :
			mtk_gpu_power[g_gpu_level + gpu_max_opp].gpufreq_power);
	} else {
		mtk_cooler_gpu_dprintk("%s: mtk_gpu_power_table=%p, state=%lu, GPU_OPP=%d\n",
			__func__, mtk_gpu_power, state, Num_of_GPU_OPP - gpu_max_opp);
	}

	return 0;
}

/* bind fan callbacks to fan device */
static struct thermal_cooling_device_ops mtk_cl_gpu_ops = {
	.get_max_state = mtk_cl_gpu_get_max_state,
	.get_cur_state = mtk_cl_gpu_get_cur_state,
	.set_cur_state = mtk_cl_gpu_set_cur_state,
};

static int mtk_cooler_gpu_register_ltf(void)
{
	mtk_cooler_gpu_dprintk("%s\n", __func__);

	g_cl_id = 0;
	cl_gpu_dev = mtk_thermal_cooling_device_register
		("mtk-cl-gpu", (void *)&g_cl_id,
		 &mtk_cl_gpu_ops);

	return 0;
}

static void mtk_cooler_gpu_unregister_ltf(void)
{
	mtk_cooler_gpu_dprintk("%s\n", __func__);

	if (cl_gpu_dev) {
		mtk_thermal_cooling_device_unregister(cl_gpu_dev);
		cl_gpu_dev = NULL;
	}
}

int mtk_cooler_gpu_init(void)
{
	int err = 0;

	err = mtk_cooler_gpu_register_ltf();
	if (err)
		goto err_unreg;

	return 0;

err_unreg:
	mtk_cooler_gpu_unregister_ltf();
	return err;
}

void mtk_cooler_gpu_exit(void)
{
	mtk_cooler_gpu_unregister_ltf();
}
