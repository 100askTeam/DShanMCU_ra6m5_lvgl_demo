#include "drv_gt911.h"
#include "hal_data.h"

#include "drv_touch.h"
#include <stdio.h>
#include <stdlib.h> 

static void I2C2WaitTxCplt(void);
static void I2C2WaitRxCplt(void);

static void FT5x06DrvWriteReg(uint16_t reg, uint8_t *buf, uint8_t len);
static void FT5x06DrvReadReg(uint16_t reg, uint8_t *buf, uint8_t len);
static void FT5x06DrvSoftReset(void);
static uint32_t FT5x06DrvReadProductID(void);
static uint32_t FT5x06DrvReadVendorID(void);
static void FT5x06DrvClearBuf(void);
static uint8_t FT5x06DrvReadVersion(void);
static uint8_t FT5x06DrvGetGSTID(void);
static void FT5x06DrvSetRotation(TouchDrv_t *tp, TouchRotation_t rot);
static bool FT5x06DrvIsTouched(TouchDrv_t * tp);

static void FT5x06DrvInit(struct TouchDev *ptDev);
static bool FT5x06DrvRead(struct TouchDev *ptDev, unsigned short *pX, unsigned short *pY);

static struct TouchDev gTouchDev = {
                                    .name = "FT5x06",
                                    .Init = FT5x06DrvInit,
                                    .Read = FT5x06DrvRead
};

static struct TouchDrv gTP;

static volatile bool gI2C2TxCplt = false;
static volatile bool gI2C2RxCplt = false;

struct TouchDev* TouchDevGet(void)
{
    return &gTouchDev;
}

void i2c_master2_callback(i2c_master_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case I2C_MASTER_EVENT_TX_COMPLETE:
        {
            gI2C2TxCplt = true;
            break;
        }
        case I2C_MASTER_EVENT_RX_COMPLETE:
        {
            gI2C2RxCplt = true;
            break;
        }
        default:
        {
            gI2C2TxCplt = gI2C2RxCplt = false;
            break;
        }
    }
}

static void I2C2WaitTxCplt(void)
{
    uint16_t wTimeOut = 100;
    while(!gI2C2TxCplt && wTimeOut)
    {
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
        wTimeOut--;
    }
    gI2C2TxCplt = false;
}

static void I2C2WaitRxCplt(void)
{
    uint16_t wTimeOut = 100;
    while(!gI2C2RxCplt && wTimeOut)
    {
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
        wTimeOut--;
    }
    gI2C2RxCplt = false;
}

static void FT5x06DrvWriteReg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t regl = (uint8_t)(reg & 0xff);
    uint8_t regh = (uint8_t)(reg>>8);
    uint8_t * write_package = (uint8_t*)malloc((len + 2) * sizeof(uint8_t));
    memcpy(write_package, &regh, 1);
    memcpy(write_package + 1, &regl, 1);
    memcpy(write_package + 2, buf, len);

    fsp_err_t err = g_i2c_master2.p_api->write(g_i2c_master2.p_ctrl, write_package, len + 2, 0);
    if (FSP_SUCCESS != err)
    {
        //printf("%s %d\r\n", __FUNCTION__, __LINE__);
        return;
    }
    I2C2WaitTxCplt();

    free(write_package);
}

static void FT5x06DrvReadReg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t tmpbuf[2];

    tmpbuf[0] = (uint8_t)reg;

    fsp_err_t err = g_i2c_master2.p_api->write(g_i2c_master2.p_ctrl, tmpbuf, 1, 0);
    if (FSP_SUCCESS != err)
    {
        //printf("%s %d\r\n", __FUNCTION__, __LINE__);
        return;
    }
    I2C2WaitTxCplt();

    err = g_i2c_master2.p_api->read(g_i2c_master2.p_ctrl, buf, len, 0);
    if (FSP_SUCCESS != err)
    {
        //printf("%s %d\r\n", __FUNCTION__, __LINE__);
        return;
    }
    I2C2WaitRxCplt();
}



static uint32_t FT5x06DrvReadVendorID(void)
{
    uint32_t id = 0;
    FT5x06DrvReadReg(0xa3, (uint8_t*)&id ,1);
    return id;
}




/* GT9XX可以选择2个I2C地址:0x5d 或 x14
 * rst信号从低到高变化: int信号是0则使用地址0x5d, 是1则使用地址0x14,
 * 在这之后再把int信号设置为中断引脚
 */
