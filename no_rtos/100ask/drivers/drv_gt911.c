/*
 * drv_touch.c
 *
 *  Created on: 2023年4月20日
 *      Author: slhuan
 */

#include "drv_gt911.h"
//#include "hal_systick.h"
#include "drv_touch.h"
#include <stdio.h>
#include <stdlib.h>

static void I2C2WaitTxCplt(void);
static void I2C2WaitRxCplt(void);

static void GT911DrvWriteReg(uint16_t reg, uint8_t *buf, uint8_t len);
static void GT911DrvReadReg(uint16_t reg, uint8_t *buf, uint8_t len);
static void GT911DrvSoftReset(void);
static uint32_t GT911DrvReadProductID(void);
static uint32_t GT911DrvReadVendorID(void);
static void GT911DrvClearBuf(void);
static uint8_t GT911DrvReadVersion(void);
static uint8_t GT911DrvGetGSTID(void);
static void GT911DrvSetRotation(TouchDrv_t *tp, TouchRotation_t rot);
static bool GT911DrvIsTouched(TouchDrv_t * tp);

static void GT911DrvInit(struct TouchDev *ptDev);
static bool GT911DrvRead(struct TouchDev *ptDev, unsigned short *pX, unsigned short *pY);

static struct TouchDev gTouchDev = {
                                    .name = "GT911",
                                    .Init = GT911DrvInit,
                                    .Read = GT911DrvRead
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

static void GT911DrvWriteReg(uint16_t reg, uint8_t *buf, uint8_t len)
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

static void GT911DrvReadReg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t tmpbuf[2];

    tmpbuf[0] = (uint8_t)(reg >> 8);
    tmpbuf[1] = (uint8_t)(reg &0xff);

    fsp_err_t err = g_i2c_master2.p_api->write(g_i2c_master2.p_ctrl, tmpbuf, 2, 0);
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

static void GT911DrvSoftReset(void)
{
    uint8_t buf[1];
    buf[0] = 0x02;
    GT911DrvWriteReg(GT911_COMMAND_REG, (uint8_t *)buf, 1);
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);
    buf[0] = 0x0;
    GT911DrvWriteReg(GT911_COMMAND_REG, (uint8_t *)buf, 1);
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);
}

static uint32_t GT911DrvReadProductID(void)
{
    uint32_t id = 0;
    GT911DrvReadReg(GT911_PRODUCT_ID_REG, (uint8_t*)&id ,4);
    return id;
}

static uint32_t GT911DrvReadVendorID(void)
{
    uint32_t id = 0;
    GT911DrvReadReg(GT911_VENDOR_ID_REG, (uint8_t*)&id ,4);
    return id;
}


static void GT911DrvClearBuf(void)
{
    uint8_t data = {0};
    GT911DrvWriteReg(GT911_CLEARBUF_REG, (uint8_t*)&data, 1);
}

static uint8_t GT911DrvReadVersion(void)
{
    uint8_t version = 0;
    GT911DrvReadReg(GT911_CONFIG_VERSION_REG, (uint8_t*)&version ,1);
    return version;
}

static uint8_t GT911DrvGetGSTID(void)
{
    uint8_t id = 0;
    GT911DrvReadReg(GT_GSTID_REG, (uint8_t*)&id, 1);
    return id;
}

/* GT9XX可以选择2个I2C地址:0x5d 或 x14
 * rst信号从低到高变化: int信号是0则使用地址0x5d, 是1则使用地址0x14,
 * 在这之后再把int信号设置为中断引脚
 */
static void GT911DrvInit(struct TouchDev *ptDev)
{
    if(NULL == ptDev->name) return;
    uint8_t buf[4];
    gTP.ucAddr = (uint8_t)g_i2c_master2.p_cfg->slave;
    gTP.tRotation = TP_ROT_NONE;

    /* 初始化I2C驱动 */
    fsp_err_t err = g_i2c_master2.p_api->open(g_i2c_master2.p_ctrl, g_i2c_master2.p_cfg);
    if (FSP_SUCCESS != err)
    {
        //printf("%s %d\r\n", __FUNCTION__, __LINE__);
        return;
    }

    /* 读ID */
    //uint32_t nVendorID = GT911DrvReadVendorID();
    //printf("gt911 vendor id: 0x%.4x\r\n", (int)nVendorID);

    //uint32_t nProductID = GT911DrvReadProductID();
    //printf("gt911 product id: 0x%.4x\r\n", (int)nProductID);

    //uint8_t nVersion = GT911DrvReadVersion();
    //printf("version = 0x%x\r\n", nVersion);

    GT911DrvReadReg(0x8048, buf, 2);
    gTP.wWidth = (unsigned short)((buf[1] << 8) | buf[0]);

    GT911DrvReadReg(0x804A, buf, 2);
    gTP.wHeight = (unsigned short)((buf[1] << 8) | buf[0]);

}

