/*dive.c*/



/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  "cpu_cfg.h"
#include  "bsp_cfg.h"
#include  "assert.h"
#include  "stdio.h"

#include  <cpu_core.h>
#include  <os.h>
#include  <bsp_glcd.h>


#include  "bsp.h"
#include  "bsp_int_vect_tbl.h"
#include  "bsp_led.h"
#include  "os_app_hooks.h"

#include "protectedled.h"
#include "pushbutton.h"
#include "common.h"
#include "adc.h"
#include "alarm.h"
#include "scuba.h"

uint16_t g_current_depth;
uint16_t g_air_to_surface;
uint16_t * g_p_rate;
uint16_t  g_current_air_volume;

//OS_Q g_adc_q;

extern OS_SEM g_add_air_sem;

//Scuba globals
uint16_t g_current_air_volume;
uint16_t g_current_depth;

extern OS_SEM g_add_air_sem;

//Scuba globals
uint16_t g_current_air_volume;
uint16_t g_current_depth;

//*****************Tasks
//****Depth updating
//Get current rate and adjust depth using depth_change_in_mm() macro.
void dive_task(void * p_arg)
{
        OS_ERR err;
        // Wait for message from ADC ISR.
        OS_MSG_SIZE  msg_size;

        //in meters/min
	uint16_t * g_p_rate = (uint16_t *)
        OSQPend(&g_adc_q, 0, OS_OPT_PEND_BLOCKING, &msg_size, NULL, &err);
	assert(OS_ERR_NONE == err);

        uint16_t depth_change = depth_change_in_mm(*g_p_rate);
        uint16_t prev_depth = g_current_depth;
        g_current_depth = prev_depth + depth_change;

        uint16_t air_consumed = gas_rate_in_cl(g_current_depth);
        g_air_to_surface = gas_to_surface_in_cl(g_current_depth);

        uint16_t prev_air = g_current_air_volume;
        g_current_air_volume = prev_air - air_consumed;
}
//air updating
//Adjust air using gas_rate_in_cl() function

//Dive_time - keep track of elapsed dive time

//Dive task - keeping track of depth - timer-based

//Output: Posts sems when depth=surface and appropriate alarms based on current depth and insufficient air


//Add timer task

//Add air task
/* add_air_task
*  Pends air semaphore and increments global air volume by 5 if at the surface and tank isn't filled.
*/
void
add_air_task (void * p_arg)
{
    uint16_t        sw1_counter = 0;
    char	    p_str[LCD_CHARS_PER_LINE+1];
    OS_ERR	    err;


    (void)p_arg;    // NOTE: Silence compiler warning about unused param.


    for (;;)
    {
        // Wait for a signal from the button debouncer.
	OSSemPend(&g_add_air_sem, 0, OS_OPT_PEND_BLOCKING, 0, &err);
        if ( g_current_depth == 0 && (g_current_air_volume < 2000) ) {
            g_current_air_volume += 5;
        }
        // Check for errors.
	//assert(OS_ERR_NONE == err);
        // Increment button press counter.
	//sw1_counter++;

        // Format and display current count.
	//sprintf(p_str, "SW1: % 4u", sw1_counter);
        //BSP_GraphLCD_String(LCD_LINE1, (char const *) p_str);
    }
}
