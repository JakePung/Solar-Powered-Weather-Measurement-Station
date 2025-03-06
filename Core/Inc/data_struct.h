/*
 * data_struct.h
 *
 *  Created on: Feb 17, 2025
 *      Author: Jake
 */

#ifndef INC_DATA_STRUCT_H_
#define INC_DATA_STRUCT_H_

#include <stdint.h>
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

#endif /* INC_DATA_STRUCT_H_ */
