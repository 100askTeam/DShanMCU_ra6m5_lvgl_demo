#include "drv_lcd.h"
#include "drv_disp.h"
#include <stdio.h>

static void LCDDrvWaitTX(void);
static void LCDDrvWriteCS(CS eState);
static void LCDDrvWriteDCX(DCX eState);
static void LCDDrvWriteReset(Reset eState);
static void LCDDrvWriteBlack(Black eState);
static void LCDDrvHWReset(void);
static void LCDDrvWriteReg(uint8_t reg);
static void LCDDrvWriteDat(uint8_t dat);
static void LCDDrvWriteBuf(uint8_t* buf, uint32_t size);

static void LCDDrvInit(struct DisplayDevice* ptDev);
static void LCDDrvSetDisplayOn(struct DisplayDevice* ptDev);
static void LCDDrvSetDisplayOff(struct DisplayDevice* ptDev);
static void LCDDrvSetDisplayWindow(struct DisplayDevice* ptDev, \
                                   unsigned short wXs, unsigned short wYs, \
                                   unsigned short wXe, unsigned short wYe);
//static void LCDDrvFlush(struct DisplayDevice *ptDev);
static void LCDDrvFlush(struct DisplayDevice *ptDev, unsigned char * data, uint32_t len);

static int  LCDDrvSetPixel(struct DisplayDevice *ptDev, \
                           unsigned short wX, unsigned short wY, \
                           unsigned short wColor);

/* Event flags for master */
static volatile spi_event_t g_master_event_flag;    // Master Transfer Event completion flag

//static unsigned short gLcdFbuf[320*480];

static DisplayDevice gLcdDevice = {
        .name = "LCD",
        //.FBBase = gLcdFbuf,
        .wXres = 320,
        .wYres = 480,
        .wBpp = 16,
        .dwSize = 320*480*16/8,
        .Init = LCDDrvInit,
        .DisplayON = LCDDrvSetDisplayOn,
        .DisplayOFF = LCDDrvSetDisplayOff,
        .SetDisplayWindow = LCDDrvSetDisplayWindow,
        .Flush = LCDDrvFlush,
        .SetPixel = LCDDrvSetPixel
};

struct DisplayDevice *LCDGetDevice(void)
{
    return &gLcdDevice;
}

static void LCDDrvWriteCS(CS eState)
{
    g_ioport.p_api->pinWrite(g_ioport.p_ctrl,
                             BSP_IO_PORT_01_PIN_03,
                             (bsp_io_level_t)eState);
}
static void LCDDrvWriteDCX(DCX eState)
{
    g_ioport.p_api->pinWrite(g_ioport.p_ctrl,
                             BSP_IO_PORT_01_PIN_04,
                             (bsp_io_level_t)eState);
}
static void LCDDrvWriteReset(Reset eState)
{
    g_ioport.p_api->pinWrite(g_ioport.p_ctrl,
                             BSP_IO_PORT_01_PIN_05,
                             (bsp_io_level_t)eState);
}
static void LCDDrvWriteBlack(Black eState)
{
    g_ioport.p_api->pinWrite(g_ioport.p_ctrl,
                             BSP_IO_PORT_06_PIN_08,
                             (bsp_io_level_t)eState);
}

void spi1_callback(spi_callback_args_t *p_args)
{
    /* 判断是否是发送完成触发的中断 */
    /* 如果是的话就将发送完成标志位置1 */
    if (SPI_EVENT_TRANSFER_COMPLETE == p_args->event)
    {
        g_master_event_flag = SPI_EVENT_TRANSFER_COMPLETE;
    }
    else
    {
        g_master_event_flag = SPI_EVENT_TRANSFER_ABORTED;
    }
}

static void LCDDrvWaitTX(void)
{
    while(!g_master_event_flag);
    g_master_event_flag = false;
}

static void LCDDrvHWReset(void)
{
    LCDDrvWriteReset(isReset);
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);
    LCDDrvWriteReset(notReset);
    R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
}

