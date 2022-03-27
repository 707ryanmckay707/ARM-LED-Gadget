/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


#include "main.h"
#include "stm32f0xx.h"

const int NUM_OF_LEDS = 4;
const int DELAY_TIME = 100;

void SystemClock_Config(void);
void resetLEDs(const int, int*, int*);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();


  /* Configure the system clock */
  SystemClock_Config();


  /* Initialize all configured peripherals */


  //Port C is used for the LEDs

  //Enable GPIO for Port C (For the output pin(s))
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

  for(int i = 0; i < 4; ++i)
  {
	  GPIOC->MODER |= (0x1UL << (12 + i * 2));		//GPIO_MODER_MODERx_0 set the 1 in 01
	  GPIOC->MODER &= ~(0x2UL << (12 + i * 2)); 	//~(GPIO_MODER_MODERx_1) set the 0 in 01

	  GPIOC->OTYPER &= ~(0x40 << i);				//~(GPIO_OTYPER_OT_x)

	  GPIOC->OSPEEDR |= (0x3UL << (12 + i * 2));	//GPIO_OSPEEDER_OSPEEDRx

	  GPIOC->PUPDR  &= ~(0x3UL << (12 + i * 2));	//~(GPIO_PUPDR_PUPDRx)
  }


  //Port B is used for the buttons

  //Enable GPIO for Port B (For the input pin)
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

  //The button connected to Port B Pin 0 is used to change the number of LEDs that light up in the chain.
  //Setup Port B Pin 0 for Input
  GPIOB->MODER &= ~(GPIO_MODER_MODER0_Msk);
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR0);

  //The button connected to Port B Pin 1 is used to light up the LED chain.
  //Setup Port B Pin 1 for Input
  GPIOB->MODER &= ~(GPIO_MODER_MODER1_Msk);
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR1);




  int numOfLEDsToLight = NUM_OF_LEDS;
  int pBp1ButtonCurrentlyPressed = 0;

  int setIdx = 0;
  int resetIdx = NUM_OF_LEDS - 1;


  while (1)
  {
	  if((GPIOB->IDR & GPIO_IDR_0) && !pBp1ButtonCurrentlyPressed)
	  {
		  --numOfLEDsToLight;
		  if(numOfLEDsToLight == 0)
			  numOfLEDsToLight = NUM_OF_LEDS;
		  resetLEDs(numOfLEDsToLight, &setIdx, &resetIdx);

		  pBp1ButtonCurrentlyPressed = 1;
	  }

	  //if the button isn't currently pressed but it previously was
	  if((~(GPIOB->IDR) & GPIO_IDR_0) && pBp1ButtonCurrentlyPressed)
	  {
		  pBp1ButtonCurrentlyPressed = 0;
	  }

	  if(GPIOB->IDR & GPIO_IDR_1)
	  {
		  if(numOfLEDsToLight == 1)
		  {
			  GPIOC->BSRR |= GPIO_BSRR_BS_6;
		  }
		  else
		  {
			  GPIOC->BSRR |= (0x40U << setIdx);		//GPIO_BSRR_BS_setIdx
			  GPIOC->BRR |= (0x40U << resetIdx);	//GPIO_BRR_BR_resetIdx;

			  ++setIdx;
			  if(setIdx >= numOfLEDsToLight)
				  setIdx = 0;
			  ++resetIdx;
			  if(resetIdx >= numOfLEDsToLight)
				  resetIdx = 0;

			  HAL_Delay(DELAY_TIME);
		  }
	  }
	  else
	  {
		  resetLEDs(numOfLEDsToLight, &setIdx, &resetIdx);
	  }
  }
}

void resetLEDs(const int numOfLEDsToLight, int* setIdx, int* resetIdx)
{
	for(int pinOffest = 0; pinOffest < NUM_OF_LEDS; ++pinOffest)
	  GPIOC->BRR |= (0x40U << pinOffest);

	*setIdx = 0;
	*resetIdx = numOfLEDsToLight - 1;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
