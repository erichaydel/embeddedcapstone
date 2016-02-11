/** \file main.c
*
* @brief Embedded Software Boot Camp
*
* @par
* COPYRIGHT NOTICE: (C) 2014 Barr Group, LLC.
* All rights reserved.
*/

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

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

// Relative Task Priorities (0 = highest; 15 = idle task)
#define  STARTUP_PRIO           1   // Highest priority, to launch others.
#define  DEBOUNCE_PRIO          7   // Every 50 ms, in a timed loop.
#define  SW1_PRIO               8   // Up to every 50 ms, when held down.
#define  ADC_PRIO               9   // Every 125 ms, in a timed loop.
#define  ALARM_PRIO            11   // Up to every 125 ms, when chopping.
#define  SW2_PRIO              12   // Up to every 150 ms, if retriggered.
#define  LED6_PRIO             13   // Every 167 ms, in a timed loop.
#define  LED5_PRIO             14   // Every 500 ms, in a timed loop.

// Allocate Task Stacks
#define  TASK_STACK_SIZE      128

static CPU_STK  g_startup_stack[TASK_STACK_SIZE];
static CPU_STK  g_led5_stack[TASK_STACK_SIZE];
static CPU_STK  g_led6_stack[TASK_STACK_SIZE];
static CPU_STK  g_debounce_stack[TASK_STACK_SIZE];
static CPU_STK  g_sw1_stack[TASK_STACK_SIZE];
static CPU_STK  g_sw2_stack[TASK_STACK_SIZE];
static CPU_STK  g_adc_stack[TASK_STACK_SIZE];
static CPU_STK  g_alarm_stack[TASK_STACK_SIZE];

// Allocate Task Control Blocks
static OS_TCB   g_startup_tcb;
static OS_TCB   g_led5_tcb;
static OS_TCB   g_led6_tcb;
static OS_TCB   g_debounce_tcb;
static OS_TCB   g_sw1_tcb;
static OS_TCB   g_sw2_tcb;
static OS_TCB   g_adc_tcb;
static OS_TCB   g_alarm_tcb;

// Allocate Shared OS Objects
OS_SEM      g_sw1_sem;
OS_SEM      g_sw2_sem;

OS_FLAG_GRP g_alarm_flags;

