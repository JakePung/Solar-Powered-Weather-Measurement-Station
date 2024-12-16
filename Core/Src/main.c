/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdbool.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct{
	uint8_t day;		//RTC GetDate
	uint8_t month;
	uint8_t year;
	uint8_t hours;		//RTC GetTime
	uint8_t minutes;
	float temp;			//hi2c1
	float humid;		//hi2c1
	int irrad;			//hi2c3
}samples;

samples sample_array[720]; 	//Sample every 2 minutes, 30 samples/ hour for 24 hours is 720 samples a day
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C1_SLAVE_ADDR	(0x44<<1) //Temp and Humid device address is 0x44, Hal expects 8 bit address though.
#define I2C2_SLAVE_ADDR (0x23<<1) //HAL expects 8 bit address, if you pull addr pin high address changes 0x23low, 0x5c high
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;
__IO uint32_t BspButtonState = BUTTON_RELEASED;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
static void MX_GPIO_Init(void);
static void MX_ICACHE_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C3_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
void store_data(samples *sample); //Pass the struct array of samples, then update the struct with values from sensors
void display_sample_struct(samples *sample); //function for displaying 24 hours worth of data
void power_down(void);
void power_up(void);
//#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//volatile int sensor_read = 1; //For signaling when RTC Alarm is triggered
//volatile bool on_signal = false;
volatile uint32_t count = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the System Power */
  SystemPower_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ICACHE_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_USART3_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  	//ST7735_Init(0);
    //fillScreen(BLACK);
    //testAll();

    HAL_Delay(1000);
    //ST7735_FillRectangle(0, 0, 180, 128, 0xF800); // Fill screen with red
    __enable_irq();
  /* USER CODE END 2 */

  /* Initialize led */
  BSP_LED_Init(LED_GREEN);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN BSP */

  /* -- Sample board code to send message over COM1 port ---- */
  //printf("Welcome to STM32 world !\n\r");

  /* -- Sample board code to switch on led ---- */
  //BSP_LED_On(LED_GREEN);

  /* USER CODE END BSP */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //bool button1, button2, button3; // temporary, need button inputs
  //HAL_UART_Transmit(&huart3, (uint8_t *)"Hello world\n", 12, HAL_MAX_DELAY);

  //printf("Literally print anything\n");
  int entry_shown = 0;
  int new_entry, last_entry = 0;
  int num_of_entries = 0;
  int max_entries = 720; //can be changed, 720 = sample every 2 minutes for 1 day
  //int sensor_read = 1; // can be changed to implement RTC methods, but needs to be set high/at read-level to check sensors on first cycle
  //bool input_detected = 0; no longer required

  int button2 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
  //int button3 =HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
  bool busy = false;
  uint32_t start_powerdown = 0;
  while (1)
  {
	  //printf("Something\n");
	  button2 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	  //button3 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
	 /* button2 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	  HAL_Delay(100);
	  button3 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
	  HAL_Delay(50);
	  	  if (button2 == GPIO_PIN_SET) {  // If button 2 is pressed
	          printf("Button 2 Pressed\n");  // Print message to console
	          button2 = 0;
	      }

	  	  else if (button3 == GPIO_PIN_SET) {  // If button 3 is pressed
	          printf("Button 3 Pressed\n");  // Print message to console
	          button3 = 0;
	      }

	      // Optional: Test BspButtonState if you're using a custom button system (e.g., for button 1)
	  	  else if (BspButtonState == BUTTON_PRESSED) {
	    	  BspButtonState = BUTTON_RELEASED;
	          printf("Button 1 Pressed\n");  // Print message for Button 1
	      }*/
	    //NO USLEEP()
	  	  //printf("about to enter power_down\n");
	  	  //power_down();
	  	  if((count - start_powerdown >= 120000) && !busy){
		     power_up();  // sensor_read wakes up controller from sleep w

			  //printf("\033\143"); // TEMP - putty clear
			  printf("Checking new data..."); //screen shows 'updating'

			  // update the new_entry and last_entry 'pointers' to the storage array
			  if(num_of_entries == 0){ // when there is no data, new_entry is set to the first value
				  new_entry = 0;
				  last_entry = 0;
			  }
			  else{ // in normal operating, new_entry is incremented to not overwrite the previous read
				  new_entry += 1;
				  new_entry = new_entry % max_entries; // will wrap around if reaching the end of the array
			  }
			  //printf("1\n");
			  if(new_entry == last_entry){ // if this read will overwrite the oldest value, increase the pointer to the next oldest value
				last_entry += 1;
				last_entry = last_entry % max_entries; // will wrap around if reaching the end of the array
			  }
			  //printf("2\n");
			  //read from sensors to index of new_entry
			  store_data(&sample_array[new_entry]);

			  //printf("3\n");
			  entry_shown = new_entry; // show the most up-to-date data on the display
			  if(num_of_entries < max_entries){ // increase the 'size' of the used array if not full yet
				  num_of_entries += 1;
			  }

			  //HAL_Delay(500); //leave error messages on the screen for a moment
	  	  	  	  // TODO - store error messages to struct when they come up.

			  //update screen
			  display_sample_struct(&sample_array[entry_shown]);
			  //printf("4\n");
			  power_down();
			  start_powerdown = count;
	  	  }





		 if(BspButtonState == BUTTON_PRESSED){
			  HAL_Delay(50);//input_detected = true; // debounce detector- won't run this section again until all buttons left unpressed.
			  if(BspButtonState == BUTTON_PRESSED){
			  // DISABLED due to RTC being implemented differently
			  //printf("A button is pressed\n");
			/*  if(button3){ // Button 1 decrements the index of the data showed on-screen

				  //printf("button1 was pressed");
				  if(entry_shown == 0){
					  entry_shown = num_of_entries - 1; //wraps around to highest used value when falling off the low end of storage
				  }
				  else{
					  entry_shown -= 1;
				  }
			  }
			  if(button2){ // Button 2 increments the index of the data shown on-screen
				  //printf("button2 was pressed\n");
				  entry_shown += 1;
				  entry_shown = entry_shown % num_of_entries;
			  }*/
			  if(BspButtonState == BUTTON_PRESSED){ //  Button 3 deposits available data to the SD card
				  BspButtonState = BUTTON_RELEASED;
				  // REPURPOSED - instead this button with scan through all avaliable data, displaying each for each struct for a half second before transitioning to the next
				  entry_shown = last_entry;
				  printf("About to display 24 Hours worth of data\n");
				  printf("***************************************\n");
				  printf("***************************************\n");
				  while(1){
					printf("\033\143"); //Clear terminal
					display_sample_struct(&sample_array[entry_shown]);
					HAL_Delay(200);
					if(entry_shown == new_entry){
						printf("REACHED END OF STORED DATA\n RESUMING NORMAL OPERATIONS\n");
						printf("*******************************************************\n");
						printf("*******************************************************\n");
						HAL_Delay(20000);
						break;
					}
					entry_shown += 1;
					entry_shown = entry_shown % num_of_entries;
					if((BspButtonState == BUTTON_PRESSED) || button2){ // Emergency abort of scan
						BspButtonState = BUTTON_RELEASED;
						printf("ABORTING SCROLL\n RESUMING NORMAL FUNCTION WHEN BUTTON RELEASED\n");
						while((BspButtonState == BUTTON_PRESSED) || button2){
							HAL_Delay(100);
						}
					break;
					}
				  }

			  }
		  	}
  	  	  }
		  HAL_Delay(5000);

			//printf("I GET HERE\n");
			//display_sample_struct(&sample_array[entry_shown]);
		   //Following code was heavily influenced from the STM32 website for getting RTC periodic interrupts
		   //https://community.st.com/t5/stm32-mcus/how-to-configure-the-rtc-to-wake-up-the-stm32-periodically-from/ta-p/49836
						//Resume systick
			//}

		  /*if(!button1 && !button2 && !button3){
			  input_detected = 0;
		  } DISABLED due to RTC being implemented differently*/


		  //Sleep til interrupt (RTC or GPIO)



	  /* --- End of Logic Rewrite --- */



	  	//Now format the string:



    /* -- Sample board code for User push-button in interrupt mode ---- */

	//if (BspButtonState == BUTTON_PRESSED)
    //{
      /* Update button state */
      //BspButtonState = BUTTON_RELEASED;
      /* -- Sample board code to toggle led ---- */
      //BSP_LED_Toggle(LED_GREEN);

      /* ..... Perform your action ..... */
    //}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
   }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_0;
  RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV4;
  RCC_OscInitStruct.PLL.PLLM = 3;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 1;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Power Configuration
  * @retval None
  */
