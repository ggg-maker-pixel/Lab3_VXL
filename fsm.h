#ifndef INC_FSM_H_
#define INC_FSM_H_

#include "main.h"

typedef struct
{
    GPIO_PinState redA;
    GPIO_PinState yellowA;
    GPIO_PinState greenA;
    GPIO_PinState redB;
    GPIO_PinState yellowB;
    GPIO_PinState greenB;
    GPIO_PinState extraRed;
} TrafficLightOutput_t;

void fsm_init(void);
void fsm_system_run(void);
void fsm_get_display(uint8_t *modeDigits, uint8_t *valueDigits);
void fsm_get_lights(TrafficLightOutput_t *output);

#endif /* INC_FSM_H_ */
