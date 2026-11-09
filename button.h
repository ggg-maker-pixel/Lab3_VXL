/*
 * button.h
 *
 *  Created on: Oct 21, 2026
 *      Author: User
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "main.h"

#define NORMAL_STATE  GPIO_PIN_SET
#define PRESSED_STATE GPIO_PIN_RESET

void getKeyInput(void);
int isButton1Pressed(void);
int isButton2Pressed(void);
int isButton3Pressed(void);

#endif /* INC_BUTTON_H_ */
