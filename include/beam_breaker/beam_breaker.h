#ifndef BEAM_BREAKER_H
#define BEAM_BREAKER_H

#pragma once

#include "pins.h"
#include "stm32f7xx_hal.h"

class beam_breaker
{
public:
    beam_breaker(GPIO_TypeDef * , uint16_t);
    ~beam_breaker();
    void event_handler();

private:

};

#endif