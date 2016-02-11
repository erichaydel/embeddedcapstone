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

//*****************Tasks
//****Depth updating 
//Get current rate and adjust depth using depth_change_in_mm() macro.

//Air updating
//Adjust air using gas_rate_in_cl() function

//Dive_time - keep track of elapsed dive time

//Dive task - keeping track of depth - timer-based

//Output: Posts sems when depth=surface and appropriate alarms based on current depth and insufficient air


//Add timer task


//Add air task
/*!
* @brief Button SW1 Catcher Task
*/
void
sw1_task (void * p_arg)
{
    uint16_t    sw1_counter = 0;
    char	    p_str[LCD_CHARS_PER_LINE+1];
    OS_ERR	    err;
	

    (void)p_arg;    // NOTE: Silence compiler warning about unused param.

    // Draw the initial display.
    sprintf(p_str, "SW1: % 4u", sw1_counter);
    BSP_GraphLCD_String(LCD_LINE1, (char const *) p_str);

    for (;;)
    {
        // Wait for a signal from the button debouncer.
	OSSemPend(&g_sw1_sem, 0, OS_OPT_PEND_BLOCKING, 0, &err);

        // Check for errors.
	assert(OS_ERR_NONE == err);
		
        // Increment button press counter.
	sw1_counter++;

        // Format and display current count.
	sprintf(p_str, "SW1: % 4u", sw1_counter);
        BSP_GraphLCD_String(LCD_LINE1, (char const *) p_str);
    }
}