static void SystemPower_Config(void)
{

  /*
   * Switch to SMPS regulator instead of LDO
   */
  if (HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY) != HAL_OK)
  {
    Error_Handler();
  }
/* USER CODE BEGIN PWR */
/* USER CODE END PWR */
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x30909DEC;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x30909DEC;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache in 1-way (direct mapped cache)
  */
  if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_PrivilegeStateTypeDef privilegeState = {0};
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_POS1;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  hrtc.Init.BinMode = RTC_BINARY_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
  privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
  privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
  privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
  if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x8;
  sTime.Minutes = 0x30;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_TUESDAY;
  sDate.Month = RTC_MONTH_DECEMBER;
  sDate.Date = 0x16;
  sDate.Year = 0x24;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable Calibration
  */
  if (HAL_RTCEx_SetCalibrationOutPut(&hrtc, RTC_CALIBOUTPUT_1HZ) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  //HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);  // Set the interrupt priority
  HAL_NVIC_EnableIRQ(RTC_IRQn);
  HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC7 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
  //	  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    //  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
     // GPIO_InitStruct.Pull = GPIO_PULLUP;
      //GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
     // GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
  * @brief  Retargets the C library printf function to the USART.
  *   None
  * @retval None
  */
//PUTCHAR_PROTOTYPE
//{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
//	HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);
//	return ch;
//}


void power_down(void){
	 __HAL_I2C_DISABLE(&hi2c3); //illuminance sensor
	 __HAL_I2C_DISABLE(&hi2c1); //temp and humidity sensor
}
void power_up(void){
	 __HAL_I2C_ENABLE(&hi2c3); //illuminance sensor
	 __HAL_I2C_ENABLE(&hi2c1); //temperature and humidity sensor
}
void display_sample_struct(samples *sample){ // temporary Putty code while trying to figure out our display
// Example use: display_sample_struct( *store_data[entry_shown]);d
	printf("\033\143"); //clear Putty
	printf("Date/Time - %d/%d/%d - %d:%d\n",sample->month,sample->day,sample->year,sample->hours,sample->minutes); //print data and time in a single line
	printf("Temperature - %f\u00B0C\n", sample->temp); //print temperature with degree symbol
	printf("Humidity - %f\n", sample->humid); //print relative humidity
	printf("Irradiance - %d\n", sample->irrad); //print irradiance (may require additional filtering)
}
//Function for getting sensor data
void store_data(samples *sample_inst){

	uint8_t SEND_DATA_TH[2] = {0x24, 0x16}; //LOW REPEATABILITY, CLOCK STRETCHING DISSABLED
	uint8_t HUM_TEMP_DATA[6] = {}; //2 bytes for temp, 2 bytes for humid, then checksum
    uint8_t SI_SENSOR_ON[1] = {0x01};// Solar intensity sensor on opcode
	//uint8_t SI_SENSOR_OFF[1] = {0x00};//Solar Intensity sensor off opcode
	uint8_t SI_SENSOR_READ[1] = {0x20}; //One time read/ standard command, dont need mode 2 for lowlight
	uint8_t SI_SENSOR_DATA[2] = {};
	//Query temperature and humidity sensor for a one time measurement, automatically shutsdown after single measurement mode
  	if(HAL_I2C_Master_Transmit(&hi2c1, I2C1_SLAVE_ADDR, SEND_DATA_TH, 2, HAL_MAX_DELAY) != HAL_OK){ //Write 00
  		printf("ERROR!\n");;
  	    HAL_Delay(1000);
  	 }
  	 HAL_Delay(100);
  	 //Get the response from the sensor, 3rd byte is checksum, don't need that just ignore when combining values
  	 if(HAL_I2C_Master_Receive(&hi2c1, (I2C1_SLAVE_ADDR | 0x01), HUM_TEMP_DATA, 6, HAL_MAX_DELAY)!=HAL_OK){ //read |0x01
  		 printf("ERROR!\n");;
  		 HAL_Delay(1000);
  	 }
  	 uint16_t temp_raw = HUM_TEMP_DATA[0] << 8 | HUM_TEMP_DATA[1]; 			//Combine the two bytes into one temperature value
  	 uint16_t humidity_raw = HUM_TEMP_DATA[3] << 8 | HUM_TEMP_DATA[4];		//Combine the humidity values raw
  	 float temp_F = -49 + 315*((float)temp_raw/65535); 						//convert raw reading to Fahrenheit
	 float rel_humidity = 100*((float)humidity_raw/65535); 					//Convert raw reading to humidity%
	 //printf("TEMPERATURE: %f\n", temp_F);
	 /*******************************************************TEST*************************************************************/
	 sample_inst->temp = temp_F;											//Update the array for temperature(F)

	 //HAL_Delay(500);

	 //printf("HUMIDITY: %f\n", rel_humidity);

	 sample_inst->humid = rel_humidity; 									//Update the array for humidity
  	 /*************************************************************************************************************************/
	 //HAL_Delay(1000);

 // __HAL_I2C_DISABLE(&hi2c3); //Restarting i2c3 made it work
  //HAL_Delay(500);
  //__HAL_I2C_ENABLE(&hi2c3);
  //HAL_Delay(100);

   /****************************************************SOLAR INTENSITY****************************************************/
  	//Query Illuminance sensor to get it turned on
  	if(HAL_I2C_Master_Transmit(&hi2c3, I2C2_SLAVE_ADDR, SI_SENSOR_ON, 1, 100) != HAL_OK){ //write
  		  	printf("ERROR!\n");
  		  	HAL_Delay(1000);
    } else{
    	//printf("Im so turned on rn\n");
    }

  	HAL_Delay(10);
  	//Query sensor to get a reading
  	if(HAL_I2C_Master_Transmit(&hi2c3, I2C2_SLAVE_ADDR, SI_SENSOR_READ, 1, 10) != HAL_OK){ //write
  		printf("ERROR!\n");;
  		HAL_Delay(1000);
    }

  	HAL_Delay(120);
  	//Grab measured illuminance from the
  	if(HAL_I2C_Master_Receive(&hi2c3, (I2C2_SLAVE_ADDR | 0x01), SI_SENSOR_DATA, 2, 10)!=HAL_OK){ //Read

  		printf("ERROR!\n");;
  		HAL_Delay(1000);
    }

  	HAL_Delay(100);
  	//printf("Got here #3\n");
  	uint16_t raw_intensity = SI_SENSOR_DATA[0] << 8 | SI_SENSOR_DATA[1]; 	//format the sensor data 16 bits
  	int intensity = (int)raw_intensity/1.2; 								// cast binary value to a int for conversion formula,

  	//printf("SOLAR INTENSITY: %d\n", intensity); 							//pprint it oooooout


  	sample_inst->irrad = intensity;


  	RTC_TimeTypeDef sTime;
  	RTC_DateTypeDef sDate;
  	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); 						//Put this because RTC locks up after callling GetTime for protection, unlock it by calling this

  	//Update sample structure array instance with the date and time.
  	sample_inst->day = sDate.Date;
  	sample_inst->month = sDate.Month;
  	sample_inst->year = sDate.Year;

  	sample_inst->minutes = sTime.Minutes;
  	sample_inst->hours = sTime.Hours;

  	//printf("%d/%d/%d\n", sDate.Month, sDate.Date, sDate.Year);
  	//printf("%d:%d:%d\n", sTime.Hours, sTime.Minutes, sTime.Seconds);

}
/* USER CODE END 4 */

/**
  * @brief BSP Push Button callback
  * @param Button Specifies the pressed button
  * @retval None
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  if (Button == BUTTON_USER)
  {
    BspButtonState = BUTTON_PRESSED;
  }
}

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
