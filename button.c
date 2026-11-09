/*
 * button.c
 *
 * Button input and debouncing. TIM2 calls getKeyInput() every 10 ms.
 */
#include "main.h"
#include "button.h"

#define NUMBER_OF_BUTTONS  3U

static GPIO_PinState keyReg0[NUMBER_OF_BUTTONS] =
    {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};
static GPIO_PinState keyReg1[NUMBER_OF_BUTTONS] =
    {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};
static GPIO_PinState keyReg2[NUMBER_OF_BUTTONS] =
    {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};
static GPIO_PinState keyReg3[NUMBER_OF_BUTTONS] =
    {NORMAL_STATE, NORMAL_STATE, NORMAL_STATE};

static volatile uint8_t button1Flag;
static volatile uint8_t button2Flag;
static volatile uint8_t button3Flag;

static GPIO_PinState ReadButton(uint8_t index)
{
    if (index == 0U) return HAL_GPIO_ReadPin(BUT1_GPIO_Port, BUT1_Pin);
    if (index == 1U) return HAL_GPIO_ReadPin(BUT2_GPIO_Port, BUT2_Pin);
    return HAL_GPIO_ReadPin(BUT3_GPIO_Port, BUT3_Pin);
}

static void subKeyProcess(uint8_t index)
{
    if (index == 0U) button1Flag = 1U;
    else if (index == 1U) button2Flag = 1U;
    else button3Flag = 1U;
}

void getKeyInput(void)
{
    uint8_t i;

    for (i = 0U; i < NUMBER_OF_BUTTONS; i++)
    {
        keyReg0[i] = keyReg1[i];
        keyReg1[i] = keyReg2[i];
        keyReg2[i] = ReadButton(i);

        /* Accept a change only after three identical 10 ms samples (30 ms). */
        if ((keyReg0[i] == keyReg1[i]) && (keyReg1[i] == keyReg2[i]))
        {
            if (keyReg3[i] != keyReg2[i])
            {
                keyReg3[i] = keyReg2[i];
                if (keyReg3[i] == PRESSED_STATE)
                {
                    subKeyProcess(i);  /* One event per physical press. */
                }
            }
        }
    }
}

int isButton1Pressed(void)
{
    if (button1Flag != 0U) { button1Flag = 0U; return 1; }
    return 0;
}

int isButton2Pressed(void)
{
    if (button2Flag != 0U) { button2Flag = 0U; return 1; }
    return 0;
}

int isButton3Pressed(void)
{
    if (button3Flag != 0U) { button3Flag = 0U; return 1; }
    return 0;
}

