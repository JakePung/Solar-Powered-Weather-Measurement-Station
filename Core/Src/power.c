/*
 * power.c
 *
 *  Created on: Feb 17, 2025
 *      Author: Jake Pung
 */
#include "main.h"
#include "power.h"
#include "peripheral.h"

void power_down(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); //Demonstrate Low Power Mode
	HAL_Delay(50);
	 __HAL_I2C_DISABLE(&hi2c3); //illuminance sensor

	 __HAL_I2C_DISABLE(&hi2c1); //temp and humidity sensor
}

void power_up(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); //Demonstrate Low Power Mode
	HAL_Delay(50);
	 __HAL_I2C_ENABLE(&hi2c3); //illuminance sensor

	 __HAL_I2C_ENABLE(&hi2c1); //temperature and humidity sensor
}