static void GT911DrvSetRotation(TouchDrv_t *tp, TouchRotation_t rot)
{
    tp->tRotation = rot;
}


/**
 * @brief 检测是否被触摸并且获取相关值
 * @param tp
 * @return 触摸为 true，反之 false
 */
static bool GT911DrvIsTouched(TouchDrv_t * tp)
{
    uint8_t touched_state, touch_num, buffer_status;
    touched_state = GT911DrvGetGSTID();
    touch_num = touched_state & 0xf;            //触点数量
    buffer_status = (touched_state >> 7) & 1;   // 帧状态

    if(buffer_status == 1 && (touch_num <= TOUCH_POINT_TOTAL) && (touch_num > 0))
    {
        uint16_t pointers_regs[TOUCH_POINT_TOTAL] = {GT_TP1_REG, GT_TP2_REG, GT_TP3_REG, GT_TP4_REG, GT_TP5_REG};
        // 获取每个触控点的坐标值并保存
        for (int i = 0; i < touch_num; ++i)
        {
            uint8_t point_info_per_size = 7;
            uint8_t * point_info_p = malloc(point_info_per_size * sizeof(uint8_t ));
            GT911DrvReadReg(pointers_regs[i], point_info_p, point_info_per_size);

            tp->tPointsInfo[i].id = point_info_p[0];
            tp->tPointsInfo[i].x = (unsigned short)(point_info_p[1] + (point_info_p[2] << 8));
            tp->tPointsInfo[i].y = (unsigned short)(point_info_p[3] + (point_info_p[4] << 8));
            tp->tPointsInfo[i].size = (unsigned short)(point_info_p[5] + (point_info_p[6] << 8));

            free(point_info_p);

            //旋转方向
            uint16_t temp;
            switch (tp->tRotation)
            {
                case TP_ROT_NONE:
                    tp->tPointsInfo[i].x = tp->wWidth - tp->tPointsInfo[i].x;
                    tp->tPointsInfo[i].y = tp->wHeight - tp->tPointsInfo[i].y;
                    break;
                case TP_ROT_270:
                    temp = tp->tPointsInfo[i].x;
                    tp->tPointsInfo[i].x = tp->wWidth - tp->tPointsInfo[i].y;
                    tp->tPointsInfo[i].y = temp;
                    break;
                case TP_ROT_180:
//                    tp->tPointsInfo[i].x = tp->tPointsInfo[i].x;
//                    tp->tPointsInfo[i].y = tp->tPointsInfo[i].y;
                    break;
                case TP_ROT_90:
                    temp = tp->tPointsInfo[i].x;
                    tp->tPointsInfo[i].x = tp->tPointsInfo[i].y;
                    tp->tPointsInfo[i].y = tp->wHeight - temp;
                    break;
                default:
                    break;
            }
        }
        GT911DrvClearBuf();
        return true;
    }
    //必须给GT911_POINT_INFO缓冲区置0,不然读取的数据一直为128！！！！
    GT911DrvClearBuf();

    return false;
}


/**
 * @brief 获取每个触控点的位置
 * @param tp 类实例
 * @param x 被修改的x值
 * @param y 被修改的y值
 * @param index 触控点下标 [0-4]
 */
static bool GT911DrvRead(struct TouchDev *ptDev, unsigned short *pX, unsigned short *pY)
{
    if(NULL == ptDev->name) return false;
    if(GT911DrvIsTouched(&gTP))
    {
        *pX = gTP.tPointsInfo[0].x;
        *pY = gTP.tPointsInfo[0].y;
        return true;
    }

    return false;
}


