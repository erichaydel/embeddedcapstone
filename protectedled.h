/** \file protectedLED.h
*
* @brief Reentrant LED driver.
*
* @par
* COPYRIGHT NOTICE: (c) 2011 Netrino, LLC.
* All rights reserved.
*/

#ifndef _PROTECTEDLED_H
#define _PROTECTEDLED_H

void protectedLED_Init(void);
void protectedLED_Toggle(uint8_t led);

#endif  /* _PROTECTEDLED_H */
