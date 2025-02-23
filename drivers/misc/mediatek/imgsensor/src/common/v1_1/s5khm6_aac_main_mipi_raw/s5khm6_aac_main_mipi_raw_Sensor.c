#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/types.h>

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "s5khm6_aac_main_mipi_raw_Sensor.h"
#include "s5khm6_aac_setting.h"

#undef VENDOR_EDIT

#define PFX "S5KHM6_camera_sensor"

#define LOG_INF(format, args...)	pr_debug(PFX "[%s] " format, __func__, ##args)

#define I2C_BUFFER_LEN 255 /* trans# max is 255, each 3 bytes */

//N17 code for HQ-293325 by jixinji at 2023/05/10 start
//N17 code for HQ-297334 by liyang at 2023/05/31 start
//N17 code for HQ-297662 by liyang at 2023/06/15 start
#define S5KHM6_PDAF_SWITCH 1
//N17 code for HQ-297662 by liyang at 2023/06/15 end
//N17 code for HQ-297334 by liyang at 2023/05/31 end
//N17 code for HQ-293325 by wuzhenyue at 2023/05/16 start
#define VENDOR_ID 0x10
//N17 code for HQ-293325 by wuzhenyue at 2023/05/16 end
static bool bIsLongExposure = KAL_FALSE;

static kal_uint16 s5khm6_table_write_cmos_sensor(kal_uint16 * para, kal_uint32 len);

static DEFINE_SPINLOCK(imgsensor_drv_lock);

//N17 code for HQ-293325 by wangqiang start
static struct imgsensor_info_struct imgsensor_info = {
	.sensor_id = S5KHM6_AAC_MAIN_SENSOR_ID,	//0x1AD7

	.checksum_value = 0xa4c32546,
//N17 code for HQ-293325 by wuzhenyue at 2023/05/10 start
	.pre = {
		.pclk = 1640000000,
/* N17 code for HQ-299317 by miaozhongshu at 2023/06/16 start */
		.linelength = 14080,
		.framelength = 3882,
/* N17 code for HQ-299317 by miaozhongshu at 2023/06/16 end */
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4000,
		.grabwindow_height = 3000,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 1440504000,
		.max_framerate = 300,
	},
	.cap = {
		.pclk = 1640000000,
/* N17 code for HQ-308731 by wangqiang at 2023/07/25 start */
		.linelength = 16896,
		.framelength = 3234,
/* N17 code for HQ-308731 by wangqiang at 2023/07/25 end */
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4000,
		.grabwindow_height = 3000,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 1440504000,
		.max_framerate = 300,
	},
	.normal_video = {
		.pclk = 1640000000,
		.linelength = 11088,
		.framelength = 4928,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4000,
		.grabwindow_height = 2252,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 1440504000,
		.max_framerate = 300,
	},
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
//N17 code for HQ-293325 by wuzhenyue at 2023/05/16 start
	.hs_video = {
		.pclk = 1640000000,
		.linelength = 8704,
		.framelength = 1564,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 1920,
		.grabwindow_height = 1080,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 1440504000,
		.max_framerate = 1200,
	},
//N17 code for HQ-293325 by wuzhenyue at 2023/05/16 end
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
	.slim_video = {
		.pclk = 1640000000,
		.linelength = 17360,
		.framelength = 3148,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4000,
		.grabwindow_height = 3000,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 838400000,
		.max_framerate = 300,
	},
	.custom1 = {
		.pclk = 1640000000,
/* N17 code for HQ-293330 by xuyanfei at 2023/05/11 start */
		.linelength = 11152,
		.framelength = 6127,
/* N17 code for HQ-293330 by xuyanfei at 2023/05/11 end*/
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4000,
		.grabwindow_height = 3000,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 1440504000,
		.max_framerate = 240,
	},
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
//N17 code for HQ-319475 by wuzhenyue at 2023/08/17 start
	.custom2 = {
		.pclk = 1640000000,
		.linelength = 20472,
		.framelength = 13351,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 12000,
		.grabwindow_height = 9000,
		.mipi_data_lp2hs_settle_dc = 0x22,
		.max_framerate = 60,
		.mipi_pixel_rate = 1440504000,
	},
//N17 code for HQ-319475 by wuzhenyue at 2023/08/17 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
	.custom3 = {
		.pclk = 1640000000,
		.linelength = 16896,
		.framelength = 3234,
		.startx = 0,
		.starty = 0,
		.grabwindow_width = 4000,
		.grabwindow_height = 3000,
		.mipi_data_lp2hs_settle_dc = 85,
		.mipi_pixel_rate = 1440504000,
		.max_framerate = 300,
	},
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
//N17 code for HQ-293325 by wuzhenyue at 2023/05/10 end
//N17 code for HQ-293325 by jixinji at 2023/05/17 start
	.margin = 25,		/* sensor framelength & shutter margin */
	.min_shutter = 12,	/* min shutter */
	.min_gain = BASEGAIN,
	.max_gain = 64 * BASEGAIN,
	.min_gain_iso = 50,
	.exp_step = 2,
	.gain_step = 1,
	.gain_type = 0,
	.max_frame_length = 0x56FFFF,
//N17 code for HQ-293325 by jixinji at 2023/05/17 end
	.ae_shut_delay_frame = 0,
	.ae_sensor_gain_delay_frame = 0,
	.ae_ispGain_delay_frame = 2,	/* isp gain delay frame for AE cycle */
	.ihdr_support = 0,	/* 1, support; 0,not support */
	.ihdr_le_firstline = 0,	/* 1,le first ; 0, se first */
	.temperature_support = 1,/* 1, support; 0,not support */
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
	.sensor_mode_num = 8,	/* support sensor mode num */
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end

	.cap_delay_frame = 2,	/* enter capture delay frame num */
	.pre_delay_frame = 2,	/* enter preview delay frame num */
	.video_delay_frame = 2,	/* enter video delay frame num */
	.hs_video_delay_frame = 2,
	.slim_video_delay_frame = 2,	/* enter slim video delay frame num */
	.custom1_delay_frame = 2,	/* enter custom1 delay frame num */
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
	.custom2_delay_frame = 2,	/* enter custom2 delay frame num */
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
	.custom3_delay_frame = 2,	/* enter custom3 delay frame num */
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
	.frame_time_delay_frame = 2,

	.isp_driving_current = ISP_DRIVING_4MA,
	.sensor_interface_type = SENSOR_INTERFACE_TYPE_MIPI,

	.mipi_sensor_type = MIPI_CPHY, /* 0,MIPI_OPHY_NCSI2; 1,MIPI_OPHY_CSI2 */

	.mipi_settle_delay_mode = 0,
	.sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_Gb,
	.mclk = 24, /* mclk value, suggest 24 or 26 for 24Mhz or 26Mhz */

	.mipi_lane_num = SENSOR_MIPI_3_LANE,

	.i2c_addr_table = {0x20, 0x5a, 0x7a, 0xac, 0xff},

	.i2c_speed = 1000, /* i2c read/write speed */
};
//N17 code for HQ-293325 by wangqiang end

static struct imgsensor_struct imgsensor = {
	.mirror = IMAGE_NORMAL,	/* mirrorflip information */
	.sensor_mode = IMGSENSOR_MODE_INIT,
	.shutter = 0x3D0,	/* current shutter */
	.gain = 0x100,		/* current gain */
	.dummy_pixel = 0,	/* current dummypixel */
	.dummy_line = 0,	/* current dummyline */
	.current_fps = 300,
	.autoflicker_en = KAL_FALSE,
	.test_pattern = KAL_FALSE,
	.current_scenario_id = MSDK_SCENARIO_ID_CAMERA_PREVIEW,
	.ihdr_mode = 0, /* sensor need support LE, SE with HDR feature */
	.i2c_write_id = 0x20, /* record current sensor's i2c write id */
	.current_ae_effective_frame = 2,
	.ae_frm_mode.frame_mode_1 = IMGSENSOR_AE_MODE_SE,
	.ae_frm_mode.frame_mode_2 = IMGSENSOR_AE_MODE_SE,
};

//N17 code for HQ-293325 by wangqiang start
/* Sensor output window information */
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
static struct SENSOR_WINSIZE_INFO_STRUCT imgsensor_winsize_info[8] = {
	{12000, 9000, 0,   0, 12000, 9000,  4000, 3000, 0, 0,  4000, 3000,  0,  0,  4000, 3000}, /* Preview */
	{12000, 9000, 0,   0, 12000, 9000,  4000, 3000, 0, 0,  4000, 3000,  0,  0,  4000, 3000}, /* capture */
	{12000, 9000, 0,1122, 12000, 6756,  4000, 2252, 0, 0,  4000, 2252,  0,  0,  4000, 2252}, // video
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
	{12000, 9000, 240,1260,11520, 6480, 1920, 1080, 0, 0,  1920, 1080,  0,  0,  1920, 1080}, /* hs_video */
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
	{12000, 9000, 0,   0, 12000, 9000,  4000, 3000, 0, 0,  4000, 3000,  0,  0,  4000, 3000}, /* slim video */
	{12000, 9000, 0,   0, 12000, 9000,  4000, 3000, 0, 0,  4000, 3000,  0,  0,  4000, 3000}, /* custom1 */
	{12000, 9000, 0,   0, 12000, 9000, 12000, 9000, 0, 0, 12000, 9000,  0,  0, 12000, 9000},/* custom2 */
	{12000, 9000,4000,3000,4000, 3000,  4000, 3000, 0, 0,  4000, 3000,  0,  0,  4000, 3000},/* custom3 */
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
};
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
 /*VC1 for HDR(DT=0X35), VC2 for PDAF(DT=0X36), unit : 10bit */
 //N17 code for HQ-293325 by wangqiang end
 //N17 code for HQ-293325 by jixinji at 2023/05/10 start
 //N17 code for HQ-297662 by liyang at 2023/06/15 start
 //N17 code for HQ-310269 by wangqiang at 2023/08/02 start
