/*
 * app_disp.c
 *
 *  Created on: 2023年3月31日
 *      Author: slhuan
 */
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "app.h"
#include "lv_demo_100ask/lv_100ask.h"
#include <stdio.h>

#define FLOYRGB565(r, g, b) ((unsigned short)((((unsigned short)(r>>3)<<11)|(((unsigned short)(g>>2))<<5)|((unsigned short)b>>3))))

static fsp_err_t gpt_timer_init(gpt_instance_ctrl_t * p_timer_ctrl, const timer_cfg_t * p_timer_cfg);

void app_lvgl(void)
{
    fsp_err_t err = FSP_SUCCESS;     // Error status

    /* Start GPT timer to 'Give' Semaphore periodically at 1sec for semaphore_task */
    err = gpt_timer_init(&g_timer0_lv_tick_inc_ctrl, &g_timer0_lv_tick_inc_cfg );
    if(FSP_SUCCESS != err)
    {
        return;
    }

    lv_init();

    lv_port_disp_init();

    lv_port_indev_init();

    //lv_obj_create(lv_scr_act());

    //lv_demo_widgets();
    //lv_demo_music();
    //lv_demo_benchmark();
    lv_100ask_demo(1500);

    while(1)
    {
        lv_task_handler();
        R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MILLISECONDS);  // delay 5ms
    }
}





void periodic_timer_lv_tick_inc_cb(timer_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);

    lv_tick_inc(1);

}


static fsp_err_t gpt_timer_init(gpt_instance_ctrl_t * p_timer_ctrl, const timer_cfg_t * p_timer_cfg)
{
    fsp_err_t fsp_err = FSP_SUCCESS;

    /* Open GPT timer instance */
    fsp_err = R_GPT_Open (p_timer_ctrl, p_timer_cfg);
    /* Handle error */
    if ( FSP_SUCCESS != fsp_err )
    {
        /* Print out in case of error */
        //APP_ERR_PRINT ("\r\nGPT Timer open API failed\r\n");
        return fsp_err;
    }


    /* Start GPT Timer instance */
    fsp_err = R_GPT_Start (p_timer_ctrl);
    /* Handle error */
    if (FSP_SUCCESS != fsp_err)
    {
        /* Close timer if failed to start */
        if ( FSP_SUCCESS  != R_GPT_Close (p_timer_ctrl) )
        {
            /* Print out in case of error */
            //APP_ERR_PRINT ("\r\nGPT Timer Close API failed\r\n");
        }

       // APP_ERR_PRINT ("\r\nGPT Timer Start API failed\r\n");
        return fsp_err;
    }

    return fsp_err;
}
