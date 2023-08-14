/**
  ******************************************************************************
  * @file    stm32f7xx_it.c
  * @author  Ac6
  * @version V1.0
  * @date    02-Feb-2015
  * @brief   Default Interrupt Service Routines.
  ******************************************************************************
*/
#define USE_RTOS_SYSTICK
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"
#ifdef USE_RTOS_SYSTICK
#include <cmsis_os.h>
#endif
#include "stm32f7xx_it.h"
#include "BSP_DISCO_F769NI/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery.h"
#include "BSP_DISCO_F769NI/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_lcd.h"
#include "BSP_DISCO_F769NI/Drivers/BSP/STM32F769I-Discovery/stm32f769i_discovery_sdram.h"
#include "../lvgl.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern DSI_HandleTypeDef hdsi_discovery;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            	  	    Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles SysTick Handler, but only if no RTOS defines it.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
	lv_tick_inc(1);
#ifdef USE_RTOS_SYSTICK
	osSystickHandler();
#endif
}

/**
  * @brief  This function handles DSI Handler.
  * @param  None
  * @retval None
  */
void DSI_IRQHandler(void){
  HAL_DSI_IRQHandler(&hdsi_discovery);
}