/*
 * drv_disp.h
 *
 *  Created on: 2023年4月12日
 *      Author: slhuan
 */

#ifndef DRV_LCD_H_
#define DRV_LCD_H_

#include "hal_data.h"

typedef enum{
    notLight,
    isLight
}Black;   /* 背光引脚控制状态 */

typedef enum{
    isReset,
    notReset
}Reset;   /* 复位引脚控制状态 */

typedef enum{
    isSelect,
    notSelect
}CS;      /* 片选信号控制状态 */

typedef enum{
    isCommand,
    isData
}DCX;     /* 数据/命令切换控制状态 */

#endif /* DRV_LCD_H_ */
