/** \file protectedled.c
*
* @brief Reentrant LED driver.
*
* @par
* COPYRIGHT NOTICE: (c) 2014 Barr Group, LLC.
* All rights reserved.
*/

#include <assert.h>
#include <stdint.h>
#include  "cpu.h"
#include  "bsp_led.h"

#include "os.h"

#include "protectedled.h"								


// Private mutex.
static OS_MUTEX  g_led_mutex;    	


/*!
* @brief Initialize the reentrant LED driver.
*/
void
protectedLED_Init (void)
{
    OS_ERR err;

   // Create the mutex that protects the hardware from race conditions.
   OSMutexCreate(&g_led_mutex, "LED Mutex", &err);
   assert(OS_ERR_NONE == err);
}

/*!
* @brief Toggle one or all user LEDs, safely.
* @param[in] led The LED number, or 0 for all.
*/
void
protectedLED_Toggle (uint8_t led)
{
    OS_ERR   err;
	

    // Try to acquire the mutex.
    OSMutexPend(&g_led_mutex, 0, OS_OPT_PEND_BLOCKING, 0, &err);
	assert(OS_ERR_NONE == err);
    {
        // Safely inside the critical section.

        // Call the non-reentrant driver.
        BSP_LED_Toggle(led);
    }
    // Release the mutex.
    OSMutexPost(&g_led_mutex, OS_OPT_POST_NONE, &err);
	assert(OS_ERR_NONE == err);
}