static struct SENSOR_VC_INFO_STRUCT SENSOR_VC_INFO[6] = {
	/* Preview mode setting */
	{0x03, 0x0a, 0x00, 0x08, 0x40, 0x00,
	 0x00, 0x2b, 0x0FA0, 0x0BB8, 0x00, 0x00, 0x0280, 0x0001,
	 0x01, 0x2b, 2000,   1500,   0x03, 0x00, 0x0000, 0x0000},
	/* Normal_Video mode setting */
	{0x03, 0x0a, 0x00, 0x08, 0x40, 0x00,
	 0x00, 0x2b, 0x0FA0, 2252, 0x00, 0x00, 0x00, 0x00,
	 0x01, 0x2b, 2000, 1126,0x00, 0x00, 0x0000, 0x0000},
	/* 4K_Video mode setting */
	{0x02, 0x0a, 0x00, 0x08, 0x40, 0x00,
	 0x00, 0x2b, 0x0F00, 0x0870, 0x00, 0x00, 0x0000, 0x0000,
	 0x00, 0x34, 0x04B0, 0x0430, 0x00, 0x00, 0x0000, 0x0000},
	/* Slim_Video mode setting */
	{0x02, 0x0a, 0x00, 0x08, 0x40, 0x00,
	 0x00, 0x2b, 0x0FA0, 0x08d0, 0x00, 0x00, 0x0000, 0x0000,
	 0x00, 0x34, 0x04D8, 0x0460, 0x00, 0x00, 0x0000, 0x0000},
	 /*custom1 setting*/
	 {0x03, 0x0a, 0x00, 0x08, 0x40, 0x00,
	 0x00, 0x2b, 0x0780, 0x0438, 0x00, 0x00, 0x00, 0x00,
	 0x00, 0x00, 0x0000, 0x0000, 0x00, 0x00, 0x0000, 0x0000},
	 /*isz setting*/
    {0x03, 0x0a, 0x00, 0x08, 0x40, 0x00,
     0x00, 0x2b, 4000, 3000, 0x00, 0x00, 0x0000, 0x0000,
     0x01, 0x2b, 1332, 1000, 0x00, 0x00, 0x0000, 0x0000},
};
static struct SET_PD_BLOCK_INFO_T imgsensor_pd_info_isz = {
		.i4OffsetX	= 1,
		.i4OffsetY	= 0,
		.i4PitchX	= 12,
		.i4PitchY	= 12,
		.i4PairNum	= 8,
		.i4SubBlkW	= 3,
		.i4SubBlkH	= 6,
		.i4BlockNumX = 333,
		.i4BlockNumY = 250,
		.iMirrorFlip = 0,
		.i4PosL = {
			{3, 1}, {6, 1}, {9, 1}, {12, 1}, {3, 7}, {6, 7}, {9, 7}, {12, 7},
		},
		.i4PosR = {
			{2, 1}, {5, 1}, {8, 1}, {11, 1}, {2, 7}, {5, 7}, {8, 7}, {11, 7},
		},
		.i4Crop = {
			{0,0},{4000,3000},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
			{0,0},{0,0},{0,0},
		},
};
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
/* If mirror flip */
static struct SET_PD_BLOCK_INFO_T imgsensor_pd_info_binning = {
		.i4OffsetX	= 0,
		.i4OffsetY	= 0,
		.i4PitchX	= 4,
		.i4PitchY	= 4,
		.i4PairNum	= 2,
		.i4SubBlkW	= 2,
		.i4SubBlkH	= 4,
		.i4BlockNumX = 1000,
		.i4BlockNumY = 750,
		.iMirrorFlip = 0,
		.i4PosL = {
			{1, 0}, {3, 0},
		},
		.i4PosR = {
			{0, 0}, {2, 0},
		},
};
//N17 code for HQ-297662 by liyang at 2023/06/26 start
static struct SET_PD_BLOCK_INFO_T imgsensor_pd_info_binning_video = {
		.i4OffsetX	= 0,
		.i4OffsetY	= 0,
		.i4PitchX	= 4,
		.i4PitchY	= 4,
		.i4PairNum	= 2,
		.i4SubBlkW	= 2,
		.i4SubBlkH	= 4,
		.i4BlockNumX = 1000,
		.i4BlockNumY = 563,
		.iMirrorFlip = 0,
		.i4PosL = {
			{1, 0}, {3, 0},
		},
		.i4PosR = {
			{0, 0}, {2, 0},
		},
		.i4Crop = {
			{0,0},{0,0},{0,374},{0,0},{0,0},{0,0},{0,0},{0,374},{0,0},
			{0,0},{0,0},{0,374},
		},
};
//N17 code for HQ-297662 by liyang at 2023/06/26 end
//N17 code for HQ-297662 by liyang at 2023/06/15 end
//N17 code for HQ-293325 by jixinji at 2023/05/10 end

static kal_uint16 read_cmos_sensor(kal_uint32 addr)
{
	kal_uint16 get_byte = 0;
	char pusendcmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF)};

	iReadRegI2C(pusendcmd, 2, (u8 *)&get_byte, 2, imgsensor.i2c_write_id);
	return ((get_byte<<8)&0xff00) | ((get_byte>>8)&0x00ff);
}

static void write_cmos_sensor(kal_uint16 addr, kal_uint16 para)
{
	char pusendcmd[4] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para >> 8), (char)(para & 0xFF)};

	iWriteRegI2C(pusendcmd, 4, imgsensor.i2c_write_id);
}

static kal_uint16 read_cmos_sensor_8(kal_uint16 addr)
{
	kal_uint16 get_byte = 0;
	char pusendcmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2C(pusendcmd, 2, (u8 *)&get_byte, 1, imgsensor.i2c_write_id);
	return get_byte;
}

static void write_cmos_sensor_8(kal_uint16 addr, kal_uint8 para)
{
	char pusendcmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};

	iWriteRegI2C(pusendcmd, 3, imgsensor.i2c_write_id);
}

static void s5khm6_get_pdaf_reg_setting(MUINT32 regNum, kal_uint16 *regDa)
{
	int i, idx;

	for (i = 0; i < regNum; i++) {
		idx = 2 * i;
		regDa[idx + 1] = read_cmos_sensor_8(regDa[idx]);
		LOG_INF("%x %x", regDa[idx], regDa[idx+1]);
	}
}
static void s5khm6_set_pdaf_reg_setting(MUINT32 regNum, kal_uint16 *regDa)
{
	int i, idx;

	for (i = 0; i < regNum; i++) {
		idx = 2 * i;
		write_cmos_sensor_8(regDa[idx], regDa[idx + 1]);
		LOG_INF("%x %x", regDa[idx], regDa[idx+1]);
	}
}

static void set_dummy(void)
{
	LOG_INF("dummyline = %d, dummypixels = %d\n", imgsensor.dummy_line, imgsensor.dummy_pixel);

	write_cmos_sensor(0x0340, imgsensor.frame_length);
}	/*	set_dummy  */

static void set_mirror_flip(kal_uint8 image_mirror)
{
	kal_uint8 itemp;

	LOG_INF("image_mirror = %d\n", image_mirror);
	itemp = read_cmos_sensor_8(0x0101);
	itemp &= ~0x03;

	switch (image_mirror) {

	case IMAGE_NORMAL:
	write_cmos_sensor_8(0x0101, itemp);
	break;

	case IMAGE_V_MIRROR:
	write_cmos_sensor_8(0x0101, itemp | 0x02);
	break;

	case IMAGE_H_MIRROR:
	write_cmos_sensor_8(0x0101, itemp | 0x01);
	break;

	case IMAGE_HV_MIRROR:
	write_cmos_sensor_8(0x0101, itemp | 0x03);
	break;
	}
}

static void set_max_framerate(UINT16 framerate, kal_bool min_framelength_en)
{
	/*kal_int16 dummy_line;*/
	kal_uint32 frame_length = imgsensor.frame_length;

	LOG_INF("framerate = %d, min framelength should enable %d\n", framerate, min_framelength_en);

	frame_length = imgsensor.pclk / framerate * 10 / imgsensor.line_length;
	spin_lock(&imgsensor_drv_lock);
	if (frame_length >= imgsensor.min_frame_length)
		imgsensor.frame_length = frame_length;
	else
		imgsensor.frame_length = imgsensor.min_frame_length;

	imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length) {
		imgsensor.frame_length = imgsensor_info.max_frame_length;
		imgsensor.dummy_line = imgsensor.frame_length - imgsensor.min_frame_length;
	}
	if (min_framelength_en)
		imgsensor.min_frame_length = imgsensor.frame_length;
	spin_unlock(&imgsensor_drv_lock);
	set_dummy();
}	/*	set_max_framerate  */

