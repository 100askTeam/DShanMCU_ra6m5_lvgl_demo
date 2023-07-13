/*
 * drv_touch.h
 *
 *  Created on: 2023年4月20日
 *      Author: slhuan
 */

#include "hal_data.h"

typedef struct TouchDev{
    char *name;
    void (*Init)(struct TouchDev *ptDev);
    bool (*Read)(struct TouchDev *ptDev, unsigned short *pX, unsigned short *pY);
}TouchDev, *PTouchDev;

struct TouchDev* TouchDevGet(void);

