/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f7xx_hal_conf_template.h
  * @author  MCD Application Team
  * @brief   HAL configuration template file.
  *          This file should be copied to the application folder and renamed
  *          to stm32f7xx_hal_conf.h.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H


#ifdef __cplusplus
 extern "C" {
#endif

#include "pins.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>

#include "usbd_conf.h"
#include "usbd_desc.h"
#include "usbd_cdc_interface.h"


#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc); return 1;}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}


void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
static void Error_Handler(void);
int main(void);


#define STEP1_GPIO_OUTPUT GPIOC
#define STEP1_OUTPUT_PIN  GPIO_PIN_8
#define STEP2_GPIO_OUTPUT GPIOF
#define STEP2_OUTPUT_PIN  GPIO_PIN_7

#define STEP1_GPIO_DIR    GPIOJ
#define STEP1_DIR_PIN     GPIO_PIN_0
#define STEP2_GPIO_DIR    GPIOI
#define STEP2_DIR_PIN     GPIO_PIN_3

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/



//void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/






/* Section 2: PHY configuration section */



#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