/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
/*!
* @brief LED Flasher Task
*/
void
led5_task (void * p_arg)
{
    OS_ERR  err;


    (void)p_arg;    // NOTE: Silence compiler warning about unused param.

    for (;;)
    {
        // Flash LED at 1 Hz.
	protectedLED_Toggle(5);
	OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}

/*!
*
* @brief LED Flasher Task
*
*/
void
led6_task (void * p_arg)
{
    OS_ERR  err;


    (void)p_arg;    // NOTE: Silence compiler warning about unused param.

    for (;;)
    {
        // Flash LED at 3 Hz.
	protectedLED_Toggle(6);
	OSTimeDlyHMSM(0, 0, 0, 167, OS_OPT_TIME_HMSM_STRICT, &err);
    }
}




/*!
* @brief Button SW2 Catcher Task
*/
void
sw2_task (void * p_arg)
{
    uint16_t    sw2_counter = 0;
    char	    p_str[LCD_CHARS_PER_LINE+1];
    OS_ERR	    err;
	

    (void)p_arg;    // NOTE: Silence compiler warning about unused param.

    // Draw the initial display.
    sprintf(p_str, "SW2: % 4u", sw2_counter);
    BSP_GraphLCD_String(LCD_LINE2, (char const *) p_str);

    for (;;)
    {
        // Wait for a signal from the button debouncer.
	OSSemPend(&g_sw2_sem, 0, OS_OPT_PEND_BLOCKING, 0, &err);

        // Check for errors.
	assert(OS_ERR_NONE == err);
		
        // Increment button press counter.
	sw2_counter++;

        // Format and display current count.
	sprintf(p_str, "SW2: % 4u", sw2_counter);
        BSP_GraphLCD_String(LCD_LINE2, (char const *) p_str);
    }
}


void
startup_task (void * p_arg)
{
    OS_ERR  err;


    (void)p_arg;    // NOTE: Silence compiler warning about unused param.

    // Perform hardware initializations that should be after multitasking.
    BSP_Init();
    CPU_Init();
    Mem_Init();

    // Set the font for the LCD
    BSP_GraphLCD_SetFont(GLYPH_FONT_8_BY_8);
    
    // Initialize the reentrant LED driver.
    protectedLED_Init();

    // Create the LED flasher tasks.
    OSTaskCreate((OS_TCB     *)&g_led5_tcb,
                 (CPU_CHAR   *)"LED5 Flasher",
                 (OS_TASK_PTR ) led5_task,
                 (void       *) 0,
                 (OS_PRIO     ) LED5_PRIO,
                 (CPU_STK    *)&g_led5_stack[0],
                 (CPU_STK_SIZE) TASK_STACK_SIZE / 10u,
                 (CPU_STK_SIZE) TASK_STACK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      ) 0,
                 (OS_ERR     *)&err);
    assert(OS_ERR_NONE == err);

    OSTaskCreate((OS_TCB     *)&g_led6_tcb,
                 (CPU_CHAR   *)"LED6 Flasher",
                 (OS_TASK_PTR ) led6_task,
                 (void       *) 0,
                 (OS_PRIO     ) LED6_PRIO,
                 (CPU_STK    *)&g_led6_stack[0],
                 (CPU_STK_SIZE) TASK_STACK_SIZE / 10u,
                 (CPU_STK_SIZE) TASK_STACK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      ) 0,
                 (OS_ERR     *)&err);
    assert(OS_ERR_NONE == err);

    // Create the semaphores signaled by the button debouncer.
    OSSemCreate(&g_sw1_sem, "Switch 1", 0, &err);
    assert(OS_ERR_NONE == err);	

    OSSemCreate(&g_sw2_sem, "Switch 2", 0, &err);
    assert(OS_ERR_NONE == err);	

    // Create the button debouncer.
    OSTaskCreate((OS_TCB     *)&g_debounce_tcb,
                 (CPU_CHAR   *)"Button Debouncer",
                 (OS_TASK_PTR ) debounce_task,
                 (void       *) 0,
                 (OS_PRIO     ) DEBOUNCE_PRIO,
                 (CPU_STK    *)&g_debounce_stack[0],
                 (CPU_STK_SIZE) TASK_STACK_SIZE / 10u,
                 (CPU_STK_SIZE) TASK_STACK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      ) 0,
                 (OS_ERR     *)&err);
    assert(OS_ERR_NONE == err);

    // Create the tasks to catch the button semaphores.
    OSTaskCreate((OS_TCB     *)&g_sw1_tcb,
                 (CPU_CHAR   *)"Button 1 Catcher",
                 (OS_TASK_PTR ) sw1_task,
                 (void       *) 0,
                 (OS_PRIO     ) SW1_PRIO,
                 (CPU_STK    *)&g_sw1_stack[0],
                 (CPU_STK_SIZE) TASK_STACK_SIZE / 10u,
                 (CPU_STK_SIZE) TASK_STACK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      ) 0,
                 (OS_ERR     *)&err);
    assert(OS_ERR_NONE == err);

    OSTaskCreate((OS_TCB     *)&g_sw2_tcb,
                 (CPU_CHAR   *)"Button 2 Catcher",
                 (OS_TASK_PTR ) sw2_task,
                 (void       *) 0,
                 (OS_PRIO     ) SW2_PRIO,
                 (CPU_STK    *)&g_sw2_stack[0],
                 (CPU_STK_SIZE) TASK_STACK_SIZE / 10u,
                 (CPU_STK_SIZE) TASK_STACK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      ) 0,
                 (OS_ERR     *)&err);
    assert(OS_ERR_NONE == err);

    // Create the ADC task.
    OSTaskCreate((OS_TCB     *)&g_adc_tcb,
                 (CPU_CHAR   *)"ADC Driver",
                 (OS_TASK_PTR ) adc_task,
                 (void       *) 0,
                 (OS_PRIO     ) ADC_PRIO,
                 (CPU_STK    *)&g_adc_stack[0],
                 (CPU_STK_SIZE) TASK_STACK_SIZE / 10u,
                 (CPU_STK_SIZE) TASK_STACK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      ) 0,
                 (OS_ERR     *)&err);
    assert(OS_ERR_NONE == err);

    // Create the alarm task.
    OSTaskCreate((OS_TCB     *)&g_alarm_tcb,
                 (CPU_CHAR   *)"Alarm Manager",
                 (OS_TASK_PTR ) alarm_task,
                 (void       *) 0,
                 (OS_PRIO     ) ALARM_PRIO,
                 (CPU_STK    *)&g_alarm_stack[0],
                 (CPU_STK_SIZE) TASK_STACK_SIZE / 10u,
                 (CPU_STK_SIZE) TASK_STACK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      ) 0,
                 (OS_ERR     *)&err);
    assert(OS_ERR_NONE == err);

    // Delete the startup task (or enter an infinite loop like other tasks).
    OSTaskDel((OS_TCB *)0, &err);

    // We should never get here.
    assert(0); 
}


/*
*********************************************************************************************************
*                                               main()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : (1) It is assumed that your code will call main() once you have performed all necessary
*                   initialization.
*********************************************************************************************************
*/

void  main (void)
{
    OS_ERR  err;



    CPU_IntDis();                                               /* Disable all interrupts.                              */

    BSP_IntVectSet(27, (CPU_FNCT_VOID)OSCtxSwISR);              /* Setup kernel context switch                          */

    OSInit(&err);                                               /* Init uC/OS-III.                                      */
    assert(OS_ERR_NONE == err);

    // Install application-specific OS hooks.
    App_OS_SetAllHooks();

    // Create the startup task.
    OSTaskCreate((OS_TCB     *)&g_startup_tcb,
                 (CPU_CHAR   *)"Startup Task",
                 (OS_TASK_PTR ) startup_task,
                 (void       *) 0,
                 (OS_PRIO     ) STARTUP_PRIO,
                 (CPU_STK    *)&g_startup_stack[0],
                 (CPU_STK_SIZE) TASK_STACK_SIZE / 10u,
                 (CPU_STK_SIZE) TASK_STACK_SIZE ,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      ) 0,
                 (OS_ERR     *)&err);
    assert(OS_ERR_NONE == err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */

    // We should never get here.
    assert(0);
}

