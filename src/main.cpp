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

DMA2D_HandleTypeDef hdma2d;
DSI_HandleTypeDef hdsi;
RTC_HandleTypeDef hrtc;
TIM_HandleTypeDef htim10;

osThreadId defaultTaskHandle;
uint8_t cec_receive_buffer[16];
static void Error_Handler(void);

void PeriphCommonClock_Config(void);
static void MX_DMA2D_Init(void);
static void MX_DSIHOST_DSI_Init(void);
static void MX_TIM10_Init(void);


void ui_thread(void const *arg);
void LVGLTimer(void const *arg);
void LGVLTick(void const *arg);
void uros_thread(void const *arg);

static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);

lv_ui guider_ui;

void screen_roller_1_event_handler(lv_event_t *e);
extern "C" void TIM3_IRQHandler(void);
extern "C" void TIM11_IRQHandler(void);

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
osThreadId lvgl_tickHandle;
osThreadId lvgl_timerHandle;

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

  CPU_CACHE_Enable();

  HAL_Init();

  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  MX_DMA2D_Init();
  MX_DSIHOST_DSI_Init();
  MX_TIM10_Init();
  MX_TIM3_Init();


  // lvgl initialisation

  lv_init();
  tft_init();
  touchpad_init();
  setup_ui(&guider_ui);
  //  beam_breaker * bb = new beam_breaker(GPIOA_BASE,GPIO_PIN_2);

  events_init_screen(&guider_ui);

  step_init();

  /* Init Device Library */
  USBD_Init(&USBD_Device, &VCP_Desc, 0);

  /* Add Supported Class */
  USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);

  /* Add CDC Interface Class */
  USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);

  /* Start Device Process */
  USBD_Start(&USBD_Device);

  // initial command for stepper initialisation - link to timers and irq handler

  motor1->timerInit(TIM3, 3, TIM3_IRQn, 16000000);
  motor1->setDirPin(GPIOJ, 0);
  motor1->setSleepPin(GPIOB, 8);
  motor1->setSpeed(150);
  motor1->enableInterrupt();

  motor2->timerInit(TIM11, 1, TIM1_TRG_COM_TIM11_IRQn, 16000000);
  motor2->setDirPin(GPIOI, 3);
  motor2->setSleepPin(GPIOB, 8);
  motor2->setSpeed(150);
  motor2->enableInterrupt();

  /* Create the thread(s) */
  /* definition and creation of defaultTask */

  /* definition and creation of lvgl_tick */
  osThreadDef(LGVLTick, LGVLTick, osPriorityNormal, 0, 1024);
  lvgl_tickHandle = osThreadCreate(osThread(LGVLTick), NULL);

  // LVGL update timer
  osThreadDef(LVGLTimer, LVGLTimer, osPriorityNormal, 0, 1024);
  lvgl_timerHandle = osThreadCreate(osThread(LVGLTimer), NULL);

  osThreadDef(ui_thread, ui_thread, osPriorityNormal, 0, 4096);
  defaultTaskHandle = osThreadCreate(osThread(ui_thread), NULL);

  osThreadDef(uros_thread, uros_thread, osPriorityNormal, 0, 4096);
  defaultTaskHandle = osThreadCreate(osThread(uros_thread), NULL);

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

/**
 * @brief DMA2D Initialization Function
 * @param None
 * @retval None
 */
static void MX_DMA2D_Init(void)
{
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }

}


