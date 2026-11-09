/*
 * timer.c
 *
 *  Created on: Oct 21, 2026
 *      Author: User
 */
#include "main.h"
#include "button.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        getKeyInput();
    }
}

