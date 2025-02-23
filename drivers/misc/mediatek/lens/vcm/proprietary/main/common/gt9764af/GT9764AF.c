// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */


/*
 * GT9764AF voice coil motor driver
 *
 *
 */

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/time.h>
#include <linux/uaccess.h>

#include "lens_info.h"

#define AF_DRVNAME "GT9764AF_DRV"
#define AF_I2C_SLAVE_ADDR 0x18

#define AF_DEBUG
#ifdef AF_DEBUG
#define LOG_INF(format, args...)                                               \
	pr_info(AF_DRVNAME " [%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif


static struct i2c_client *g_pstAF_I2Cclient;
static int *g_pAF_Opened;
static spinlock_t *g_pAF_SpinLock;

static unsigned long g_u4AF_INF;
static unsigned long g_u4AF_MACRO = 1023;
/*N17 code for HQ-296316 by jixinji at 2023.05.23 start.*/
static unsigned long g_u4TargetPosition;
/*N17 code for HQ-296316 by jixinji at 2023.05.23 end.*/
static unsigned long g_u4CurrPosition;

/*N17 code for HQ-296316 by jixinji at 2023.05.23 start.*/
static int i2c_read(u8 a_u2Addr, u8 *a_puBuff)
{
	int i4RetValue = 0;
	char puReadCmd[1] = { (char)(a_u2Addr) };
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puReadCmd, 1);
	if (i4RetValue != 1) {
		LOG_INF(" I2C write failed!!\n");
		return -1;
	}
	i4RetValue = i2c_master_recv(g_pstAF_I2Cclient, (char *)a_puBuff, 1);
	if (i4RetValue != 1) {
		LOG_INF(" I2C read failed!!\n");
		return -1;
	}
	return 0;
}
static u8 read_data(u8 addr)
{
	u8 get_byte = 0;
	i2c_read(addr, &get_byte);
	return get_byte;
}
static int s4GT9764AF_ReadReg(unsigned short *a_pu2Result)
{
	*a_pu2Result = (read_data(0x03) << 8) + (read_data(0x04) & 0xff);
	return 0;
}
static int s4AF_WriteReg(u16 a_u2Data)
{
	int i4RetValue = 0;
	char puSendCmd[3] = { 0x03, (char)(a_u2Data >> 8),
		(char)(a_u2Data & 0xFF) };
	i4RetValue = i2c_master_send(g_pstAF_I2Cclient, puSendCmd, 3);
	if (i4RetValue < 0) {
		LOG_INF("I2C send failed!!\n");
		return -1;
	}
	return 0;
}
/*N17 code for HQ-296316 by jixinji at 2023.05.23 end.*/

static inline int getAFInfo(__user struct stAF_MotorInfo *pstMotorInfo)
{
	struct stAF_MotorInfo stMotorInfo;

	stMotorInfo.u4MacroPosition = g_u4AF_MACRO;
	stMotorInfo.u4InfPosition = g_u4AF_INF;
	stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
	stMotorInfo.bIsSupportSR = 1;

	stMotorInfo.bIsMotorMoving = 1;

	if (*g_pAF_Opened >= 1)
		stMotorInfo.bIsMotorOpen = 1;
	else
		stMotorInfo.bIsMotorOpen = 0;

	if (copy_to_user(pstMotorInfo, &stMotorInfo,
			 sizeof(struct stAF_MotorInfo)))
		LOG_INF("copy to user failed when getting motor information\n");

	return 0;
}