static void LCDDrvWriteReg(uint8_t reg)
{
    LCDDrvWriteDCX(isCommand);
    g_spi1.p_api->write(&g_spi1_ctrl, (uint8_t*)&reg, 1, SPI_BIT_WIDTH_8_BITS);
    LCDDrvWaitTX();
}

static void LCDDrvWriteDat(uint8_t dat)
{
    LCDDrvWriteDCX(isData);
    g_spi1.p_api->write(&g_spi1_ctrl, (uint8_t*)&dat, 1, SPI_BIT_WIDTH_8_BITS);
    LCDDrvWaitTX();
}

static void LCDDrvWriteBuf(uint8_t* buf, uint32_t size)
{
    //LCDDrvWriteReg(0x3C);
    LCDDrvWriteDCX(isData);
    
    unsigned char *pbuf = (unsigned char*)buf;
    while(size)
    {
        uint32_t length = 0;
        if(size < 65536)
            length = (uint16_t)size;
        else
        {
            length = 65535;
        }
        fsp_err_t err = g_spi1.p_api->write(g_spi1.p_ctrl, pbuf, length, SPI_BIT_WIDTH_8_BITS);
        assert(FSP_SUCCESS==err);
        LCDDrvWaitTX();
        size = size - length;
        pbuf = pbuf + length;
    }
}

void LCDDrvInit(struct DisplayDevice* ptDev)
{
    if(NULL == ptDev->name)    return;
    /* 打开SPI设备完成初始化 */
    fsp_err_t err = g_spi1.p_api->open(&g_spi1_ctrl, &g_spi1_cfg);
    if(FSP_SUCCESS != err)
        return;
    
    /* 初始化屏幕设备 */
    LCDDrvHWReset(); //LCD 复位
    LCDDrvWriteBlack(isLight);//点亮背光
    
    LCDDrvWriteReg(0x11);
    LCDDrvWriteDat(0x00);
    R_BSP_SoftwareDelay(120, BSP_DELAY_UNITS_MILLISECONDS);

    LCDDrvWriteReg(0xf0);
    LCDDrvWriteDat(0xc3);

    LCDDrvWriteReg(0xf0);
    LCDDrvWriteDat(0x96);

    LCDDrvWriteReg(0x36);
    LCDDrvWriteDat(0x48);

    LCDDrvWriteReg(0xb4);
    LCDDrvWriteDat(0x01);

    LCDDrvWriteReg(0xb7);
    LCDDrvWriteDat(0xc6);

    LCDDrvWriteReg(0xe8);
    LCDDrvWriteDat(0x40);
    LCDDrvWriteDat(0x8a);
    LCDDrvWriteDat(0x00);
    LCDDrvWriteDat(0x00);
    LCDDrvWriteDat(0x29);
    LCDDrvWriteDat(0x19);
    LCDDrvWriteDat(0xa5);
    LCDDrvWriteDat(0x33);

    LCDDrvWriteReg(0xc1);
    LCDDrvWriteDat(0x06);

    LCDDrvWriteReg(0xc2);
    LCDDrvWriteDat(0xa7);

    LCDDrvWriteReg(0xc5);
    LCDDrvWriteDat(0x18);

    LCDDrvWriteReg(0xe0);
    LCDDrvWriteDat(0xf0);
    LCDDrvWriteDat(0x09);
    LCDDrvWriteDat(0x0b);
    LCDDrvWriteDat(0x06);
    LCDDrvWriteDat(0x04);
    LCDDrvWriteDat(0x15);
    LCDDrvWriteDat(0x2f);
    LCDDrvWriteDat(0x54);
    LCDDrvWriteDat(0x42);
    LCDDrvWriteDat(0x3c);
    LCDDrvWriteDat(0x17);
    LCDDrvWriteDat(0x14);
    LCDDrvWriteDat(0x18);
    LCDDrvWriteDat(0x1b);

    LCDDrvWriteReg(0xe1);
    LCDDrvWriteDat(0xf0);
    LCDDrvWriteDat(0x09);
    LCDDrvWriteDat(0x0b);
    LCDDrvWriteDat(0x06);
    LCDDrvWriteDat(0x04);
    LCDDrvWriteDat(0x03);
    LCDDrvWriteDat(0x2d);
    LCDDrvWriteDat(0x43);
    LCDDrvWriteDat(0x42);
    LCDDrvWriteDat(0x3b);
    LCDDrvWriteDat(0x16);
    LCDDrvWriteDat(0x14);
    LCDDrvWriteDat(0x17);
    LCDDrvWriteDat(0x1b);

    LCDDrvWriteReg(0xf0);
    LCDDrvWriteDat(0x3c);

    LCDDrvWriteReg(0xf0);
    LCDDrvWriteDat(0x69);

    LCDDrvWriteReg(0x3a);
    LCDDrvWriteDat(0x55);
    R_BSP_SoftwareDelay(120, BSP_DELAY_UNITS_MILLISECONDS);

    LCDDrvWriteReg(0x29);

    /////////
    LCDDrvWriteReg(0x36);
    LCDDrvWriteDat(0x48);

    LCDDrvWriteReg(0x2a);
    LCDDrvWriteDat(0x00);
    LCDDrvWriteDat(0x00);
    LCDDrvWriteDat(0x01);
    LCDDrvWriteDat(0x3f);

    LCDDrvWriteReg(0x2b);
    LCDDrvWriteDat(0x00);
    LCDDrvWriteDat(0x00);
    LCDDrvWriteDat(0x01);
    LCDDrvWriteDat(0xdf);
}