static void FT5x06DrvInit(struct TouchDev *ptDev)
{
    int i;
    
    if(NULL == ptDev->name) return;
   gTP.ucAddr = (uint8_t)g_i2c_master2.p_cfg->slave;
    gTP.tRotation = TP_ROT_NONE;

    /* 复位 */
    g_ioport.p_api->pinWrite(g_ioport.p_ctrl,
                             BSP_IO_PORT_04_PIN_03,
                             BSP_IO_LEVEL_LOW);
    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);

    g_ioport.p_api->pinWrite(g_ioport.p_ctrl,
                             BSP_IO_PORT_04_PIN_03,
                             BSP_IO_LEVEL_HIGH);
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);

    g_ioport.p_api->pinCfg(g_ioport.p_ctrl,
                             BSP_IO_PORT_04_PIN_08,
                             IOPORT_CFG_PORT_DIRECTION_INPUT);
    /* 初始化I2C驱动 */
    fsp_err_t err = g_i2c_master2.p_api->open(g_i2c_master2.p_ctrl, g_i2c_master2.p_cfg);
    if (FSP_SUCCESS != err)
    {
        //printf("%s %d\r\n", __FUNCTION__, __LINE__);
        return;
    }
	
	//g_i2c_master2.p_api->slaveAddressSet(g_i2c_master2.p_ctrl,0x5D, I2C_MASTER_ADDR_MODE_7BIT);

    /* 读ID */
    for (i = 0; i < 100; i++)
    {
        uint32_t nVendorID = 0;
    	nVendorID = FT5x06DrvReadVendorID();
        if((nVendorID==0x06)||(nVendorID==0x36)||(nVendorID== 0x64))
        {
            //printf("ft5x06 read vendor id %d times: 0x%.4x\r\n", i, (int)nVendorID);
		 	return;
        }
    }

    //printf("ft5x06 vendor id err\r\n");

}

//read touch point information
static bool FT5x06DrvRead(struct TouchDev *ptDev, unsigned short *pX, unsigned short *pY)
{
#define CFG_MAX_TOUCH_POINTS  5
#define CFG_POINT_READ_BUF  (3 + 6 * (CFG_MAX_TOUCH_POINTS))

    //unsigned short tmp_p;
    uint8_t buf[CFG_POINT_READ_BUF] = {0};
    //int ret = -1;
    //int i;
    int touch_point;    

    FT5x06DrvReadReg(0, buf, CFG_POINT_READ_BUF);

    ////printf("touch point = %d\n\r", buf[2]);
    touch_point = buf[2]; 
    if (!touch_point)        
        return false;


#if 0
    for (i = 0; i < event->touch_point; i++)
    {
        event->au16_x[i] = (s16)(buf[3 + 6*i] & 0x0F)<<8 | (s16)buf[4 + 6*i];
        event->au16_y[i] = (s16)(buf[5 + 6*i] & 0x0F)<<8 | (s16)buf[6 + 6*i];
        event->auint8_t_touch_event[i] = buf[0x3 + 6*i] >> 6;
        event->auint8_t_finger_id[i] = (buf[5 + 6*i])>>4;
        //printk("%d, %d\n", event->au16_x[i], event->au16_y[i]);
    }
#endif
    *pX = (uint16_t)(buf[3 + 6*0] & 0x0F)<<8 | (uint16_t)buf[4 + 6*0];
    *pY = (uint16_t)(buf[5 + 6*0] & 0x0F)<<8 | (uint16_t)buf[6 + 6*0];

    //*pX = (uint16_t)(buf[3 + 6*0] & 0x0F)<<8;
    //*pY = (uint16_t)(buf[5 + 6*0] & 0x0F)<<8;

    //旋转方向
    uint16_t temp;
    switch (TP_ROT_NONE)
    {
        case TP_ROT_NONE:
            break;
        case TP_ROT_270:
            temp = *pX;
            *pX = 320 - *pY;
            *pY = temp;
            break;
        case TP_ROT_180:
            temp = *pY;
            *pY = *pX;
            *pX = temp;
            break;
        case TP_ROT_90:
            temp = *pY;
            *pY = *pX;
            *pX = 480 - temp;
            break;
        default:
            break;
    }

    return true;    
}