/*************************************************************************
 * FUNCTION
 *	set_shutter
 *
 * DESCRIPTION
 *	This function set e-shutter of sensor to change exposure time.
 *
 * PARAMETERS
 *	iShutter : exposured lines
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static void set_shutter(kal_uint32 shutter)
{
	unsigned long flags;
	kal_uint16 realtime_fps = 0;

	if(shutter >= 72150)
	{
		/*enter long exposure mode */
		kal_uint32 exposure_time;
		kal_uint16 new_framelength;
		kal_uint16 long_shutter=0;

		bIsLongExposure = KAL_TRUE;
		exposure_time = shutter*imgsensor.line_length/1599000;
		long_shutter = shutter / 128 ;
		LOG_INF("Long Exposure Mode long_shutter = %d, long_exposure_time =%ld ms\n", long_shutter, exposure_time);
		new_framelength = long_shutter + 24;

		write_cmos_sensor(0x0340, new_framelength);
		write_cmos_sensor(0x0202, long_shutter);
		write_cmos_sensor(0x0702, 0x0700);
		write_cmos_sensor(0x0704, 0x0700);
	}
	else
	{
		if (bIsLongExposure)
		{
			LOG_INF("Normal Exposure Mode\n");
			write_cmos_sensor(0x6028, 0x4000);
			write_cmos_sensor(0x6028, 0x4000);
			write_cmos_sensor(0x0340, imgsensor.frame_length);
			write_cmos_sensor(0x0702, 0x0000);
			write_cmos_sensor(0x0704, 0x0000);
			write_cmos_sensor(0x0202, shutter);
			bIsLongExposure = KAL_FALSE;
		}

		spin_lock_irqsave(&imgsensor_drv_lock, flags);
		imgsensor.shutter = shutter;
		spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

		spin_lock(&imgsensor_drv_lock);
		if (shutter > imgsensor.min_frame_length - imgsensor_info.margin)
			imgsensor.frame_length = shutter + imgsensor_info.margin;
		else
			imgsensor.frame_length = imgsensor.min_frame_length;
		if (imgsensor.frame_length > imgsensor_info.max_frame_length)
			imgsensor.frame_length = imgsensor_info.max_frame_length;
		spin_unlock(&imgsensor_drv_lock);
		shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
		shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin)) ? (imgsensor_info.max_frame_length - imgsensor_info.margin) : shutter;

		if (imgsensor.autoflicker_en) {
			realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 / imgsensor.frame_length;
			if(realtime_fps >= 297 && realtime_fps <= 305)
				set_max_framerate(296,0);
			else if(realtime_fps >= 147 && realtime_fps <= 150)
				set_max_framerate(146,0);
			else {
			// Extend frame length
				write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
			}
		} else {
			// Extend frame length
			write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
		}

		// Update Shutter
		write_cmos_sensor(0x0202, shutter & 0xFFFF);
		LOG_INF("Exit! shutter =%d, framelength =%d\n", shutter,imgsensor.frame_length);
	}
} /* set_shutter */


/*************************************************************************
 * FUNCTION
 *	set_shutter_frame_length
 *
 * DESCRIPTION
 *	for frame & 3A sync
 *
 *************************************************************************/
static void set_shutter_frame_length(kal_uint16 shutter, kal_uint16 frame_length, kal_bool auto_extend_en)
{
	unsigned long flags;
	kal_uint16 realtime_fps = 0;
	kal_int32 dummy_line = 0;

	spin_lock_irqsave(&imgsensor_drv_lock, flags);
	imgsensor.shutter = shutter;
	spin_unlock_irqrestore(&imgsensor_drv_lock, flags);

	spin_lock(&imgsensor_drv_lock);
	/* Change frame time */
	if (frame_length > 1)
		dummy_line = frame_length - imgsensor.frame_length;

	imgsensor.frame_length = imgsensor.frame_length + dummy_line;

	if (shutter > imgsensor.frame_length - imgsensor_info.margin)
		imgsensor.frame_length = shutter + imgsensor_info.margin;

	if (imgsensor.frame_length > imgsensor_info.max_frame_length)
		imgsensor.frame_length = imgsensor_info.max_frame_length;
	spin_unlock(&imgsensor_drv_lock);
	shutter = (shutter < imgsensor_info.min_shutter) ? imgsensor_info.min_shutter : shutter;
	shutter = (shutter > (imgsensor_info.max_frame_length - imgsensor_info.margin))
		? (imgsensor_info.max_frame_length - imgsensor_info.margin)	: shutter;

	if (imgsensor.autoflicker_en) {
		realtime_fps = imgsensor.pclk / imgsensor.line_length * 10 /
				imgsensor.frame_length;
		if (realtime_fps >= 297 && realtime_fps <= 305)
			set_max_framerate(296, 0);
		else if (realtime_fps >= 147 && realtime_fps <= 150)
			set_max_framerate(146, 0);
		else {
			/* Extend frame length */
			write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
		}
	} else {
		/* Extend frame length */
		write_cmos_sensor(0x0340, imgsensor.frame_length & 0xFFFF);
	}

	/* Update Shutter */
	write_cmos_sensor(0x0202, shutter);
	LOG_INF("Exit! shutter =%d, framelength =%d/%d, dummy_line=%d\n", shutter, imgsensor.frame_length, frame_length, dummy_line);

}	/* set_shutter_frame_length */

static kal_uint16 gain2reg(const kal_uint16 gain)
{
	kal_uint16 reg_gain = 0x0000;
    reg_gain = gain/2;
	return (kal_uint16)reg_gain;
}

/*************************************************************************
 * FUNCTION
 *	set_gain
 *
 * DESCRIPTION
 *	This function is to set global gain to sensor.
 *
 * PARAMETERS
 *	iGain : sensor global gain(base: 0x40)
 *
 * RETURNS
 *	the actually gain set to sensor.
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint16 set_gain(kal_uint16 gain)
{
	kal_uint16 reg_gain, max_gain = 64 * BASEGAIN;

//N17 code for HQ-293325 by wangqiang at 2023/05/29 start
	if (imgsensor.sensor_mode == IMGSENSOR_MODE_CUSTOM2) {
		max_gain = 8 * BASEGAIN;
	}
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
	else if (imgsensor.sensor_mode == IMGSENSOR_MODE_CUSTOM3) {
		max_gain = 16 * BASEGAIN;
	}
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end/

	if (gain < BASEGAIN || gain > max_gain) {
		LOG_INF("Error current gain :%d max gain setting: %d\n", gain, max_gain);
//N17 code for HQ-293325 by wangqiang at 2023/05/29 end
		if (gain < BASEGAIN)
			gain = BASEGAIN;
		else if (gain > max_gain)
			gain = max_gain;
	}

	reg_gain = gain2reg(gain);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.gain = reg_gain;
	spin_unlock(&imgsensor_drv_lock);
	LOG_INF("gain = %d, reg_gain = 0x%x, max_gain:0x%x\n ",	gain, reg_gain, max_gain);

	write_cmos_sensor(0x0204, reg_gain);

	return gain;
} /* set_gain */

static kal_uint32 s5khm6_awb_gain(struct SET_SENSOR_AWB_GAIN *pSetSensorAWB)
{
	#if 0
	UINT32 rgain_32, grgain_32, gbgain_32, bgain_32;

	grgain_32 = (pSetSensorAWB->ABS_GAIN_GR + 1) >> 1;
	rgain_32 = (pSetSensorAWB->ABS_GAIN_R + 1) >> 1;
	bgain_32 = (pSetSensorAWB->ABS_GAIN_B + 1) >> 1;
	gbgain_32 = (pSetSensorAWB->ABS_GAIN_GB + 1) >> 1;

	write_cmos_sensor_8(0x0b8e, (grgain_32 >> 8) & 0xFF);
	write_cmos_sensor_8(0x0b8f, grgain_32 & 0xFF);
	write_cmos_sensor_8(0x0b90, (rgain_32 >> 8) & 0xFF);
	write_cmos_sensor_8(0x0b91, rgain_32 & 0xFF);
	write_cmos_sensor_8(0x0b92, (bgain_32 >> 8) & 0xFF);
	write_cmos_sensor_8(0x0b93, bgain_32 & 0xFF);
	write_cmos_sensor_8(0x0b94, (gbgain_32 >> 8) & 0xFF);
	write_cmos_sensor_8(0x0b95, gbgain_32 & 0xFF);

	s5khm6_awb_gain_table[1]  = (grgain_32 >> 8) & 0xFF;
	s5khm6_awb_gain_table[3]  = grgain_32 & 0xFF;
	s5khm6_awb_gain_table[5]  = (rgain_32 >> 8) & 0xFF;
	s5khm6_awb_gain_table[7]  = rgain_32 & 0xFF;
	s5khm6_awb_gain_table[9]  = (bgain_32 >> 8) & 0xFF;
	s5khm6_awb_gain_table[11] = bgain_32 & 0xFF;
	s5khm6_awb_gain_table[13] = (gbgain_32 >> 8) & 0xFF;
	s5khm6_awb_gain_table[15] = gbgain_32 & 0xFF;
	s5khm6_table_write_cmos_sensor(s5khm6_awb_gain_table, sizeof(s5khm6_awb_gain_table)/sizeof(kal_uint16));
	#endif

	return ERROR_NONE;
}

//N17 code for HQ-293325 by wangqiang at 2023/05/29 start
/*write AWB gain to sensor*/
static void feedback_awbgain(struct SET_SENSOR_AWB_GAIN *pSetSensorAWB)
{
	UINT32 r_gain = pSetSensorAWB->ABS_GAIN_R;
	UINT32 g_gain = (pSetSensorAWB->ABS_GAIN_GR + pSetSensorAWB->ABS_GAIN_GB)/2;
	UINT32 b_gain = pSetSensorAWB->ABS_GAIN_B;

//N17 code for HQ-292705 by wangqiang at 2023/05/30 start
	write_cmos_sensor(0x0D82, r_gain * 2);
	write_cmos_sensor(0x0D84, g_gain * 2);
	write_cmos_sensor(0X0D86, b_gain * 2);
//N17 code for HQ-292705 by wangqiang at 2023/05/30 end
	LOG_INF("r:g:b=%d:%d:%d, origin r:gr:gb:b=%d:%d:%d:%d", r_gain, g_gain, b_gain,
		pSetSensorAWB->ABS_GAIN_R, pSetSensorAWB->ABS_GAIN_GR, pSetSensorAWB->ABS_GAIN_GB, pSetSensorAWB->ABS_GAIN_B);
}
//N17 code for HQ-293325 by wangqiang at 2023/05/29 end

