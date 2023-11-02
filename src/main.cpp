// anibox - research box controller
// by oliver foster - omwfoster
// published under GPL
// untested and with zero liability

#include "main.hpp"
#include "MX_config.h"
#include "string.h"
#include "tft.h"
#include "touchpad.h"
#include "lvgl.h"
#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"
#include "stepper/stepper_C.h"
#include "beam_breaker/beam_breaker.h"
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/custom_transport.h>
#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>

#include <std_msgs/msg/int32.h>

static volatile int uxTopUsedPriority;












uint8_t cec_receive_buffer[16];
static void Error_Handler(void);

void PeriphCommonClock_Config(void);


osThreadId lvgl_tickHandle;
osThreadId lvgl_timerHandle;
osThreadId uros_threadHandle;
osThreadId ui_threadHandle;



void ui_thread(void const *arg);
void LVGLTimer(void const *arg);
void uros_thread(void const *arg);
void LGVLTick(void const *arg);


static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);

lv_ui guider_ui;

void screen_roller_1_event_handler(lv_event_t *e);



rcl_publisher_t publisher;
std_msgs__msg__Int32 msg;



//USBD_HandleTypeDef hUsbDeviceFS;
USBD_HandleTypeDef  USBD_Device;

void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
  (void)last_call_time;
  if (timer != NULL)
  {
    RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
    // printf("Sent: %d\n", msg.data);
    msg.data++;
  }
}

// Thread Handles


/* LVGL timer for tasks. */
void LVGLTimer(void const *argument)
{
  for (;;)
  {
    lv_timer_handler();
    osDelay(20);
  }
}
/* LVGL tick source */
void LGVLTick(void const *argument)
{
  for (;;)
  {
    lv_tick_inc(10);
    osDelay(10);
  }
}

#define STEP1_GPIO_OUTPUT GPIOC
#define STEP1_OUTPUT_PIN GPIO_PIN_8
#define STEP2_GPIO_OUTPUT GPIOF
#define STEP2_OUTPUT_PIN GPIO_PIN_7

#define STEP1_GPIO_DIR GPIOJ
#define STEP1_DIR_PIN GPIO_PIN_0
#define STEP2_GPIO_DIR GPIOI
#define STEP2_DIR_PIN GPIO_PIN_3

#define STEP_PIN_AF_MODE (1 << 11)
#define STEP_PIN_AF1 (1 << 20)

Stepper *motor1;
Stepper *motor2;
// Queue commands(20);

// J4 and b
// TODO: define correct pins
uint8_t step_init()
{
  motor1 = new Stepper();
  motor2 = new Stepper();

  // Configure output pin for stepper 1 step pin
  GPIO_InitTypeDef GPIO_initstruct;
  GPIO_initstruct.Pin = STEP1_OUTPUT_PIN;
  GPIO_initstruct.Mode = GPIO_MODE_AF_PP;
  GPIO_initstruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(STEP1_GPIO_OUTPUT, &GPIO_initstruct);

  // Configure output pin for stepper 2 step pin
  GPIO_initstruct.Pin = STEP2_OUTPUT_PIN;
  GPIO_initstruct.Mode = GPIO_MODE_AF_PP;
  GPIO_initstruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(STEP2_GPIO_OUTPUT, &GPIO_initstruct);

  // Configure output pin for stepper 1 direction

  GPIO_initstruct.Pin = GPIO_PIN_0;
  GPIO_initstruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_initstruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOJ, &GPIO_initstruct);

  // Configure output pin for stepper 2 direction
  GPIO_initstruct.Pin = GPIO_PIN_3;
  GPIO_initstruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_initstruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_initstruct);

  // Configure output pin for stepper 1 enable

  GPIO_initstruct.Pin = GPIO_PIN_4;
  GPIO_initstruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_initstruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOJ, &GPIO_initstruct);

  // Configure output pin for stepper 2 enable
  GPIO_initstruct.Pin = GPIO_PIN_12;
  GPIO_initstruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_initstruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_initstruct);

  return 1;
}
uint8_t CDC_BUF[128];
int main(void)
{
  uxTopUsedPriority = configMAX_PRIORITIES - 1;
  volatile uint8_t ret = 0;

  CPU_CACHE_Enable();

  HAL_Init();

  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  MX_DMA2D_Init();
  MX_DSIHOST_DSI_Init();
  MX_TIM10_Init();
  MX_TIM3_Init();
  MX_USB_OTG_FS_PCD_Init();


  // lvgl initialisation

  lv_init();
  tft_init();
  touchpad_init();
  setup_ui(&guider_ui);
  //beam_breaker * bb = new beam_breaker(GPIOA_BASE,GPIO_PIN_2);

  events_init_screen(&guider_ui);

  step_init();

  /* Init Device Library */
  ret = USBD_Init(&USBD_Device, &VCP_Desc, 0);

  /* Add Supported Class */
  ret = USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);

  /* Add CDC Interface Class */
  ret = USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);

  /* Start Device Process */
  ret = USBD_Start(&USBD_Device);

  // initial command for stepper initialisation - link to timers and irq handler

  motor1->timerInit(TIM3, 3, TIM3_IRQn, 16000000);
  motor1->setDirPin(GPIOJ, 0);
  motor1->setSleepPin(GPIOB, 8);
  motor1->setSpeed(150);
  motor1->enableInterrupt();