/*N17 code for HQ-296316 by jixinji at 2023.05.23 start.*/
static int initdrv(void)
{
	int i4RetValue = 0;
	char puSendCmdArray[7][2] = {
	{0x02, 0x01}, {0x02, 0x00}, {0xFE, 0xFE},
	{0x02, 0x02}, {0x06, 0x40}, {0x07, 0x6C}, {0xFE, 0xFE},
	};
	unsigned char cmd_number;
	LOG_INF("InitDrv[1] %p, %p\n", &(puSendCmdArray[1][0]),
			puSendCmdArray[1]);
	LOG_INF("InitDrv[2] %p, %p\n", &(puSendCmdArray[2][0]),
			puSendCmdArray[2]);
	for (cmd_number = 0; cmd_number < 7; cmd_number++) {
		if (puSendCmdArray[cmd_number][0] != 0xFE) {
			i4RetValue = i2c_master_send(g_pstAF_I2Cclient,
					puSendCmdArray[cmd_number], 2);
			if (i4RetValue < 0)
				return -1;
		} else {
			udelay(2000);
		}
	}
	return i4RetValue;
}
static inline int moveAF(unsigned long a_u4Position)
{
	int ret = 0;
		LOG_INF("husfa_u4Position %d \n",a_u4Position);
	if ((a_u4Position > g_u4AF_MACRO) || (a_u4Position < g_u4AF_INF)) {
		LOG_INF("out of range\n");
		return -EINVAL;
	}
	if (*g_pAF_Opened == 1) {
		unsigned short InitPos;
		initdrv();
		ret = s4GT9764AF_ReadReg(&InitPos);
		if (ret == 0) {
			LOG_INF("Init Pos %6d\n", InitPos);
			spin_lock(g_pAF_SpinLock);
			g_u4CurrPosition = (unsigned long)InitPos;
			spin_unlock(g_pAF_SpinLock);
		} else {
			spin_lock(g_pAF_SpinLock);
			g_u4CurrPosition = 0;
			spin_unlock(g_pAF_SpinLock);
		}
/*N17 code for HQ-296581 by jixinji at 2023.05.31 start.*/
		if (a_u4Position > 750) {
			int CntStep = a_u4Position - 750;
			int Position = 750;
			int PerStep = 50;

	   		s4AF_WriteReg(750);
			spin_lock(g_pAF_SpinLock);
			g_u4CurrPosition = 750;
			spin_unlock(g_pAF_SpinLock);
			mdelay(2);
			while (CntStep > 0) {
			        PerStep = 50;
			        spin_lock(g_pAF_SpinLock);
			        g_u4TargetPosition = Position;
				spin_unlock(g_pAF_SpinLock);
			if (s4AF_WriteReg((unsigned short)g_u4TargetPosition) == 0) {
				spin_lock(g_pAF_SpinLock);
				g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
				spin_unlock(g_pAF_SpinLock);
			}
			mdelay(2);
			Position += PerStep;
			CntStep -= PerStep;
			}
		}
/*N17 code for HQ-296581 by jixinji at 2023.05.31 end.*/
		spin_lock(g_pAF_SpinLock);
		*g_pAF_Opened = 2;
		spin_unlock(g_pAF_SpinLock);
	}
//	if (g_u4CurrPosition == a_u4Position)
//		return 0;
	spin_lock(g_pAF_SpinLock);
	g_u4TargetPosition = a_u4Position;
	spin_unlock(g_pAF_SpinLock);
	if (s4AF_WriteReg((unsigned short)g_u4TargetPosition) == 0) {
		spin_lock(g_pAF_SpinLock);
		g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
		spin_unlock(g_pAF_SpinLock);
	} else {
		LOG_INF("set I2C failed when moving the motor\n");
		ret = -1;
	}
	return ret;
}
/*N17 code for HQ-296316 by jixinji at 2023.05.23 end.*/

static inline int setAFInf(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_INF = a_u4Position;
	spin_unlock(g_pAF_SpinLock);

	return 0;
}

static inline int setAFMacro(unsigned long a_u4Position)
{
	spin_lock(g_pAF_SpinLock);
	g_u4AF_MACRO = a_u4Position;
	spin_unlock(g_pAF_SpinLock);

	return 0;
}

/* ////////////////////////////////////////////////////////////// */
long GT9764AF_Ioctl(struct file *a_pstFile, unsigned int a_u4Command,
		      unsigned long a_u4Param)
{
	long i4RetValue = 0;

	switch (a_u4Command) {
	case AFIOC_G_MOTORINFO:
		i4RetValue =
			getAFInfo((__user struct stAF_MotorInfo *)(a_u4Param));
		break;

	case AFIOC_T_MOVETO:
		i4RetValue = moveAF(a_u4Param);
		break;

	case AFIOC_T_SETINFPOS:
		i4RetValue = setAFInf(a_u4Param);
		break;

	case AFIOC_T_SETMACROPOS:
		i4RetValue = setAFMacro(a_u4Param);
		break;

	default:
		LOG_INF("No CMD\n");
		i4RetValue = -EPERM;
		break;
	}

	return i4RetValue;
}

