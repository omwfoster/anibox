#include "main.h"
#include "stm32f7xx_hal.h"
#include "anibox_step.h"

TIM_HandleTypeDef htim1;

int step=0,durum=0,sayac=0;

static void anibox_step_gpio(void);
static void anibox_step_tim(void);                                    
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1) // eger kesme kaynagi timer1 den gelmis ise
	{
		sayac++;								// her kesmede sayaci arttir.
		if(sayac == step)				// eger sayac istenilen adim sayisi kadar artmis ise
		{
			HAL_TIM_PWM_Stop_IT(&htim1,TIM_CHANNEL_1);	// PWM'i durdur.
			sayac=0;																// bir sonraki komutta sayac sifirdan baslamali
			durum=0;																// step fonksiyonunun i�indeki while d�ng�s�nden �ikilmasi i�in
																							// durum degiskeni sifirlandi.
		}
	}
}

void Step(int adim ,int yon)  // step motor fonksiyonu. iki adet parametre alicak. Pals ve y�n parametreleri
{
	step=adim;
	if(yon==0)
		HAL_GPIO_WritePin(GPIOA,DIR_Pin,GPIO_PIN_RESET);	// eger y�n bilgisi sifir ise DIR pini lojik 0 yapildi.
	else
		HAL_GPIO_WritePin(GPIOA,DIR_Pin,GPIO_PIN_SET);	// sifirdan farkli bir deger ise DIR pini lojik 1 yapildi.
	durum=1;
	HAL_TIM_PWM_Start_IT(&htim1,TIM_CHANNEL_1);	// PWM'i baslatildi.
	while(1) // step motorun istenilen konuma gitmesi i�in belli bir s�re gececek
	{						//	bu s�re boyunca programin tekrar step fonksiyonunun i�ine girmesini �nlemek i�in sonsuz d�ng� olusturuldu.
		if(durum==0)					// step motor istenilen konuma gittiginde durum degiskeni sifir olacak ve d�ng�den �ikilacak.
			break;
		HAL_Delay(1);
	}
	
}
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int anibox_step_init(void)
{


 

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  anibox_step_gpio();
  anibox_step_tim();

  /* USER CODE BEGIN 2 */
  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, 128); // Duty cycle %50 olarak ayarlandi.	
	HAL_Delay(100);

}


/* TIM1 init function */
static void anibox_step_tim(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 217;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 255;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim1);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void anibox_step_gpio(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : DIR_Pin */
  GPIO_InitStruct.Pin = DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DIR_GPIO_Port, &GPIO_InitStruct);

}