static void check_stremoff(void)
{
	unsigned int i = 0, framecnt = 0;
	int timeout = 50;

	for (i = 0; i < timeout; i++) {
		framecnt = read_cmos_sensor_8(0x0005);
		if (framecnt == 0xFF)
			return;
		else
			mdelay(2);
	}
	LOG_INF(" Stream Off Fail!!!\n");
}

static kal_uint32 streaming_control(kal_bool enable)
{
	LOG_INF("streaming_enable(0=Sw Standby,1=streaming): %d\n", enable);
//N17 code for HQ-293325 by wangqiang start
	write_cmos_sensor(0xFCFC, 0x4000);
	if (enable)
	{
		write_cmos_sensor(0x0100, 0X0103);
	}
	else
	{
		write_cmos_sensor(0x0100, 0x0000);
//N17 code for HQ-293325 by wangqiang end
		check_stremoff();
	}
	return ERROR_NONE;
}

static kal_uint16 s5khm6_table_write_cmos_sensor(kal_uint16 *para, kal_uint32 len)
{
	char puSendCmd[I2C_BUFFER_LEN];
	kal_uint32 tosend, IDX;
	kal_uint16 addr = 0, addr_last = 0, data;

	tosend = 0;
	IDX = 0;

	while (len > IDX) {
		addr = para[IDX];

		{
			puSendCmd[tosend++] = (char)(addr >> 8);
			puSendCmd[tosend++] = (char)(addr & 0xFF);
			data = para[IDX + 1];
			puSendCmd[tosend++] = (char)(data >> 8);
			puSendCmd[tosend++] = (char)(data & 0xFF);
			IDX += 2;
			addr_last = addr;
		}
		/* Write when remain buffer size is less than 3 bytes
		 * or reach end of data
		 */
		if ((I2C_BUFFER_LEN - tosend) < 4 || IDX == len || addr != addr_last) {
			iBurstWriteReg_multi(puSendCmd,	tosend,	imgsensor.i2c_write_id,	4, imgsensor_info.i2c_speed);
			tosend = 0;
		}
	}
	return 0;
}

static void sensor_init(void)
{
	LOG_INF("+\n");
//N17 code for HQ-293325 by wangqiang start
	write_cmos_sensor(0xFCFC, 0x4000);
	write_cmos_sensor(0x0000, 0x01C0);
	write_cmos_sensor(0x0000, 0x1AD6);
	write_cmos_sensor(0xFCFC, 0x4000);//init
	write_cmos_sensor(0x6010, 0x0001);
	mdelay(30); //p30   delay 30ms
	write_cmos_sensor(0x6218, 0xE9C0);
	write_cmos_sensor(0xF468, 0x0000);
	write_cmos_sensor(0x0136, 0x1800);
	mdelay(1);
	// 2nd Tnp
//N17 code for HQ-293325 by wangqiang end
	s5khm6_table_write_cmos_sensor(s5khm6_init_setting,	sizeof(s5khm6_init_setting)/sizeof(kal_uint16));
	LOG_INF("-\n");
}	/*	  sensor_init  */

static void preview_setting(void)
{
	LOG_INF("+\n");
	s5khm6_table_write_cmos_sensor(s5khm6_preview_setting, sizeof(s5khm6_preview_setting)/sizeof(kal_uint16));
	LOG_INF("-\n");
} /* preview_setting */

static void capture_setting(kal_uint16 currefps)
{
	LOG_INF("+ currefps:%d\n", currefps);
/* N17 code for HQ-HQ-308731 by wangqiang at 2023/07/25 start */
	s5khm6_table_write_cmos_sensor(s5khm6_capture_setting, sizeof(s5khm6_capture_setting)/sizeof(kal_uint16));
/* N17 code for HQ-HQ-308731 by wangqiang at 2023/07/25 end */
	LOG_INF("-\n");
}

static void normal_video_setting(kal_uint16 currefps)
{
	LOG_INF("+ currefps:%d\n", currefps);
//N17 code for HQ-293325 by wangqiang start
	s5khm6_table_write_cmos_sensor(s5khm6_normal_video_setting,	sizeof(s5khm6_normal_video_setting)/sizeof(kal_uint16));
//N17 code for HQ-293325 by wangqiang end
	LOG_INF("-\n");
}

static void hs_video_setting(void)
{
	LOG_INF("+\n");
//N17 code for HQ-293325 by jixinji at 2023/05/30 start
	s5khm6_table_write_cmos_sensor(s5khm6_hs_video_setting,	sizeof(s5khm6_hs_video_setting)/sizeof(kal_uint16));
	//s5khm6_table_write_cmos_sensor(s5khm6_preview_setting, sizeof(s5khm6_preview_setting)/sizeof(kal_uint16));
//N17 code for HQ-293325 by jixinji at 2023/05/30 end
	LOG_INF("-\n");
}

static void slim_video_setting(void)
{
	LOG_INF("+\n");
	//s5khm6_table_write_cmos_sensor(s5khm6_slim_video_setting, sizeof(s5khm6_slim_video_setting)/sizeof(kal_uint16));
	s5khm6_table_write_cmos_sensor(s5khm6_preview_setting, sizeof(s5khm6_preview_setting)/sizeof(kal_uint16));
	LOG_INF("-*\n");
}

static void custom1_setting(void)
{
	LOG_INF("+\n");
	s5khm6_table_write_cmos_sensor(s5khm6_custom1_setting, sizeof(s5khm6_custom1_setting)/sizeof(kal_uint16));
	LOG_INF("-\n");
}

//N17 code for HQ-293325 by jixinji at 2023/05/11 start
static void custom2_setting(void)
{
	LOG_INF("+\n");
	s5khm6_table_write_cmos_sensor(s5khm6_custom2_setting, sizeof(s5khm6_custom2_setting)/sizeof(kal_uint16));
	LOG_INF("-\n");
}
//N17 code for HQ-293325 by jixinji at 2023/05/11 end

//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
static void custom3_setting(void)
{
	LOG_INF("+\n");
	s5khm6_table_write_cmos_sensor(s5khm6_custom3_setting, sizeof(s5khm6_custom3_setting)/sizeof(kal_uint16));
	LOG_INF("-\n");
}
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
static kal_uint16 get_vendor_id(void)
{
	kal_uint16 get_byte = 0;
	char pusendcmd[2] = { (char)(0x01 >> 8), (char)(0x01 & 0xFF) };
	iReadRegI2C(pusendcmd, 2, (u8 *) &get_byte, 1, 0xA2);
	return get_byte;
}
/*************************************************************************
 * FUNCTION
 *	get_imgsensor_id
 *
 * DESCRIPTION
 *	This function get the sensor ID
 *
 * PARAMETERS
 *	*sensorID : return the sensor ID
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 get_imgsensor_id(UINT32 *sensor_id)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 3;
	kal_uint16 vendor_id = 0;
	vendor_id = get_vendor_id();
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
//N17 code for HQ-293325 by wuzhenyue at 2023/05/16 start
			*sensor_id = read_cmos_sensor(0x0000)+1;
			LOG_INF("s5khm6-aac sensor id = 0x%x, vendor id = 0x%x",*sensor_id ,vendor_id);
			if (*sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("s5khm6-aac i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id, *sensor_id);
				if(vendor_id == VENDOR_ID){
					return ERROR_NONE;
				}
				else{
					LOG_INF("s5khm6-aac read vendor id fail");
					*sensor_id = 0xFFFFFFFF;
					return ERROR_SENSOR_CONNECT_FAIL;
				}
			}
			LOG_INF("s5khm6-aac Read sensor id fail, sensor id: 0x%x, i2c: 0x%x\n", *sensor_id, imgsensor.i2c_write_id);
			retry--;
		} while (retry > 0);
		i++;
		retry = 2;
	}
//N17 code for HQ-293325 by wuzhenyue at 2023/05/16 end
	if (*sensor_id != imgsensor_info.sensor_id) {
//N17 code for HQ-293325 by wangqiang start
		/*if Sensor ID is not correct,
		 *Must set *sensor_id to 0xFFFFFFFF
		 */
		*sensor_id = 0xFFFFFFFF;
		return ERROR_SENSOR_CONNECT_FAIL;
//N17 code for HQ-293325 by wangqiang end
	}
	return ERROR_NONE;
}


/*************************************************************************
 * FUNCTION
 *	open
 *
 * DESCRIPTION
 *	This function initialize the registers of CMOS sensor
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 open(void)
{
	kal_uint8 i = 0;
	kal_uint8 retry = 2;
	kal_uint16 sensor_id = 0;

	LOG_INF("s5khm6-aac open +\n");
	while (imgsensor_info.i2c_addr_table[i] != 0xff) {
		spin_lock(&imgsensor_drv_lock);
		imgsensor.i2c_write_id = imgsensor_info.i2c_addr_table[i];
		spin_unlock(&imgsensor_drv_lock);
		do {
			sensor_id = read_cmos_sensor(0x0000)+1;
			if (sensor_id == imgsensor_info.sensor_id) {
				LOG_INF("s5khm6-aac i2c write id: 0x%x, sensor id: 0x%x\n", imgsensor.i2c_write_id, sensor_id);
				break;
			}
			LOG_INF("s5khm6-aac Read sensor id fail, sensor id: 0x%x, i2c: 0x%x\n", sensor_id, imgsensor.i2c_write_id);
			retry--;
		} while (retry > 0);
		i++;
		if (sensor_id == imgsensor_info.sensor_id)
			break;
		retry = 2;
	}
	if (imgsensor_info.sensor_id != sensor_id)
		return ERROR_SENSOR_CONNECT_FAIL;

	/* initail sequence write in  */
	sensor_init();

	spin_lock(&imgsensor_drv_lock);

	imgsensor.autoflicker_en = KAL_FALSE;
	imgsensor.sensor_mode = IMGSENSOR_MODE_INIT;
	imgsensor.shutter = 0x3D0;
	imgsensor.gain = 0x100;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.dummy_pixel = 0;
	imgsensor.dummy_line = 0;
	imgsensor.ihdr_mode = 0;
	imgsensor.test_pattern = KAL_FALSE;
	imgsensor.current_fps = imgsensor_info.pre.max_framerate;
	spin_unlock(&imgsensor_drv_lock);
	LOG_INF("-\n");

	return ERROR_NONE;
} /* open */