static void MX_DSIHOST_DSI_Init(void)
{

  DSI_PLLInitTypeDef PLLInit = {0};
  DSI_HOST_TimeoutTypeDef HostTimeouts = {0};
  DSI_PHY_TimerTypeDef PhyTimings = {0};
  DSI_LPCmdTypeDef LPCmd = {0};
  DSI_CmdCfgTypeDef CmdCfg = {0};


  hdsi.Instance = DSI;
  hdsi.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
  hdsi.Init.TXEscapeCkdiv = 4;
  hdsi.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
  PLLInit.PLLNDIV = 20;
  PLLInit.PLLIDF = DSI_PLL_IN_DIV1;
  PLLInit.PLLODF = DSI_PLL_OUT_DIV1;
  if (HAL_DSI_Init(&hdsi, &PLLInit) != HAL_OK)
  {
    Error_Handler();
  }
  HostTimeouts.TimeoutCkdiv = 1;
  HostTimeouts.HighSpeedTransmissionTimeout = 0;
  HostTimeouts.LowPowerReceptionTimeout = 0;
  HostTimeouts.HighSpeedReadTimeout = 0;
  HostTimeouts.LowPowerReadTimeout = 0;
  HostTimeouts.HighSpeedWriteTimeout = 0;
  HostTimeouts.HighSpeedWritePrespMode = DSI_HS_PM_DISABLE;
  HostTimeouts.LowPowerWriteTimeout = 0;
  HostTimeouts.BTATimeout = 0;
  if (HAL_DSI_ConfigHostTimeouts(&hdsi, &HostTimeouts) != HAL_OK)
  {
    Error_Handler();
  }
  PhyTimings.ClockLaneHS2LPTime = 28;
  PhyTimings.ClockLaneLP2HSTime = 33;
  PhyTimings.DataLaneHS2LPTime = 15;
  PhyTimings.DataLaneLP2HSTime = 25;
  PhyTimings.DataLaneMaxReadTime = 0;
  PhyTimings.StopWaitTime = 10;
  if (HAL_DSI_ConfigPhyTimer(&hdsi, &PhyTimings) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigFlowControl(&hdsi, DSI_FLOW_CONTROL_BTA) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetLowPowerRXFilter(&hdsi, 10000) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigErrorMonitor(&hdsi, HAL_DSI_ERROR_NONE) != HAL_OK)
  {
    Error_Handler();
  }
  LPCmd.LPGenShortWriteNoP = DSI_LP_GSW0P_ENABLE;
  LPCmd.LPGenShortWriteOneP = DSI_LP_GSW1P_ENABLE;
  LPCmd.LPGenShortWriteTwoP = DSI_LP_GSW2P_ENABLE;
  LPCmd.LPGenShortReadNoP = DSI_LP_GSR0P_ENABLE;
  LPCmd.LPGenShortReadOneP = DSI_LP_GSR1P_ENABLE;
  LPCmd.LPGenShortReadTwoP = DSI_LP_GSR2P_ENABLE;
  LPCmd.LPGenLongWrite = DSI_LP_GLW_ENABLE;
  LPCmd.LPDcsShortWriteNoP = DSI_LP_DSW0P_ENABLE;
  LPCmd.LPDcsShortWriteOneP = DSI_LP_DSW1P_ENABLE;
  LPCmd.LPDcsShortReadNoP = DSI_LP_DSR0P_ENABLE;
  LPCmd.LPDcsLongWrite = DSI_LP_DLW_ENABLE;
  LPCmd.LPMaxReadPacket = DSI_LP_MRDP_ENABLE;
  LPCmd.AcknowledgeRequest = DSI_ACKNOWLEDGE_DISABLE;
  if (HAL_DSI_ConfigCommand(&hdsi, &LPCmd) != HAL_OK)
  {
    Error_Handler();
  }
  CmdCfg.VirtualChannelID = 0;
  CmdCfg.ColorCoding = DSI_RGB888;
  CmdCfg.CommandSize = 400;
  CmdCfg.TearingEffectSource = DSI_TE_EXTERNAL;
  CmdCfg.TearingEffectPolarity = DSI_TE_RISING_EDGE;
  CmdCfg.HSPolarity = DSI_HSYNC_ACTIVE_LOW;
  CmdCfg.VSPolarity = DSI_VSYNC_ACTIVE_LOW;
  CmdCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;
  CmdCfg.VSyncPol = DSI_VSYNC_FALLING;
  CmdCfg.AutomaticRefresh = DSI_AR_ENABLE;
  CmdCfg.TEAcknowledgeRequest = DSI_TE_ACKNOWLEDGE_ENABLE;
  if (HAL_DSI_ConfigAdaptedCommandMode(&hdsi, &CmdCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetGenericVCID(&hdsi, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DSIHOST_Init 2 */

  /* USER CODE END DSIHOST_Init 2 */
}

static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 0;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 65535;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */
  HAL_TIM_MspPostInit(&htim10);
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

  if (!rcutils_set_default_allocator(&freeRTOS_allocator))
  {
    printf("Error on default allocators (line %d)\n", __LINE__);
  }

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

  for (;;)
  {
    rcl_ret_t ret = rcl_publish(&publisher, &msg, NULL);
    if (ret != RCL_RET_OK)
    {
      printf("Error publishing (line %d)\n", __LINE__);
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

void TIM3_IRQHandler(void)
{
  motor1->interruptHandler();
}

void TIM11_IRQHandler(void)
{
  motor2->interruptHandler();
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
