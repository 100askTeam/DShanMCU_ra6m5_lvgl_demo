/*
 * drv_gt911.h
 *
 *  Created on: 2023年4月20日
 *      Author: slhuan
 */

#ifndef DRV_GT911_H_
#define DRV_GT911_H_

#include "hal_data.h"

#define TOUCH_POINT_TOTAL       5 /* 此芯片最多支持五点触控 */

//GT911 部分寄存器定义
#define GT_CTRL_REG     0x8040      //GT911控制寄存器
#define GT_CFGS_REG     0x8047      //GT911配置起始地址寄存器
#define GT_CHECK_REG    0x80FF      //GT911校验和寄存器
#define GT_PID_REG      0x8140      //GT911产品ID寄存器

#define GT_GSTID_REG    0x814E      //GT911当前检测到的触摸情况
#define GT_TP1_REG      0x814F      //第一个触摸点数据地址
#define GT_TP2_REG      0x8157      //第二个触摸点数据地址
#define GT_TP3_REG      0x815F      //第三个触摸点数据地址
#define GT_TP4_REG      0x8167      //第四个触摸点数据地址
#define GT_TP5_REG      0x816F      //第五个触摸点数据地址

#define GT911_READ_XY_REG       0x814E  /* 坐标寄存器 */
#define GT911_CLEARBUF_REG      0x814E  /* 清除坐标寄存器 */
#define GT911_CONFIG_REG        0x8047  /* 配置参数寄存器 */
#define GT911_COMMAND_REG       0x8040  /* 实时命令 */
#define GT911_PRODUCT_ID_REG    0x8140  /*productid*/
#define GT911_VENDOR_ID_REG     0x814A  /* 当前模组选项信息 */
#define GT911_CONFIG_VERSION_REG    0x8047  /* 配置文件版本号 */
#define GT911_CONFIG_CHECKSUM_REG   0x80FF  /* 配置文件校验码 */
#define GT911_FIRMWARE_VERSION_REG  0x8144  /* 固件版本号 */

typedef enum{
    TP_ROT_NONE = 0,
    TP_ROT_90,
    TP_ROT_180,
    TP_ROT_270
} TouchRotation_t;

/**用于存放每一个触控点的id，坐标，大小**/
typedef struct TouchPointInfo{
    unsigned char id;
    unsigned short x;
    unsigned short y;
    unsigned short size;
}TouchPointInfo_t;

/**类结构体**/
typedef struct TouchDrv{
    unsigned char  ucAddr;
    unsigned short wHeight;
    unsigned short wWidth;
    TouchRotation_t tRotation;
    TouchPointInfo_t tPointsInfo[TOUCH_POINT_TOTAL]; //用于存储五个触控点的坐标
}TouchDrv_t;

#endif /* DRV_GT911_H_ */