/*************************************************************************
 * FUNCTION
 *	close
 *
 * DESCRIPTION
 *
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 close(void)
{
	LOG_INF("E\n");
	/* No Need to implement this function */
	streaming_control(KAL_FALSE);
	return ERROR_NONE;
} /* close */

/*************************************************************************
 * FUNCTION
 * preview
 *
 * DESCRIPTION
 *	This function start the sensor preview.
 *
 * PARAMETERS
 *	*image_window : address pointer of pixel numbers in one period of HSYNC
 *  *sensor_config_data : address pointer of line numbers in one period of VSYNC
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_PREVIEW;
	imgsensor.pclk = imgsensor_info.pre.pclk;
	imgsensor.line_length = imgsensor_info.pre.linelength;
	imgsensor.frame_length = imgsensor_info.pre.framelength;
	imgsensor.min_frame_length = imgsensor_info.pre.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);

	preview_setting();

	return ERROR_NONE;
} /* preview */

/*************************************************************************
 * FUNCTION
 *	capture
 *
 * DESCRIPTION
 *	This function setup the CMOS sensor in capture MY_OUTPUT mode
 *
 * PARAMETERS
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
static kal_uint32 capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CAPTURE;

	if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
		LOG_INF(
			"Warning: current_fps %d fps is not support, so use cap's setting: %d fps!\n",
			imgsensor.current_fps,
			imgsensor_info.cap.max_framerate / 10);
	imgsensor.pclk = imgsensor_info.cap.pclk;
	imgsensor.line_length = imgsensor_info.cap.linelength;
	imgsensor.frame_length = imgsensor_info.cap.framelength;
	imgsensor.min_frame_length = imgsensor_info.cap.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;

	spin_unlock(&imgsensor_drv_lock);
	capture_setting(imgsensor.current_fps);
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/* capture() */
static kal_uint32 normal_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_VIDEO;
	imgsensor.pclk = imgsensor_info.normal_video.pclk;
	imgsensor.line_length = imgsensor_info.normal_video.linelength;
	imgsensor.frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.normal_video.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	normal_video_setting(imgsensor.current_fps);
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/*	normal_video   */

static kal_uint32 hs_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_HIGH_SPEED_VIDEO;
	imgsensor.pclk = imgsensor_info.hs_video.pclk;
	/*imgsensor.video_mode = KAL_TRUE;*/
	imgsensor.line_length = imgsensor_info.hs_video.linelength;
	imgsensor.frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.hs_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	/*imgsensor.current_fps = 300;*/
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	hs_video_setting();
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/*	hs_video   */

static kal_uint32 slim_video(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
				MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_SLIM_VIDEO;
	imgsensor.pclk = imgsensor_info.slim_video.pclk;
	/*imgsensor.video_mode = KAL_TRUE;*/
	imgsensor.line_length = imgsensor_info.slim_video.linelength;
	imgsensor.frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.min_frame_length = imgsensor_info.slim_video.framelength;
	imgsensor.dummy_line = 0;
	imgsensor.dummy_pixel = 0;
	/*imgsensor.current_fps = 300;*/
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	slim_video_setting();
	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/* slim_video */


static kal_uint32 custom1(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM1;
	imgsensor.pclk = imgsensor_info.custom1.pclk;
	imgsensor.line_length = imgsensor_info.custom1.linelength;
	imgsensor.frame_length = imgsensor_info.custom1.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom1.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom1_setting();

	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
}	/* custom1 */

//N17 code for HQ-293325 by jixinji at 2023/05/11 start
static kal_uint32 custom2(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM2;
	imgsensor.pclk = imgsensor_info.custom2.pclk;
	imgsensor.line_length = imgsensor_info.custom2.linelength;
	imgsensor.frame_length = imgsensor_info.custom2.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom2.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom2_setting();

	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
} /* custom2*/
//N17 code for HQ-293325 by jixinji at 2023/05/11 end

//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
static kal_uint32 custom3(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	spin_lock(&imgsensor_drv_lock);
	imgsensor.sensor_mode = IMGSENSOR_MODE_CUSTOM3;
	imgsensor.pclk = imgsensor_info.custom3.pclk;
	imgsensor.line_length = imgsensor_info.custom3.linelength;
	imgsensor.frame_length = imgsensor_info.custom3.framelength;
	imgsensor.min_frame_length = imgsensor_info.custom3.framelength;
	imgsensor.autoflicker_en = KAL_FALSE;
	spin_unlock(&imgsensor_drv_lock);
	custom3_setting();

	set_mirror_flip(imgsensor.mirror);

	return ERROR_NONE;
} /* custom3*/
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end

static kal_uint32
get_resolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution)
{
	LOG_INF("E\n");
	sensor_resolution->SensorFullWidth =
		imgsensor_info.cap.grabwindow_width;
	sensor_resolution->SensorFullHeight =
		imgsensor_info.cap.grabwindow_height;

	sensor_resolution->SensorPreviewWidth =
		imgsensor_info.pre.grabwindow_width;
	sensor_resolution->SensorPreviewHeight =
		imgsensor_info.pre.grabwindow_height;

	sensor_resolution->SensorVideoWidth =
		imgsensor_info.normal_video.grabwindow_width;
	sensor_resolution->SensorVideoHeight =
		imgsensor_info.normal_video.grabwindow_height;

	sensor_resolution->SensorHighSpeedVideoWidth =
		imgsensor_info.hs_video.grabwindow_width;
	sensor_resolution->SensorHighSpeedVideoHeight =
		imgsensor_info.hs_video.grabwindow_height;

	sensor_resolution->SensorSlimVideoWidth =
		imgsensor_info.slim_video.grabwindow_width;
	sensor_resolution->SensorSlimVideoHeight =
		imgsensor_info.slim_video.grabwindow_height;

	sensor_resolution->SensorCustom1Width =
		imgsensor_info.custom1.grabwindow_width;
	sensor_resolution->SensorCustom1Height =
		imgsensor_info.custom1.grabwindow_height;

//N17 code for HQ-293325 by jixinji at 2023/05/11 start
	sensor_resolution->SensorCustom2Width =
		imgsensor_info.custom2.grabwindow_width;
	sensor_resolution->SensorCustom2Height =
		imgsensor_info.custom2.grabwindow_height;
//N17 code for HQ-293325 by jixinji at 2023/05/11 end

//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
	sensor_resolution->SensorCustom3Width =
		imgsensor_info.custom3.grabwindow_width;
	sensor_resolution->SensorCustom3Height =
		imgsensor_info.custom3.grabwindow_height;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
	return ERROR_NONE;
} /* get_resolution */

static kal_uint32 get_info(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			   MSDK_SENSOR_INFO_STRUCT *sensor_info,
			   MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4; /* not use */
	sensor_info->SensorResetActiveHigh = FALSE; /* not use */
	sensor_info->SensorResetDelayCount = 5; /* not use */

	sensor_info->SensroInterfaceType = imgsensor_info.sensor_interface_type;
	sensor_info->MIPIsensorType = imgsensor_info.mipi_sensor_type;
	sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat = imgsensor_info.sensor_output_dataformat;

	sensor_info->CaptureDelayFrame = imgsensor_info.cap_delay_frame;
	sensor_info->PreviewDelayFrame = imgsensor_info.pre_delay_frame;
	sensor_info->VideoDelayFrame = imgsensor_info.video_delay_frame;
	sensor_info->HighSpeedVideoDelayFrame = imgsensor_info.hs_video_delay_frame;
	sensor_info->SlimVideoDelayFrame = imgsensor_info.slim_video_delay_frame;
	sensor_info->Custom1DelayFrame = imgsensor_info.custom1_delay_frame;
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
	sensor_info->Custom2DelayFrame = imgsensor_info.custom2_delay_frame;
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
	sensor_info->Custom3DelayFrame = imgsensor_info.custom3_delay_frame;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 send
	sensor_info->SensorMasterClockSwitch = 0; /* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info.isp_driving_current;

	sensor_info->AEShutDelayFrame = imgsensor_info.ae_shut_delay_frame;
	sensor_info->AESensorGainDelayFrame = imgsensor_info.ae_sensor_gain_delay_frame;
	sensor_info->AEISPGainDelayFrame = imgsensor_info.ae_ispGain_delay_frame;
	sensor_info->IHDR_Support = imgsensor_info.ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info.ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info.sensor_mode_num;
	sensor_info->SensorMIPILaneNumber = imgsensor_info.mipi_lane_num;
	sensor_info->TEMPERATURE_SUPPORT = imgsensor_info.temperature_support;
	sensor_info->SensorClockFreq = imgsensor_info.mclk;
	sensor_info->SensorClockDividCount = 3; /* not use */
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2; /* not use */
	sensor_info->SensorPixelClockCount = 3; /* not use */
	sensor_info->SensorDataLatchCount = 2; /* not use */

	sensor_info->MIPIDataLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	sensor_info->SensorWidthSampling = 0; /* 0 is default 1x */
	sensor_info->SensorHightSampling = 0; /* 0 is default 1x */
	sensor_info->SensorPacketECCOrder = 1;

	sensor_info->FrameTimeDelayFrame = imgsensor_info.frame_time_delay_frame;

	#if S5KHM6_PDAF_SWITCH
	sensor_info->PDAF_Support = 2;
	#else
	sensor_info->PDAF_Support = 0;
	#endif

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount = imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		sensor_info->SensorGrabStartX = imgsensor_info.cap.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.cap.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =	imgsensor_info.cap.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		sensor_info->SensorGrabStartX =	imgsensor_info.normal_video.startx;
		sensor_info->SensorGrabStartY =	imgsensor_info.normal_video.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =	imgsensor_info.normal_video.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		sensor_info->SensorGrabStartX = imgsensor_info.hs_video.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.hs_video.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =	imgsensor_info.hs_video.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		sensor_info->SensorGrabStartX =	imgsensor_info.slim_video.startx;
		sensor_info->SensorGrabStartY =	imgsensor_info.slim_video.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =	imgsensor_info.slim_video.mipi_data_lp2hs_settle_dc;
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		sensor_info->SensorGrabStartX = imgsensor_info.custom1.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.custom1.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =	imgsensor_info.custom1.mipi_data_lp2hs_settle_dc;
		break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
	case MSDK_SCENARIO_ID_CUSTOM2:
		sensor_info->SensorGrabStartX = imgsensor_info.custom2.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.custom2.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =	imgsensor_info.custom2.mipi_data_lp2hs_settle_dc;
		break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
	case MSDK_SCENARIO_ID_CUSTOM3:
		sensor_info->SensorGrabStartX = imgsensor_info.custom3.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.custom3.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =	imgsensor_info.custom3.mipi_data_lp2hs_settle_dc;
		break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
	default:
		sensor_info->SensorGrabStartX = imgsensor_info.pre.startx;
		sensor_info->SensorGrabStartY = imgsensor_info.pre.starty;
		sensor_info->MIPIDataLowPwr2HighSpeedSettleDelayCount =	imgsensor_info.pre.mipi_data_lp2hs_settle_dc;
		break;
	}

	return ERROR_NONE;
}	/*	get_info  */


