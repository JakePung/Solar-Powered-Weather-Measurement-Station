/*
 * print_data.c
 *
 *  Created on: Feb 17, 2025
 *      Author: Jake Pung
 */
#include "main.h"
#include "print_data.h"
#include "data_struct.h"
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"

void display_sample_struct(samples *sample, int position){ // temporary Putty code while trying to figure out our display
// Example use: display_sample_struct( *store_data[entry_shown]);
	char buff1[21];
	char buff2[21];
	if(position){
		ssd1306_SetCursor(0, 0);
		sprintf(buff1, "%d/%d/%d %d:%d",sample->month,sample->day,sample->year,sample->hours,sample->minutes);
		ssd1306_WriteString(buff1, Font_6x8, White);

		ssd1306_SetCursor(0, 8);
		sprintf(buff2,"T:%0.1f H:%0.1f I:%d", sample->temp, sample->humid, sample->irrad);
		ssd1306_WriteString(buff2, Font_6x8, White);
	}else{
		ssd1306_SetCursor(0, 16);
		sprintf(buff1, "%d/%d/%d %d:%d",sample->month,sample->day,sample->year,sample->hours,sample->minutes);
	    ssd1306_WriteString(buff1, Font_6x8, White);

	    ssd1306_SetCursor(0, 24);
	    sprintf(buff2,"T:%0.1f H:%0.1f I:%d", sample->temp, sample->humid, sample->irrad);
		ssd1306_WriteString(buff2, Font_6x8, White);
    }
	/*printf("\033\143"); //clear Putty
	printf("Date/Time - %d/%d/%d - %d:%d\n",sample->month,sample->day,sample->year,sample->hours,sample->minutes); //print data and time in a single line
	printf("Temperature - %f\u00B0C\n", sample->temp); //print temperature with degree symbol
	printf("Humidity - %f\n", sample->humid); //print relative humidity
	printf("Irradiance - %d\n", sample->irrad); //print irradiance (may require additional filtering)*/
}
