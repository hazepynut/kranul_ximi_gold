#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

ccflags-y  += -I$(srctree)/drivers/thermal/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/include/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/include/ppm_v3/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/thermal/fakeHeader/
ccflags-y  += -I$(srctree)/drivers/gpu/mediatek/gpufreq/include/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/gpu/hal/
ccflags-y  += -I$(srctree)/drivers/gpu/mediatek/gpufreq/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/base/power/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/base/power/include/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/base/power/include/gpufreq_v1/
ccflags-y  += -I$(srctree)/drivers/gpu/mediatek/gpufreq/v2/include/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/thermal/common/inc/
ccflags-y  += -I$(srctree)/drivers/gpu/drm/mediatek/mediatek_v2

ccflags-y  += -I$(srctree)/drivers/misc/mediatek/vpu/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/mdla/1.0

ccflags-y  += -I$(srctree)/drivers/gpu/mediatek/gpufreq/include/

include $(srctree)/drivers/misc/mediatek/thermal/common/thermal_zones/Makefile

ccflags-y  += -I$(srctree)/drivers/misc/mediatek/cpufreq_v2/inc/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/ppm_v3/inc/
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/

obj-$(CONFIG_MTK_LEGACY_THERMAL) += thermal_monitor.o

thermal_monitor-y += common/mtk_thermal_platform.o
thermal_monitor-y += common/ap_thermal_limit.o
thermal_monitor-y += common/mtk_thermal_timer.o
thermal_monitor-y += mtk_cooler_shutdown.o
thermal_monitor-y += mtk_cooler_kshutdown.o
thermal_monitor-y += mtk_cooler_cam.o

#thermal zones
thermal_monitor-y += common/thermal_zones/mtk_ts_battery.o
thermal_monitor-y += common/thermal_zones/mtk_ts_bts.o
thermal_monitor-y += common/thermal_zones/mtk_ts_btsmdpa.o

thermal_monitor-y += common/thermal_zones/mtk_ts_pa.o
thermal_monitor-y += common/thermal_zones/mtk_ts_pa_thput.o
# N17 code for HQ-296383 by liunianliang at 2023/05/17 start
thermal_monitor-y += common/thermal_zones/mtk_ts_btscharger.o
# N17 code for HQ-296383 by liunianliang at 2023/05/17 end


thermal_monitor-y += common/thermal_zones/mtk_ts_wmt.o

thermal_monitor-y += common/thermal_zones/mtk_tsAll.o

thermal_monitor-y += common/thermal_zones/mtk_ts_pmic.o

#thermal cooler

thermal_monitor-y += common/coolers/mtk_cooler_atm.o
thermal_monitor-y += common/coolers/mtk_cooler_dtm.o
thermal_monitor-y += common/coolers/mtk_cooler_sysrst.o
thermal_monitor-y += common/coolers/mtk_cooler_VR_FPS.o
thermal_monitor-y += common/coolers/mtk_ta.o


#thermal platform

ifneq ($(strip $(CONFIG_MTK_PLAT_POWER_MT6768)),)

include $(srctree)/drivers/misc/mediatek/thermal/src/Makefile
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/thermal/inc
thermal_monitor-y += common/thermal_zones/mtk_ts_dctm.o
thermal_monitor-y += common/thermal_zones/mtk_ts_imgsensor.o
thermal_monitor-y += mtk_thermal_monitor.o
thermal_monitor-y += src/mtk_tc.o
thermal_monitor-y += src/mtk_ts_6358buck1.o
thermal_monitor-y += src/mtk_ts_6358buck2.o
thermal_monitor-y += src/mtk_ts_6358buck3.o
thermal_monitor-y += common/thermal_zones/mtk_ts_cpu_noBank.o
ifeq ($(CONFIG_MTK_THERMAL_PA_VIA_ATCMD),y)
thermal_monitor-y += common/coolers/mtk_cooler_mutt.o
endif
thermal_monitor-y += common/coolers/mtk_cooler_bcct_v1.o
thermal_monitor-y += src/mtk_pmic_efuse.o
thermal_monitor-y += src/mtk_thermal_ipi.o

else ifneq ($(strip $(CONFIG_MTK_PLAT_POWER_MT6765)),)

include $(srctree)/drivers/misc/mediatek/thermal/v2/src/Makefile
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/thermal/v2/inc
thermal_monitor-y += v2/mtk_thermal_monitor.o
thermal_monitor-y += v2/src/mtk_tc.o
thermal_monitor-y += v2/src/mtk_ts_6357buck1.o
thermal_monitor-y += v2/src/mtk_ts_6357buck2.o
thermal_monitor-y += common/thermal_zones/mtk_ts_cpu_noBank.o
ifeq ($(CONFIG_MTK_THERMAL_PA_VIA_ATCMD),y)
thermal_monitor-y += common/coolers/mtk_cooler_mutt.o
endif
thermal_monitor-y += common/coolers/mtk_cooler_bcct_v1.o
thermal_monitor-y += v2/src/mtk_pmic_efuse.o
thermal_monitor-y += common/thermal_zones/mtk_ts_charger2.o

else ifneq ($(strip $(CONFIG_MTK_PLAT_POWER_MT6833)),)

include $(srctree)/drivers/misc/mediatek/thermal/v1/src/Makefile
ccflags-y  += -I$(srctree)/drivers/misc/mediatek/thermal/v1/inc
ccflags-y  += -I$(srctree)/drivers/gpu/mediatek/gpufreq/v2/include/
# N17 code for HQ-296383 by liunianliang at 2023/05/17 start
ccflags-y  += -I$(srctree)/drivers/power/supply
# N17 code for HQ-296383 by liunianliang at 2023/05/17 end
thermal_monitor-y += common/thermal_zones/mtk_ts_imgsensor.o
thermal_monitor-y += v1/mtk_thermal_monitor.o
thermal_monitor-y += v1/src/mtk_pmic_efuse_mt6833.o
thermal_monitor-y += v1/src/mtk_thermal_ipi_mt6833.o
thermal_monitor-y += v1/src/mtk_lvts_tc_mt6833.o
thermal_monitor-y += v1/src/mtk_tc_wrapper_mt6833.o
thermal_monitor-y += v1/src/mtk_ts_6359dcxo.o
thermal_monitor-y += v1/src/mtk_ts_6359tsx.o
thermal_monitor-y += v1/src/mtk_ts_6359vcore.o
thermal_monitor-y += v1/src/mtk_ts_6359vgpu.o
thermal_monitor-y += v1/src/mtk_ts_6359vproc.o

# N17 code for HQ-296383 by liunianliang at 2023/05/17 start
#thermal_monitor-y += common/thermal_zones/mtk_ts_btsnrpa.o
# N17 code for HQ-296383 by liunianliang at 2023/05/17 end
thermal_monitor-y += common/thermal_zones/mtk_ts_dctm.o
thermal_monitor-y += common/coolers/mtk_cooler_bcct_v2.o
thermal_monitor-y += common/coolers/mtk_cooler_mutt_gen97.o
thermal_monitor-y += common/thermal_zones/mtk_ts_cpu_noBankv2.o

# N17 code for HQ-296383 by liunianliang at 2023/05/17 start
thermal_monitor-y += common/thermal_zones/mtk_cs_cpu.o
thermal_monitor-y += common/coolers/mtk_cooler_gpu_cus.o
thermal_monitor-y += common/coolers/mtk_cooler_md_cus.o
thermal_monitor-y += common/coolers/mtk_cooler_char_curr_cus.o

thermal_monitor-y += common/mi_thermal_message.o
# N17 code for HQ-296383 by liunianliang at 2023/05/17 end
endif
