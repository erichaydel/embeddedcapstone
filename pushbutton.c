/** \file pushbutton.c
*
* @brief Button Debouncer
*
* @par
* COPYRIGHT NOTICE: (C) 2014 Barr Group, LLC.
* All rights reserved.
*/

#include <assert.h>
#include <stdint.h>

#include "os.h"
#include "iorx63n.h"

#include "pushbutton.h"
#include "dive.h"

extern OS_SEM g_add_air_sem;


/*!
*
* @brief Button Debounce Task
*/
void
debounce_task (void * p_arg)
{
    // State variables for debouncing SW1.
    uint8_t	    b_sw1_curr = 1;
    uint8_t     b_sw1_prev = 1;

    // State variables for debouncing SW2.
    uint8_t	    b_sw2_curr = 1;
    uint8_t     b_sw2_prev = 1;
    uint8_t     b_sw2_retriggered = 1;

    OS_ERR      err;


    (void)p_arg;    // NOTE: Silence compiler warning about unused param.

    // Configure GPIO Port4 as an input.
    PORT4.PDR.BYTE = 0;

    for (;;)
    {
        // Delay for 50 ms.
	OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &err);
	
        // Read the current state of the buttons.
        uint8_t raw = PORT4.PIDR.BYTE;

        // Test button SW1.
	b_sw1_curr = raw & 0x01;

        // Execute SW1 debouncing state machine.
	if ((0 == b_sw1_curr) && (0 == b_sw1_prev))
        {
            // Signal that SW1 has been pressed (or is still held down).
	    OSSemPost(&g_add_air_sem, OS_OPT_POST_1, &err);
	    assert(OS_ERR_NONE == err);
	}

        // Save current SW1 state for next cycle.
	b_sw1_prev = b_sw1_curr;
	
        // Test button SW2.
	b_sw2_curr = raw & 0x02;

        // Execute SW2 debouncing state machine.
	if ((0 == b_sw2_curr) && (0 == b_sw2_prev))
        {
            if (b_sw2_retriggered)
            {
                // Signal that SW2 has been pressed.
	        OSSemPost(&g_sw2_sem, OS_OPT_POST_1, &err);
	        assert(OS_ERR_NONE == err);

                // Do not allow repeats while held down.
                b_sw2_retriggered = 0;
            }
        }
        else
        {
            // Button released; reset trigger.
            b_sw2_retriggered = 1;
	}

        // Save current SW2 state for next cycle.
	b_sw2_prev = b_sw2_curr;
    }
}
