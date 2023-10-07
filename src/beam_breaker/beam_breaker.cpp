#include "beam_breaker/beam_breaker.h"

beam_breaker::beam_breaker(GPIO_TypeDef * port, uint16_t pin)
{
  
  GPIO_InitTypeDef GPIO_initstruct;
  GPIO_initstruct.Pin = pin;
  GPIO_initstruct.Mode = GPIO_MODE_AF_PP;
  GPIO_initstruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(port, &GPIO_initstruct);

}

beam_breaker::~beam_breaker()
{

}

void beam_breaker::event_handler()
{

}