void LCDDrvSetDisplayOn(struct DisplayDevice* ptDev)
{
    if(NULL == ptDev->name)    return;
    LCDDrvWriteReg(0x29);
}
void LCDDrvSetDisplayOff(struct DisplayDevice* ptDev)
{
    if(NULL == ptDev->name)    return;
    LCDDrvWriteReg(0x28);
}

void LCDDrvSetDisplayWindow(struct DisplayDevice* ptDev, \
                           unsigned short hwXs, unsigned short hwYs, \
                           unsigned short hwXe, unsigned short hwYe)
{
    if(NULL == ptDev->name)    return;
    /* 设置列地址 */
    LCDDrvWriteReg(0x2A);
    LCDDrvWriteDat((uint8_t)(hwXs >> 8) & 0xff);       // 起始地址先高后低
    LCDDrvWriteDat((uint8_t)(hwXs) & 0xff);
    LCDDrvWriteDat((uint8_t)(hwXe >> 8) & 0xff);        // 结束地址先高后低
    LCDDrvWriteDat((uint8_t)(hwXe) & 0xff);

    /* 设置行地址 */
    LCDDrvWriteReg(0x2B);
    LCDDrvWriteDat((uint8_t)(hwYs >> 8) & 0xff);
    LCDDrvWriteDat((uint8_t)(hwYs) & 0xff);
    LCDDrvWriteDat((uint8_t)(hwYe >> 8) & 0xff);
    LCDDrvWriteDat((uint8_t)(hwYe) & 0xff);

    LCDDrvWriteReg(0x2c);
}

/* 把FBBase的数据刷到LCD的显存里 */
void LCDDrvFlush(struct DisplayDevice *ptDev, unsigned char * data, uint32_t len)
{
    if(NULL == ptDev->name)    return;
    //LCDDrvWriteBuf((uint8_t*)ptDev->FBBase, (uint32_t)ptDev->dwSize);
    LCDDrvWriteBuf(data, len);
}

int LCDDrvSetPixel(struct DisplayDevice *ptDev, \
                   unsigned short wX, unsigned short wY, \
                   unsigned short wColor)
{
    if(NULL == ptDev->name)    return -1;
    if (wX >= ptDev->wXres || wY >= ptDev->wYres)
        return -1;

    unsigned short *buf = (unsigned short*)ptDev->FBBase;

    buf[wY * ptDev->wXres + wX] = (unsigned short)wColor;

    return 0;
}