static kal_uint32 control(enum MSDK_SCENARIO_ID_ENUM scenario_id,
			MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
			MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	LOG_INF("scenario_id = %d\n", scenario_id);
	spin_lock(&imgsensor_drv_lock);
	imgsensor.current_scenario_id = scenario_id;
	spin_unlock(&imgsensor_drv_lock);
	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		preview(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		capture(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		normal_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		hs_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		slim_video(image_window, sensor_config_data);
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		custom1(image_window, sensor_config_data);
		break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
	case MSDK_SCENARIO_ID_CUSTOM2:
		custom2(image_window, sensor_config_data);
		break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
	case MSDK_SCENARIO_ID_CUSTOM3:
		custom3(image_window, sensor_config_data);
		break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
	default:
		LOG_INF("Error ScenarioId setting");
		preview(image_window, sensor_config_data);
		return ERROR_INVALID_SCENARIO_ID;
	}

	return ERROR_NONE;
}	/* control() */



static kal_uint32 set_video_mode(UINT16 framerate)
{
	LOG_INF("framerate = %d\n ", framerate);
	/* SetVideoMode Function should fix framerate */
	if (framerate == 0)
		/* Dynamic frame rate */
		return ERROR_NONE;
	spin_lock(&imgsensor_drv_lock);
	if ((framerate == 300) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 296;
	else if ((framerate == 150) && (imgsensor.autoflicker_en == KAL_TRUE))
		imgsensor.current_fps = 146;
	else
		imgsensor.current_fps = framerate;
	spin_unlock(&imgsensor_drv_lock);
	set_max_framerate(imgsensor.current_fps, 1);

	return ERROR_NONE;
}

static kal_uint32 set_auto_flicker_mode(kal_bool enable, UINT16 framerate)
{
	spin_lock(&imgsensor_drv_lock);
	if (enable) /*enable auto flicker*/ {
		imgsensor.autoflicker_en = KAL_TRUE;
		LOG_INF("enable! fps = %d", framerate);
	} else {
		 /*Cancel Auto flick*/
		imgsensor.autoflicker_en = KAL_FALSE;
	}
	spin_unlock(&imgsensor_drv_lock);

	return ERROR_NONE;
}


static kal_uint32 set_max_framerate_by_scenario(
		enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 framerate)
{
	kal_uint32 frame_length;

	LOG_INF("scenario_id = %d, framerate = %d\n", scenario_id, framerate);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		frame_length = imgsensor_info.pre.pclk / framerate * 10
				/ imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.pre.framelength)
		? (frame_length - imgsensor_info.pre.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.pre.framelength
			+ imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		if (framerate == 0)
			return ERROR_NONE;
		frame_length = imgsensor_info.normal_video.pclk /
				framerate * 10 /
				imgsensor_info.normal_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.normal_video.framelength)
		? (frame_length - imgsensor_info.normal_video.framelength)
		: 0;
		imgsensor.frame_length =
			imgsensor_info.normal_video.framelength
			+ imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
	if (imgsensor.current_fps != imgsensor_info.cap.max_framerate)
		LOG_INF("Warning: current_fps %d fps is not support, so use cap's setting: %d fps!\n", framerate, imgsensor_info.cap.max_framerate/10);
		frame_length = imgsensor_info.cap.pclk / framerate * 10
				/ imgsensor_info.cap.linelength;
		spin_lock(&imgsensor_drv_lock);
			imgsensor.dummy_line =
			(frame_length > imgsensor_info.cap.framelength)
			  ? (frame_length - imgsensor_info.cap.framelength) : 0;
			imgsensor.frame_length =
				imgsensor_info.cap.framelength
				+ imgsensor.dummy_line;
			imgsensor.min_frame_length = imgsensor.frame_length;
			spin_unlock(&imgsensor_drv_lock);

		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		frame_length = imgsensor_info.hs_video.pclk / framerate * 10
				/ imgsensor_info.hs_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.hs_video.framelength)
			  ? (frame_length - imgsensor_info.hs_video.framelength)
			  : 0;
		imgsensor.frame_length =
			imgsensor_info.hs_video.framelength
				+ imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		frame_length = imgsensor_info.slim_video.pclk / framerate * 10
			/ imgsensor_info.slim_video.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.slim_video.framelength)
			? (frame_length - imgsensor_info.slim_video.framelength)
			: 0;
		imgsensor.frame_length =
			imgsensor_info.slim_video.framelength
			+ imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		frame_length = imgsensor_info.custom1.pclk / framerate * 10
				/ imgsensor_info.custom1.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom1.framelength)
			? (frame_length - imgsensor_info.custom1.framelength)
			: 0;
		imgsensor.frame_length =
			imgsensor_info.custom1.framelength
			+ imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
		case MSDK_SCENARIO_ID_CUSTOM2:
		frame_length = imgsensor_info.custom2.pclk / framerate * 10
				/ imgsensor_info.custom2.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom2.framelength)
			? (frame_length - imgsensor_info.custom2.framelength)
			: 0;
		imgsensor.frame_length =
			imgsensor_info.custom2.framelength
			+ imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
		case MSDK_SCENARIO_ID_CUSTOM3:
		frame_length = imgsensor_info.custom3.pclk / framerate * 10
				/ imgsensor_info.custom3.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.custom3.framelength)
			? (frame_length - imgsensor_info.custom3.framelength)
			: 0;
		imgsensor.frame_length =
			imgsensor_info.custom3.framelength
			+ imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
	default:  /*coding with  preview scenario by default*/
		frame_length = imgsensor_info.pre.pclk / framerate * 10
			/ imgsensor_info.pre.linelength;
		spin_lock(&imgsensor_drv_lock);
		imgsensor.dummy_line =
			(frame_length > imgsensor_info.pre.framelength)
			? (frame_length - imgsensor_info.pre.framelength) : 0;
		imgsensor.frame_length =
			imgsensor_info.pre.framelength + imgsensor.dummy_line;
		imgsensor.min_frame_length = imgsensor.frame_length;
		spin_unlock(&imgsensor_drv_lock);
		if (imgsensor.frame_length > imgsensor.shutter)
			set_dummy();
		LOG_INF("error scenario_id = %d, we use preview scenario\n", scenario_id);
		break;
	}
	return ERROR_NONE;
}


static kal_uint32 get_default_framerate_by_scenario(
		enum MSDK_SCENARIO_ID_ENUM scenario_id, MUINT32 *framerate)
{
	LOG_INF("scenario_id = %d\n", scenario_id);

	switch (scenario_id) {
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		*framerate = imgsensor_info.pre.max_framerate;
		break;
	case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		*framerate = imgsensor_info.normal_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		*framerate = imgsensor_info.cap.max_framerate;
		break;
	case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		*framerate = imgsensor_info.hs_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_SLIM_VIDEO:
		*framerate = imgsensor_info.slim_video.max_framerate;
		break;
	case MSDK_SCENARIO_ID_CUSTOM1:
		*framerate = imgsensor_info.custom1.max_framerate;
		break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
	case MSDK_SCENARIO_ID_CUSTOM2:
		*framerate = imgsensor_info.custom2.max_framerate;
		break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
	case MSDK_SCENARIO_ID_CUSTOM3:
		*framerate = imgsensor_info.custom3.max_framerate;
		break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
	default:
		break;
	}

	return ERROR_NONE;
}

