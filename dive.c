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
#include "dive.h"
#include "scuba.h"

//Scuba globals
int16_t g_current_air_volume = 50;
int32_t g_current_depth_mm = 0;
uint32_t g_dive_time = 0;
int16_t g_air_to_surface;
int16_t g_p_rate;

extern OS_SEM g_add_air_sem;
extern OS_FLAG_GRP g_alarm_flags;

//*****************Tasks

//****Depth updating
//Get current rate and adjust depth using depth_change_in_mm() macro.
void dive_task(void * p_arg)
{
        OS_ERR err;
        uint16_t alarm_curr = 0;
        uint16_t alarm_prev = 0;
        // Wait for message from ADC ISR.
        uint32_t dive_idx = 0;
        
        for(;;)
        {
          //in meters/min
            OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);

            assert(OS_ERR_NONE == err);

            //////////////////
            // Get current depth
            //////////////////
            int16_t depth_change = depth_change_in_mm(g_p_rate);          // Returns change in one half second
            int16_t prev_depth = g_current_depth_mm;
            g_current_depth_mm = (int32_t) (prev_depth - depth_change);
            if (g_current_depth_mm < 0)
            {
                g_current_depth_mm = 0;
            }

            g_air_to_surface = gas_to_surface_in_cl((g_current_depth_mm));

            //////////////////
            /// Update dive time if under the surface
            /////////////////
            if ( g_current_depth_mm > 0 )
            {
              if (dive_idx & 0x1)
              {
                g_dive_time++;
              }
            }
            else
            {
              g_dive_time = 0;
            }
            dive_idx++;
            
            //////////////////
            // Only update air if not at surface
            //////////////////
            if (g_current_depth_mm > 0)
            {
                uint16_t air_consumed = gas_rate_in_cl(g_current_depth_mm);

                uint16_t prev_air = g_current_air_volume;

                g_current_air_volume = (int16_t) prev_air - (int16_t) air_consumed;
            }

            ///////////////
            /// Check for alarm
            ///////////////
            if (g_current_air_volume < 0)
            {
              g_current_air_volume = 0;
            }

            // Select proper alarm state.
            if (g_current_air_volume < g_air_to_surface)
            {
                alarm_curr = ALARM_HIGH;
            }
            else
            {
                alarm_curr = ALARM_NONE;
            }

            // React to changes in speaker priority.
            if (alarm_curr != alarm_prev)
            {
                // Notify the alarm task of the change.
                OSFlagPost(&g_alarm_flags, alarm_curr, OS_OPT_POST_FLAG_SET, &err);
                assert(OS_ERR_NONE == err);
            }

            // Save current alarm state for next cycle.
            alarm_prev = alarm_curr;
        }
}

//Air updating
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
add_air_task (void * p_arg) {

    OS_ERR	    err;


    (void)p_arg;    // NOTE: Silence compiler warning about unused param.


    for (;;)
    {
        // Wait for a signal from the button debouncer.
	OSSemPend(&g_add_air_sem, 0, OS_OPT_PEND_BLOCKING, 0, &err);
        if ( g_current_depth_mm  == 0 && (g_current_air_volume < 2000) ) {
            g_current_air_volume += 5;
        }
        // Check for errors.
	//assert(OS_ERR_NONE == err);

        // Increment button press counter.
	//sw1_counter++;


    }
}

//Print Task
void
print_task(void * p_arg) {

   char     p_str_brand[LCD_CHARS_PER_LINE+1];
   char	    p_str_depth[LCD_CHARS_PER_LINE+1];
   char	    p_str_rate[LCD_CHARS_PER_LINE+1];
   char	    p_str_air_volume[LCD_CHARS_PER_LINE+1];
   char	    p_str_time[LCD_CHARS_PER_LINE+1];

   (void)p_arg;    // NOTE: Silence compiler warning about unused param.

   for(;;)
   {
     //Format and display
     sprintf(p_str_brand, "Jaws&Co.");
     BSP_GraphLCD_String(LCD_LINE0, (char const *) p_str_brand);

     sprintf(p_str_depth, "DEPTH: %4d", (g_current_depth_mm / 1000));
     BSP_GraphLCD_String(LCD_LINE2, (char const *) p_str_depth);

     sprintf(p_str_rate, "RATE: %4d", g_p_rate);
     BSP_GraphLCD_String(LCD_LINE3, (char const *) p_str_rate);

     sprintf(p_str_air_volume, "AIR: %4u", g_current_air_volume);
     BSP_GraphLCD_String(LCD_LINE4, (char const *) p_str_air_volume);

     sprintf(p_str_time, "EDT: %4u", g_dive_time);
     BSP_GraphLCD_String(LCD_LINE5, (char const *) p_str_time);
   }

}