//  motor2->timerInit(TIM11, 1, TIM1_TRG_COM_TIM11_IRQn, 16000000);
///  motor2->setDirPin(GPIOI, 3);
//  motor2->setSleepPin(GPIOB, 8);
//  motor2->setSpeed(150);
//  motor2->enableInterrupt();

  /* Create the thread(s) */
  /* definition and creation of defaultTask */

  /* definition and creation of lvgl_tick */
  osThreadDef(LGVLTick, LGVLTick, osPriorityNormal, 0, 1024);
  lvgl_tickHandle = osThreadCreate(osThread(LGVLTick), NULL);

  // LVGL update timer
  osThreadDef(LVGLTimer, LVGLTimer, osPriorityNormal, 0, 1024);
  lvgl_timerHandle = osThreadCreate(osThread(LVGLTimer), NULL);

  osThreadDef(uros_thread, uros_thread, osPriorityNormal, 0, 4096);
  uros_threadHandle = osThreadCreate(osThread(uros_thread), NULL);

  osThreadDef(ui_thread, ui_thread, osPriorityNormal, 0, 4096);
  ui_threadHandle = osThreadCreate(osThread(ui_thread), NULL);



  /* Start scheduler */
  // anibox_step_gpio();// anibox_step_tim();
  osKernelStart();
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
   */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    //  Error_Handler();
  }

  /** Activate the Over-Drive mode
   */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}

/**
 * @brief Peripherals Common Clock Configuration
 * @retval None
 */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
   */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC | RCC_PERIPHCLK_SAI1 | RCC_PERIPHCLK_SAI2 | RCC_PERIPHCLK_SDMMC2 | RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 3;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLLSAI;
  PeriphClkInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLSAI;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
  PeriphClkInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}





const char *start_byte = "Hello World!\n";

void ui_thread(void const *arg)
{
  while (1)
  {
    lv_task_handler();
    osDelay(5);
  }
}

void uros_thread(void const *arg)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  // micro-ROS configuration

       rmw_uros_set_custom_transport(
        true,
        (void *)USART1,
        cubemx_transport_open,
        cubemx_transport_close,
        cubemx_transport_write,
        cubemx_transport_read);   

  rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
  freeRTOS_allocator.allocate = microros_allocate;
  freeRTOS_allocator.deallocate = microros_deallocate;
  freeRTOS_allocator.reallocate = microros_reallocate;
  freeRTOS_allocator.zero_allocate = microros_zero_allocate;

 

  // micro-ROS app

  rcl_publisher_t publisher;
  std_msgs__msg__Int32 msg;
  rclc_support_t support;
  rcl_allocator_t allocator;
  rcl_node_t node;

  allocator = rcl_get_default_allocator();

  // create init_options
  rclc_support_init(&support, 0, NULL, &allocator);

  // create node
  rclc_node_init_default(&node, "cubemx_node", "", &support);

  // create publisher
  rclc_publisher_init_default(
      &publisher,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "cubemx_publisher");

  msg.data = 0;

  while(1)
  {
    rcl_ret_t ret = rcl_publish(&publisher, &msg, NULL);
    if (ret != RCL_RET_OK)
    {
      osDelay(10);
    }

    msg.data++;
    osDelay(10);

    /* USER CODE END StartDefaultTask */
  }
}
/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
static void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

/// @brief
/// @param e
void screen_roller_1_event_handler(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED)
  {
    char buf[32];
    lv_roller_get_selected_str(obj, buf, sizeof(buf));
    motor1->setSpeed(100);
  }
}



#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
