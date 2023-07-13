/*
 * drv_disp.h
 *
 *  Created on: 2023年4月13日
 *      Author: slhuan
 */

#ifndef DRV_DISP_H_
#define DRV_DISP_H_

#include "hal_data.h"

typedef struct DisplayDevice {
    char *name;
    void *FBBase; /* CPU能直接读写的显存 */
    unsigned short wXres;    /* X方向分辨率 */
    unsigned short wYres;    /* Y方向分辨率 */
    unsigned short wBpp;     /* 每个像素使用多少个像素 */
    unsigned int   dwSize;
    void           (*Init)(struct DisplayDevice *ptDev);   /* 硬件初始化 */
    void           (*DisplayON)(struct DisplayDevice *ptDev);   /* 开启显示 */
    void           (*DisplayOFF)(struct DisplayDevice *ptDev);   /* 关闭显示 */
    void           (*SetDisplayWindow)(struct DisplayDevice* ptDev, \
                                     unsigned short wXs, unsigned short wYs, \
                                     unsigned short wXe, unsigned short wYe);
    //void           (*Flush)(struct DisplayDevice *ptDev); /* 把FBBase的数据刷到LCD的显存里 */
    void           (*Flush)(struct DisplayDevice *ptDev, unsigned char * data, uint32_t len); /* 把FBBase的数据刷到LCD的显存里 */

    /* 设置FBBase中的数据, 把(iX,iY)的像素设置为颜色dwColor
     * dwColor的格式:0x00RRGGBB
     */
    int          (*SetPixel)(struct DisplayDevice *ptDev, \
                             unsigned short wX, unsigned short wY, \
                             unsigned short wColor);
    struct DisplayDevice *pNext;
}DisplayDevice, *PDisplayDevice;


struct DisplayDevice *LCDGetDevice(void);

#endif /* DRV_DISP_H_ */
