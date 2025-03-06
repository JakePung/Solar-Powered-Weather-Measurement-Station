#include "main.h"
#include "sensor_query.h"
#include "peripheral.h"

#define I2C1_SLAVE_ADDR	(0x44<<1) //Temp and Humid device address is 0x44, Hal expects 8 bit address though.
#define I2C3_SLAVE_ADDR (0x23<<1) //HAL expects 8 bit address, if you pull addr pin high address changes 0x23low, 0x5c high (IRRD


void store_data(samples *sample_inst){

	uint8_t SEND_DATA_TH[2] = {0x24, 0x16}; 	//LOW REPEATABILITY, CLOCK STRETCHING DISSABLED
	uint8_t HUM_TEMP_DATA[6] = {}; 				//2 bytes for temp, 2 bytes for humid, then checksum
    uint8_t SI_SENSOR_ON[1] = {0x01};			// Solar intensity sensor on opcode
	//uint8_t SI_SENSOR_OFF[1] = {0x00};//Solar Intensity sensor off opcode
	uint8_t SI_SENSOR_READ[1] = {0x20}; 		//One time read/ standard command, dont need mode 2 for lowlight
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


	 /*******************************************************TEST*************************************************************/
	 sample_inst->temp = temp_F;											//Update the array for temperature(F)

	 //HAL_Delay(500);

	 //printf("HUMIDITY: %f\n", rel_humidity);

	 sample_inst->humid = rel_humidity; 									//Update the array for humidity
  	 /*************************************************************************************************************************/

 // __HAL_I2C_DISABLE(&hi2c3); //Restarting i2c3 made it work
  //HAL_Delay(500);
  //__HAL_I2C_ENABLE(&hi2c3);
  //HAL_Delay(100);

   /****************************************************SOLAR INTENSITY****************************************************/
  	//Query Illuminance sensor to get it turned on
  	if(HAL_I2C_Master_Transmit(&hi2c3, I2C3_SLAVE_ADDR, SI_SENSOR_ON, 1, 100) != HAL_OK){ //write
  		  	printf("ERROR!\n");
  		  	HAL_Delay(1000);
    } else{
    	//printf("Im so turned on rn\n");
    }

  	HAL_Delay(10);
  	//Query sensor to get a reading
  	if(HAL_I2C_Master_Transmit(&hi2c3, I2C3_SLAVE_ADDR, SI_SENSOR_READ, 1, 10) != HAL_OK){ //write
  		printf("ERROR!\n");;
  		HAL_Delay(1000);
    }

  	HAL_Delay(120);
  	//Grab measured illuminance from the
  	if(HAL_I2C_Master_Receive(&hi2c3, (I2C3_SLAVE_ADDR | 0x01), SI_SENSOR_DATA, 2, 10)!=HAL_OK){ //Read

  		printf("ERROR!\n");;
  		HAL_Delay(1000);
    }

  	HAL_Delay(100);
  	//printf("Got here #3\n");
  	uint16_t raw_intensity = SI_SENSOR_DATA[0] << 8 | SI_SENSOR_DATA[1]; 	//format the sensor data 16 bits
  	int intensity = (int)raw_intensity/1.2; 								// cast binary value to a int for conversion formula,

  	sample_inst->irrad = intensity;


  	RTC_TimeTypeDef sTime;
  	RTC_DateTypeDef sDate;

  	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); //Put this because RTC locks up after callling GetTime for protection, unlock it by calling this

  	//Update sample structure array instance with the date and time.
  	sample_inst->day = sDate.Date;
  	sample_inst->month = sDate.Month;
  	sample_inst->year = sDate.Year;

  	sample_inst->minutes = sTime.Minutes;
  	sample_inst->hours = sTime.Hours;

}