/* Main jobs: */
/* 1.Deallocate anything that "open" allocated in private_data. */
/* 2.Shut down the device on last close. */
/* 3.Only called once on last time. */
/* Q1 : Try release multiple times. */
/*N17 code for HQ-296316 by jixinji at 2023.05.23 start.*/
int GT9764AF_Release(struct inode *a_pstInode, struct file *a_pstFile)
{
	LOG_INF("Start\n");
	if (*g_pAF_Opened == 2)
	{
	    int ret = 0;
	    unsigned long af_step = 50;
	    unsigned long nextPosition = 0;
	    unsigned long endPos = 512;
	    // >512 multiple times
	    // first step move fast
	    if (g_u4CurrPosition > endPos) {
		if (g_u4CurrPosition > 800) {
		    nextPosition = 800;
		    LOG_INF("0:nextPosition = %d g_u4CurrPosition = %d g_u4AF_MACRO = %d", nextPosition, g_u4CurrPosition,g_u4AF_MACRO);
		    if (s4AF_WriteReg((unsigned short)nextPosition) == 0) {
			g_u4CurrPosition = nextPosition;
			ret = 0;
		    } else {
			LOG_INF("set I2C failed when moving the motor\n");
			ret = -1;
		    }
		}
		mdelay(3);
		while (g_u4CurrPosition > endPos + af_step) {
		    if (g_u4CurrPosition > 700) {
			af_step = 50;
		    }
		    nextPosition = g_u4CurrPosition - af_step;
		    LOG_INF("1:af_step = %d g_u4CurrPosition = %d", af_step, g_u4CurrPosition);
		    if (s4AF_WriteReg((unsigned short)nextPosition) == 0) {
			g_u4CurrPosition = nextPosition;
			ret = 0;
		    } else {
			LOG_INF("set I2C failed when moving the motor\n");
			ret = -1;
			break;
		    }
		    mdelay(3);
		}
		} else if (g_u4CurrPosition < endPos)
		    // <512 multiple times
		    // first step move fast
		    if (g_u4CurrPosition > 300) {
			nextPosition = 300;
			LOG_INF("0:nextPosition = %d g_u4CurrPosition = %d g_u4AF_MACRO = %d", nextPosition, g_u4CurrPosition,g_u4AF_MACRO);
			if (s4AF_WriteReg((unsigned short)nextPosition) == 0) {
			    g_u4CurrPosition = nextPosition;
			ret = 0;
			} else {
			    LOG_INF("set I2C failed when moving the motor\n");
			    ret = -1;
			}
		    }
		    mdelay(3);
		while (g_u4CurrPosition < endPos - af_step) {
		    if (g_u4CurrPosition > 400) {
			af_step = 50;
		    }
		    nextPosition = g_u4CurrPosition + af_step;
		    LOG_INF("1:af_step = %d g_u4CurrPosition = %d", af_step, g_u4CurrPosition);
		    if (s4AF_WriteReg((unsigned short)nextPosition) == 0) {
			g_u4CurrPosition = nextPosition;
			ret = 0;
		    } else {
			LOG_INF("set I2C failed when moving the motor\n");
			ret = -1;
			break;
		    }
		    mdelay(3);
		}
		LOG_INF("Wait\n");
	}
/*N17 code for HQ-296316 by jixinji at 2023.05.23 end.*/
	if (*g_pAF_Opened) {
		LOG_INF("Free\n");

		spin_lock(g_pAF_SpinLock);
		*g_pAF_Opened = 0;
		spin_unlock(g_pAF_SpinLock);
	}

	LOG_INF("End\n");
	return 0;
}

int GT9764AF_SetI2Cclient(struct i2c_client *pstAF_I2Cclient,
			    spinlock_t *pAF_SpinLock, int *pAF_Opened)
{
	g_pstAF_I2Cclient = pstAF_I2Cclient;
	g_pAF_SpinLock = pAF_SpinLock;
	g_pAF_Opened = pAF_Opened;
/*N17 code for HQ-296316 by jixinji at 2023.05.23 start.*/
	g_pstAF_I2Cclient->addr = AF_I2C_SLAVE_ADDR;
	g_pstAF_I2Cclient->addr = g_pstAF_I2Cclient->addr >> 1;
/*N17 code for HQ-296316 by jixinji at 2023.05.23 end.*/
	return 1;
}

int GT9764AF_GetFileName(unsigned char *pFileName)
{
	#if SUPPORT_GETTING_LENS_FOLDER_NAME
	char FilePath[256];
	char *FileString;

	sprintf(FilePath, "%s", __FILE__);
	FileString = strrchr(FilePath, '/');
	*FileString = '\0';
	FileString = (strrchr(FilePath, '/') + 1);
	strncpy(pFileName, FileString, AF_MOTOR_NAME);
	LOG_INF("FileName : %s\n", pFileName);
	#else
	pFileName[0] = '\0';
	#endif
	return 1;
}