static kal_uint32 set_test_pattern_mode(kal_bool enable)
{
	LOG_INF("enable: %d\n", enable);
//N17 code for HQ-300879 by wuzhenyue at 2023/06/26 start
	if (enable){
		write_cmos_sensor(0x0600, 0x0001); /*solid color*/
		write_cmos_sensor(0x0602, 0x0000);
		write_cmos_sensor(0x0604, 0x0000);
		write_cmos_sensor(0x0606, 0x0000);
		write_cmos_sensor(0x0608, 0x0000);
	}
	else{
		write_cmos_sensor(0x0600, 0x0000); /*No pattern*/
	}
//N17 code for HQ-300879 by wuzhenyue at 2023/06/26 end
	spin_lock(&imgsensor_drv_lock);
	imgsensor.test_pattern = enable;
	spin_unlock(&imgsensor_drv_lock);
	return ERROR_NONE;
}

static kal_uint32 get_sensor_temperature(void)
{
//N17 code for HQ-293325 by wangqiang start
	INT32 ret;
	INT32 temperature;
	INT32 temperature_convert;

	ret = read_cmos_sensor(0x000F);
	temperature = read_cmos_sensor(0x0020);

/*Sensor output range is -128~128*/
/*Sensor output is guaranteed for 20~80*/
/*if temperature < 20, set 20; if temperature > 80 set 80; else set the real temperature*/
	if (temperature < 0x1400) //0 - 20度
		temperature_convert = 20;
	else if (temperature >= 0x1400 && temperature <= 0x5188) //20度-80度
		temperature_convert = temperature/256;
	else if (temperature > 0x5188 && temperature <= 0x7FFF) //80度-128度
		temperature_convert = 80;
	else//0x800-0XFF00:(-128)度 - (-1)度
		temperature_convert = 20;

	LOG_INF("ret = %x temperature_convert(%d), read_reg(%x)\n", ret, temperature_convert, temperature);
//N17 code for HQ-293325 by wangqiang end
	return temperature_convert;
}

