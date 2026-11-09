#include "main.h"
#include "button.h"
#include "fsm.h"

#define MODE_1_NORMAL 1U
#define MODE_2_RED 2U
#define MODE_3_YELLOW 3U
#define MODE_4_GREEN 4U

typedef enum { LIGHT_RED, LIGHT_GREEN, LIGHT_YELLOW } LightState_t;

static uint8_t currentMode;
static uint8_t timeRed = 5U;
static uint8_t timeYellow = 2U;
static uint8_t timeGreen = 3U;
static uint8_t editValue;
static LightState_t stateA, stateB;
static uint32_t stateAStartedAt, stateBStartedAt;
static uint8_t displayMode, displayValue;
static TrafficLightOutput_t lightOutput;

static uint32_t durationMs(LightState_t state)
{
    if (state == LIGHT_RED) return (uint32_t)timeRed * 1000U;
    if (state == LIGHT_YELLOW) return (uint32_t)timeYellow * 1000U;
    return (uint32_t)timeGreen * 1000U;
}

static LightState_t nextLightState(LightState_t state)
{
    if (state == LIGHT_RED) return LIGHT_GREEN;
    if (state == LIGHT_GREEN) return LIGHT_YELLOW;
    return LIGHT_RED;
}

static uint8_t remainingSeconds(uint32_t duration, uint32_t elapsed)
{
    uint32_t remaining = (elapsed < duration) ? (duration - elapsed) : 0U;
    return (uint8_t)((remaining + 999U) / 1000U);
}

static void updateNormalMode(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t elapsedA = now - stateAStartedAt;
    uint32_t elapsedB = now - stateBStartedAt;
    uint32_t durationA = durationMs(stateA);
    uint32_t durationB = durationMs(stateB);

    if (elapsedA >= durationA) {
        stateA = nextLightState(stateA); stateAStartedAt = now;
        elapsedA = 0U; durationA = durationMs(stateA);
    }
    if (elapsedB >= durationB) {
        stateB = nextLightState(stateB); stateBStartedAt = now;
        elapsedB = 0U; durationB = durationMs(stateB);
    }

    lightOutput.redA = (stateA == LIGHT_RED) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    lightOutput.yellowA = (stateA == LIGHT_YELLOW) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    lightOutput.greenA = (stateA == LIGHT_GREEN) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    lightOutput.redB = (stateB == LIGHT_RED) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    lightOutput.yellowB = (stateB == LIGHT_YELLOW) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    lightOutput.greenB = (stateB == LIGHT_GREEN) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    lightOutput.extraRed = GPIO_PIN_RESET;
    displayMode = remainingSeconds(durationA, elapsedA);
    displayValue = remainingSeconds(durationB, elapsedB);
}

static void updateConfigurationMode(void)
{
    GPIO_PinState blink = ((HAL_GetTick() % 500U) < 250U) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    displayMode = currentMode;       /* EN0/EN1: 02, 03, or 04 */
    displayValue = editValue;        /* EN2/EN3: editable value 01..99 */
    lightOutput.redA = GPIO_PIN_RESET;
    lightOutput.yellowA = GPIO_PIN_RESET;
    lightOutput.greenA = GPIO_PIN_RESET;
    lightOutput.redB = GPIO_PIN_RESET;
    lightOutput.yellowB = GPIO_PIN_RESET;
    lightOutput.greenB = GPIO_PIN_RESET;
    lightOutput.extraRed = GPIO_PIN_RESET;

    if (currentMode == MODE_2_RED) {
        lightOutput.redA = blink;
        lightOutput.redB = blink;
        lightOutput.extraRed = blink;
    } else if (currentMode == MODE_3_YELLOW) {
        lightOutput.yellowA = blink;
        lightOutput.yellowB = blink;
    } else {
        lightOutput.greenA = blink;
        lightOutput.greenB = blink;
    }
}

static void enterMode(uint8_t mode)
{
    currentMode = mode;
    if (mode == MODE_2_RED) editValue = timeRed;
    else if (mode == MODE_3_YELLOW) editValue = timeYellow;
    else if (mode == MODE_4_GREEN) editValue = timeGreen;
    else {
        stateA = LIGHT_RED; stateB = LIGHT_GREEN;
        stateAStartedAt = HAL_GetTick(); stateBStartedAt = stateAStartedAt;
    }
}

void fsm_init(void)
{
    currentMode = MODE_1_NORMAL;
    stateA = LIGHT_RED; stateB = LIGHT_GREEN;
    stateAStartedAt = HAL_GetTick(); stateBStartedAt = stateAStartedAt;
}

void fsm_system_run(void)
{
    if (isButton1Pressed() != 0) {
        uint8_t nextMode = currentMode + 1U;
        if (nextMode > MODE_4_GREEN) nextMode = MODE_1_NORMAL;
        enterMode(nextMode);
    }
    if (currentMode != MODE_1_NORMAL) {
        if (isButton2Pressed() != 0) {
        	editValue++;
        	if (editValue > 99U) editValue = 1U;
        }
        if (isButton3Pressed() != 0) {
            if (currentMode == MODE_2_RED) timeRed = editValue;
            else if (currentMode == MODE_3_YELLOW) timeYellow = editValue;
            else timeGreen = editValue;
        }
        updateConfigurationMode();
    } else updateNormalMode();
}

void fsm_get_display(uint8_t *modeDigits, uint8_t *valueDigits)
{
    *modeDigits = displayMode; *valueDigits = displayValue;
}

void fsm_get_lights(TrafficLightOutput_t *output)
{
    *output = lightOutput;
}