static kal_uint32 feature_control(MSDK_SENSOR_FEATURE_ENUM feature_id,
				 UINT8 *feature_para, UINT32 *feature_para_len)
{
	UINT16 *feature_return_para_16 = (UINT16 *) feature_para;
	UINT16 *feature_data_16 = (UINT16 *) feature_para;
	UINT32 *feature_return_para_32 = (UINT32 *) feature_para;
	UINT32 *feature_data_32 = (UINT32 *) feature_para;
	unsigned long long *feature_data = (unsigned long long *) feature_para;
	/* unsigned long long *feature_return_para
	 *  = (unsigned long long *) feature_para;
	 */
	struct SET_PD_BLOCK_INFO_T *PDAFinfo;
	struct SENSOR_WINSIZE_INFO_STRUCT *wininfo;
	struct SENSOR_VC_INFO_STRUCT *pvcinfo;
//N17 code for HQ-293325 by wangqiang at 2023/05/29 start
	struct SET_SENSOR_AWB_GAIN *pSetSensorAWB =
		(struct SET_SENSOR_AWB_GAIN *) feature_para;
//N17 code for HQ-293325 by wangqiang at 2023/05/29 end

	MSDK_SENSOR_REG_INFO_STRUCT *sensor_reg_data
		= (MSDK_SENSOR_REG_INFO_STRUCT *) feature_para;

	/*LOG_INF("feature_id = %d\n", feature_id);*/
	switch (feature_id) {
	case SENSOR_FEATURE_GET_AWB_REQ_BY_SCENARIO:
		switch (*feature_data) {
/* N17 code for HQ-HQ-308731 by wangqiang at 2023/07/25 start */
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
/* N17 code for HQ-HQ-308731 by wangqiang at 2023/07/25 end */
//N17 code for HQ-293325 by wangqiang at 2023/05/29 start
		case MSDK_SCENARIO_ID_CUSTOM2:
//N17 code for HQ-293325 by wangqiang at 2023/05/29 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
		case MSDK_SCENARIO_ID_CUSTOM3:
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 1;
			break;
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 0;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_GAIN_RANGE_BY_SCENARIO:
		*(feature_data + 1) = imgsensor_info.min_gain;
		*(feature_data + 2) = imgsensor_info.max_gain;
		break;
	case SENSOR_FEATURE_GET_BASE_GAIN_ISO_AND_STEP:
		*(feature_data + 0) = imgsensor_info.min_gain_iso;
		*(feature_data + 1) = imgsensor_info.gain_step;
		*(feature_data + 2) = imgsensor_info.gain_type;
		break;
	case SENSOR_FEATURE_GET_MIN_SHUTTER_BY_SCENARIO:
		*(feature_data + 1) = imgsensor_info.min_shutter;
		*(feature_data + 2) = imgsensor_info.exp_step;
		break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.cap.pclk;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.normal_video.pclk;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.hs_video.pclk;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.slim_video.pclk;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom1.pclk;
			break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
		case MSDK_SCENARIO_ID_CUSTOM2:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom2.pclk;
			break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom3.pclk;
			break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.pre.pclk;
			break;
		}
		break;
	case SENSOR_FEATURE_GET_PERIOD_BY_SCENARIO:
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.cap.framelength << 16)
				+ imgsensor_info.cap.linelength;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.normal_video.framelength << 16)
				+ imgsensor_info.normal_video.linelength;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.hs_video.framelength << 16)
				+ imgsensor_info.hs_video.linelength;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.slim_video.framelength << 16)
				+ imgsensor_info.slim_video.linelength;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.custom1.framelength << 16)
				+ imgsensor_info.custom1.linelength;
			break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
		case MSDK_SCENARIO_ID_CUSTOM2:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.custom2.framelength << 16)
				+ imgsensor_info.custom2.linelength;
			break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.custom3.framelength << 16)
				+ imgsensor_info.custom3.linelength;
			break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
			= (imgsensor_info.pre.framelength << 16)
				+ imgsensor_info.pre.linelength;
			break;
		}
		break;
	//N17 code for HQ-306773 by wuzhenyue at 2023/07/11 start
	case SENSOR_FEATURE_GET_OFFSET_TO_START_OF_EXPOSURE: 
		*(MUINT32 *)(uintptr_t)(*(feature_data + 1)) = 1387000;
		break;
	//N17 code for HQ-306773 by wuzhenyue at 2023/07/11 end
	case SENSOR_FEATURE_GET_PERIOD:
		*feature_return_para_16++ = imgsensor.line_length;
		*feature_return_para_16 = imgsensor.frame_length;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
		*feature_return_para_32 = imgsensor.pclk;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_ESHUTTER:
		 set_shutter(*feature_data);
		break;
	case SENSOR_FEATURE_SET_NIGHTMODE:
		 /* night_mode((BOOL) *feature_data); */
		break;
	#ifdef VENDOR_EDIT
	case SENSOR_FEATURE_CHECK_MODULE_ID:
		*feature_return_para_32 = imgsensor_info.module_id;
		break;
	#endif
	case SENSOR_FEATURE_SET_GAIN:
		set_gain((UINT16) *feature_data);
		break;
	case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
		break;
	case SENSOR_FEATURE_SET_REGISTER:
		write_cmos_sensor_8(sensor_reg_data->RegAddr,
				    sensor_reg_data->RegData);
		break;
	case SENSOR_FEATURE_GET_REGISTER:
		sensor_reg_data->RegData =
			read_cmos_sensor_8(sensor_reg_data->RegAddr);
		break;
	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
		/*get the lens driver ID from EEPROM
		 * or just return LENS_DRIVER_ID_DO_NOT_CARE
		 * if EEPROM does not exist in camera module.
		 */
		*feature_return_para_32 = LENS_DRIVER_ID_DO_NOT_CARE;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_VIDEO_MODE:
		set_video_mode(*feature_data);
		break;
	case SENSOR_FEATURE_CHECK_SENSOR_ID:
		get_imgsensor_id(feature_return_para_32);
		break;
	case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
		set_auto_flicker_mode((BOOL)*feature_data_16,
				      *(feature_data_16+1));
		break;
	case SENSOR_FEATURE_GET_TEMPERATURE_VALUE:
		*feature_return_para_32 = get_sensor_temperature();
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
		 set_max_framerate_by_scenario(
				(enum MSDK_SCENARIO_ID_ENUM)*feature_data,
				*(feature_data+1));
		break;
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
		 get_default_framerate_by_scenario(
				(enum MSDK_SCENARIO_ID_ENUM)*(feature_data),
				(MUINT32 *)(uintptr_t)(*(feature_data+1)));
		break;
	case SENSOR_FEATURE_GET_PDAF_DATA:
		LOG_INF("SENSOR_FEATURE_GET_PDAF_DATA\n");
		break;
	case SENSOR_FEATURE_SET_TEST_PATTERN:
		set_test_pattern_mode((BOOL)*feature_data);
		break;
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
		/* for factory mode auto testing */
		*feature_return_para_32 = imgsensor_info.checksum_value;
		*feature_para_len = 4;
		break;
	case SENSOR_FEATURE_SET_FRAMERATE:
		LOG_INF("current fps :%d\n", (UINT32)*feature_data_32);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.current_fps = *feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_SET_HDR:
		LOG_INF("ihdr enable :%d\n", (BOOL)*feature_data_32);
		spin_lock(&imgsensor_drv_lock);
		imgsensor.ihdr_mode = *feature_data_32;
		spin_unlock(&imgsensor_drv_lock);
		break;
	case SENSOR_FEATURE_GET_CROP_INFO:
	#if 0
		LOG_INF("SENSOR_FEATURE_GET_CROP_INFO scenarioId:%d\n",
			(UINT32)*feature_data);
	#endif
		wininfo =
	(struct SENSOR_WINSIZE_INFO_STRUCT *)(uintptr_t)(*(feature_data+1));

		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			memcpy((void *)wininfo,
				(void *)&imgsensor_winsize_info[1],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)wininfo,
				(void *)&imgsensor_winsize_info[2],
				sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			memcpy((void *)wininfo,
			(void *)&imgsensor_winsize_info[3],
			sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			memcpy((void *)wininfo,
			(void *)&imgsensor_winsize_info[4],
			sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			memcpy((void *)wininfo,
			(void *)&imgsensor_winsize_info[5],
			sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
		case MSDK_SCENARIO_ID_CUSTOM2:
			memcpy((void *)wininfo,
			(void *)&imgsensor_winsize_info[6],
			sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
		case MSDK_SCENARIO_ID_CUSTOM3:
			memcpy((void *)wininfo,
			(void *)&imgsensor_winsize_info[7],
			sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			memcpy((void *)wininfo,
			(void *)&imgsensor_winsize_info[0],
			sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
			break;
		}
		break;
//N17 code for HQ-293325 by jixinji at 2023/05/10 start
	case SENSOR_FEATURE_GET_PDAF_INFO:
		LOG_INF("SENSOR_FEATURE_GET_PDAF_INFO scenarioId:%d\n",
			(UINT16) *feature_data);
		PDAFinfo =
		  (struct SET_PD_BLOCK_INFO_T *)(uintptr_t)(*(feature_data+1));
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		case MSDK_SCENARIO_ID_CUSTOM1:
			memcpy((void *)PDAFinfo, (void *)&imgsensor_pd_info_binning, sizeof(struct SET_PD_BLOCK_INFO_T));
			break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
		case MSDK_SCENARIO_ID_CUSTOM3:
			memcpy((void *)PDAFinfo, (void *)&imgsensor_pd_info_isz, sizeof(struct SET_PD_BLOCK_INFO_T));
			break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:  //4000*2252
			memcpy((void *)PDAFinfo, (void *)&imgsensor_pd_info_binning_video, sizeof(struct SET_PD_BLOCK_INFO_T));
			break;
		default:
			break;
		}
		break;
	case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
		LOG_INF("SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY scenarioId:%d\n", (UINT16) *feature_data);
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
                case MSDK_SCENARIO_ID_CUSTOM1:
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
		case MSDK_SCENARIO_ID_CUSTOM3:
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 1;
			break;
/* N17 code for HQ-HQ-308731 by wangqiang at 2023/07/25 start */
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
/* N17 code for HQ-HQ-308731 by wangqiang at 2023/07/25 end */
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
		case MSDK_SCENARIO_ID_CUSTOM2:
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data+1)) = 0;
			break;
		}
		break;
//N17 code for HQ-293325 by jixinji at 2023/05/10 end
	case SENSOR_FEATURE_GET_PDAF_REG_SETTING:
		LOG_INF("SENSOR_FEATURE_GET_PDAF_REG_SETTING %d", (*feature_para_len));
		s5khm6_get_pdaf_reg_setting((*feature_para_len) / sizeof(UINT32), feature_data_16);
		break;
	case SENSOR_FEATURE_SET_PDAF_REG_SETTING:
		LOG_INF("SENSOR_FEATURE_SET_PDAF_REG_SETTING %d", (*feature_para_len));
		s5khm6_set_pdaf_reg_setting((*feature_para_len) / sizeof(UINT32) , feature_data_16);
		break;
	case SENSOR_FEATURE_SET_PDAF:
		LOG_INF("PDAF mode :%d\n", *feature_data_16);
		imgsensor.pdaf_mode = *feature_data_16;
		break;
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
		LOG_INF("SENSOR_SET_SENSOR_IHDR LE=%d, SE=%d, Gain=%d\n", (UINT16)*feature_data, (UINT16)*(feature_data+1),	(UINT16)*(feature_data+2));
		break;
	case SENSOR_FEATURE_SET_SHUTTER_FRAME_TIME:
		set_shutter_frame_length((UINT16) (*feature_data), (UINT16) (*(feature_data + 1)), (BOOL) (*(feature_data + 2)));
		break;
	case SENSOR_FEATURE_GET_FRAME_CTRL_INFO_BY_SCENARIO:
		/*
		 * 1, if driver support new sw frame sync
		 * set_shutter_frame_length() support third para auto_extend_en
		 */
		*(feature_data + 1) = 1;
		/* margin info by scenario */
		*(feature_data + 2) = imgsensor_info.margin;
		break;
	case SENSOR_FEATURE_SET_HDR_SHUTTER:
		LOG_INF("SENSOR_FEATURE_SET_HDR_SHUTTER LE=%d, SE=%d\n", (UINT16)*feature_data, (UINT16)*(feature_data+1));
		break;
	case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_SUSPEND\n");
		streaming_control(KAL_FALSE);
		break;
	case SENSOR_FEATURE_SET_STREAMING_RESUME:
		LOG_INF("SENSOR_FEATURE_SET_STREAMING_RESUME, shutter:%llu\n", *feature_data);
		if (*feature_data != 0)
			set_shutter(*feature_data);
		streaming_control(KAL_TRUE);
		break;
	case SENSOR_FEATURE_GET_BINNING_TYPE:
		switch (*(feature_data + 1)) {
//N17 code for HQ-293325 by wangqiang at 2023/05/29 start
/* N17 code for HQ-HQ-308731 by wangqiang at 2023/07/25 start */
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
/* N17 code for HQ-HQ-308731 by wangqiang at 2023/07/25 end */
		case MSDK_SCENARIO_ID_CUSTOM2:
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
		case MSDK_SCENARIO_ID_CUSTOM3:
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
			*feature_return_para_32 = 1000; //remosaic full size
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_CUSTOM1:
		default:
			*feature_return_para_32 = 1250; /*BINNING_AVERAGED*/
			break;
		}
//N17 code for HQ-293325 by wangqiang at 2023/05/29 end
		LOG_INF("SENSOR_FEATURE_GET_BINNING_TYPE AE_binning_type:%d,\n",
			*feature_return_para_32);
		*feature_para_len = 4;

		break;
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
	{
		switch (*feature_data) {
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.cap.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.normal_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.hs_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.slim_video.mipi_pixel_rate;
			break;
		case MSDK_SCENARIO_ID_CUSTOM1:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom1.mipi_pixel_rate;
			break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 start
		case MSDK_SCENARIO_ID_CUSTOM2:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom2.mipi_pixel_rate;
			break;
//N17 code for HQ-293325 by jixinji at 2023/05/11 end
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
		case MSDK_SCENARIO_ID_CUSTOM3:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.custom3.mipi_pixel_rate;
			break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		default:
			*(MUINT32 *)(uintptr_t)(*(feature_data + 1))
				= imgsensor_info.pre.mipi_pixel_rate;
			break;
		}
	}
		break;
//N17 code for HQ-293325 by jixinji at 2023/05/10 start
	case SENSOR_FEATURE_GET_VC_INFO:
		pvcinfo =
		 (struct SENSOR_VC_INFO_STRUCT *)(uintptr_t)(*(feature_data+1));
		switch (*feature_data_32) {
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_SLIM_VIDEO:
		case MSDK_SCENARIO_ID_CUSTOM1:
//N17 code for HQ-293325 by wangqiang at 2023/05/29 start
		case MSDK_SCENARIO_ID_CUSTOM2:
//N17 code for HQ-293325 by wangqiang at 2023/05/29 end
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[0],	sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
			case MSDK_SCENARIO_ID_CUSTOM3:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[5],	sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
		case MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[4],	sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
			memcpy((void *)pvcinfo, (void *)&SENSOR_VC_INFO[1],	sizeof(struct SENSOR_VC_INFO_STRUCT));
			break;
//N17 code for HQ-293325 by jixinji at 2023/05/10 end
		default:
			LOG_INF("error: get wrong vc_INFO id = %d",	*feature_data_32);
			break;
		}
	break;
	case SENSOR_FEATURE_GET_AE_EFFECTIVE_FRAME_FOR_LE:
		*feature_return_para_32 = imgsensor.current_ae_effective_frame;
		LOG_INF("GET AE EFFECTIVE %d\n", *feature_return_para_32);
		break;
	case SENSOR_FEATURE_GET_AE_FRAME_MODE_FOR_LE:
		memcpy(feature_return_para_32, &imgsensor.ae_frm_mode, sizeof(struct IMGSENSOR_AE_FRM_MODE));
		LOG_INF("GET_AE_FRAME_MODE");
	case SENSOR_FEATURE_SET_AWB_GAIN:
		/* modify to separate 3hdr and remosaic */
//N17 code for HQ-293325 by wangqiang at 2023/05/29 start
//N17 code for HQ-310269 by wangqiang at 2023/08/02 start
		if ( (imgsensor.sensor_mode == IMGSENSOR_MODE_CUSTOM2) ||
                     (imgsensor.sensor_mode == IMGSENSOR_MODE_CUSTOM3) ) {
//N17 code for HQ-310269 by wangqiang at 2023/08/02 end
			/*write AWB gain to sensor*/
			feedback_awbgain(pSetSensorAWB);
//N17 code for HQ-293325 by wangqiang at 2023/05/29 end
		} else {
			s5khm6_awb_gain((struct SET_SENSOR_AWB_GAIN *) feature_para);
		}
		break;
	case SENSOR_FEATURE_SET_LSC_TBL:
		LOG_INF("SENSOR_FEATURE_SET_LSC_TBL");
		break;
	default:
		break;
	}

	return ERROR_NONE;
} /* feature_control() */

static struct SENSOR_FUNCTION_STRUCT sensor_func = {
	open,
	get_info,
	get_resolution,
	feature_control,
	control,
	close
};

UINT32 S5KHM6_AAC_MAIN_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc != NULL)
		*pfFunc = &sensor_func;
	return ERROR_NONE;
} /* S5KHM6_MIPI_RAW_SensorInit */
